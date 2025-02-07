/*
 * Copyright 2016 Jeremy Allison
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/*
 * Copyright 2019 Joyent, Inc.
 */

#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <pthread.h>

extern const char *__progname;

static void *
server(void *varg)
{
	int ret;
	int sock = (int)(intptr_t)varg;
	unsigned int i;

	for (i = 0; i < 5; i++) {
		struct iovec iov;
		struct msghdr msg;
		uint8_t buf[4096];

		iov = (struct iovec) {
			.iov_base = buf,
			.iov_len = sizeof (buf)
		};

		msg = (struct msghdr) {
			.msg_iov = &iov,
			.msg_iovlen = 1,
		};

		ret = recvmsg(sock, &msg, 0);
		if (ret == -1) {
			fprintf(stderr, "server - recvmsg fail %s\n",
			    strerror(errno));
			exit(1);
		}

		printf("SERVER:%s\n", (char *)msg.msg_iov->iov_base);
		fflush(stdout);
	}

	exit(0);
}

static void *
listener(void *varg)
{
	struct sockaddr_un *addr = varg;
	int ret;
	int lsock;
	int asock;

	/* Child. */
	lsock = socket(AF_UNIX, SOCK_STREAM, 0);
	if (lsock == -1) {
		fprintf(stderr, "listener - socket fail %s\n", strerror(errno));
		exit(1);
	}

	ret = bind(lsock, (struct sockaddr *)addr, sizeof (*addr));
	if (ret == -1) {
		fprintf(stderr, "listener - bind fail %s\n", strerror(errno));
		exit(1);
	}

	ret = listen(lsock, 5);
	if (ret == -1) {
		fprintf(stderr, "listener - listen fail %s\n", strerror(errno));
		exit(1);
	}

	for (;;) {
		asock = accept(lsock, NULL, 0);
		if (asock == -1) {
			fprintf(stderr, "listener - accept fail %s\n",
			    strerror(errno));
			exit(1);
		}

		/* start worker */
		ret = pthread_create(NULL, NULL, server, (void *)(intptr_t)asock);
		if (ret == -1) {
			fprintf(stderr, "%s - thread create fail %s\n",
			    __progname, strerror(errno));
			exit(1);
		}
	}
}

/*
 * This should be a place only root is allowed to write.
 * The test will create and destroy this directory.
 */
char testdir[100] = "/var/run/os-tests-sockfs";
struct sockaddr_un addr;
int test_uid = UID_NOBODY;

int
main(int argc, char **argv)
{
	int ret;
	int sock;
	unsigned int i;

	if (argc > 1) {
		ret = strlcpy(testdir, argv[1], sizeof (testdir));
		if (ret >= sizeof (testdir)) {
			fprintf(stderr, "%s: too long\n", argv[1]);
			exit(1);
		}
	}

	addr.sun_family = AF_UNIX;
	(void) sprintf(addr.sun_path, "%s/s", testdir);

	if (mkdir(testdir, 0700) != 0) {
		switch (errno) {
		case EEXIST:
		case EISDIR:
			break;
		default:
			perror(testdir);
			exit(1);
		}
	}
	(void) unlink(addr.sun_path);

	/* Set up the listener. */
	ret = pthread_create(NULL, NULL, listener, &addr);
	if (ret == -1) {
		fprintf(stderr, "%s - thread create fail %s\n",
		    argv[0], strerror(errno));
		exit(1);
	}

	sleep(1);

	/* Create and connect the socket endpoint. */

	sock = socket(AF_UNIX, SOCK_STREAM, 0);
	if (sock == -1) {
		fprintf(stderr, "%s - socket fail %s\n",
		    argv[0], strerror(errno));
		exit(1);
	}

	ret = connect(sock, (struct sockaddr *)&addr, sizeof (addr));
	if (ret == -1) {
		fprintf(stderr, "%s - connect fail %s\n",
		    argv[0], strerror(errno));
		exit(1);
	}

	/* Send some messages */
	for (i = 0; i < 5; i++) {
		struct iovec iov;
		struct msghdr msg;
		uint8_t buf[4096];

		memcpy(buf, "TEST0", sizeof ("TEST0"));
		buf[4] = '0' + i;

		printf("CLIENT:%s\n", buf);

		iov = (struct iovec) {
			.iov_base = buf,
			.iov_len = sizeof (buf),
		};

		msg = (struct msghdr) {
			.msg_iov = &iov,
			.msg_iovlen = 1,
		};

		ret = sendmsg(sock, &msg, 0);

		if (ret == -1) {
			fprintf(stderr, "%s - sendmsg fail %s\n",
			    argv[0], strerror(errno));
			exit(1);
		}

		fflush(stdout);
		sleep(1);
	}

	close(sock);
	return (0);
}
