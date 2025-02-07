/*
 * Copyright 2005 Sun Microsystems, Inc.  All rights reserved.
 * Use is subject to license terms.
 */

/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#include "tip.h"

char *DV;	/* UNIX device(s) to open */
char *EL;	/* chars marking an EOL */
char *CM;	/* initial connection message */
char *IE;	/* EOT to expect on input */
char *OE;	/* EOT to send to complete FT */
char *CU;	/* call unit if making a phone call */
char *AT;	/* acu type */
char *PN;	/* phone number(s) */
char *DI;	/* disconnect string */
char *PA;	/* parity to be generated */

char *PH;	/* phone number file */
char *RM;	/* remote file name */
char *HO;	/* host name */

int BR;		/* line speed for conversation */
int FS;		/* frame size for transfers */
int DU;		/* this host is dialed up */
char HW;	/* this device is hardwired, see hunt.c */
char *ES;	/* escape character */
char *EX;	/* exceptions */
char *FO;	/* force (literal next) char */
char *RC;	/* raise character */
char *RE;	/* script record file */
char *PR;	/* remote prompt */
int DL;		/* line delay for file transfers to remote */
int CL;		/* char delay for file transfers to remote */
int ET;		/* echocheck timeout */
int DB;		/* dialback - ignore hangup */

/*
 * Attributes to be gleened from remote host description
 *   data base.
 */
static char **caps[] = {
	&AT, &DV, &CM, &CU, &EL, &IE, &OE, &PN, &PR, &DI,
	&ES, &EX, &FO, &RC, &RE, &PA
};

static char *capstrings[] = {
	"at", "dv", "cm", "cu", "el", "ie", "oe", "pn", "pr",
	"di", "es", "ex", "fo", "rc", "re", "pa", 0
};

extern char *rgetstr(char *, char **);

static void
getremcap(char *host)
{
	int stat;
	char tbuf[BUFSIZ];
	static char buf[BUFSIZ/2];
	char *bp = buf;
	char **p, ***q;

	if ((stat = rgetent(tbuf, host, sizeof (tbuf))) <= 0) {
		if (DV ||
		    host[0] == '/' && access(DV = host, R_OK | W_OK) == 0) {
			/*
			 * If the user specifies a device on the commandline,
			 * don't trust it.
			 */
			if (host[0] == '/')
				trusted_device = 0;
			CU = DV;
			HO = host;
			HW = 1;
			DU = 0;
			if (!BR)
				BR = DEFBR;
			FS = DEFFS;
			RE = (char *)"tip.record";
			EX = (char *)"\t\n\b\f";
			DL = 0;
			CL = 0;
			ET = 10;
			return;
		}
		(void) fprintf(stderr, stat == 0 ?
		    "tip: unknown host %s\n" :
		    "tip: can't open host description file\n", host);
		exit(3);
	}

	for (p = capstrings, q = caps; *p != NULL; p++, q++)
		if (**q == NULL)
			**q = rgetstr(*p, &bp);
	if (!BR && (BR = rgetnum("br")) < 0)
		BR = DEFBR;
	if ((FS = rgetnum("fs")) < 0)
		FS = DEFFS;
	if (DU < 0)
		DU = 0;
	else
		DU = rgetflag("du");
	if (DV == NOSTR) {
		(void) fprintf(stderr, "%s: missing device spec\n", host);
		exit(3);
	}
	if (DU && CU == NOSTR)
		CU = DV;
	if (DU && PN == NOSTR) {
		(void) fprintf(stderr, "%s: missing phone number\n", host);
		exit(3);
	}
	DB = rgetflag("db");

	/*
	 * This effectively eliminates the "hw" attribute
	 *   from the description file
	 */
	if (!HW)
		HW = (CU == NOSTR) || (DU && equal(DV, CU));
	HO = host;
	/*
	 * see if uppercase mode should be turned on initially
	 */
	if (rgetflag("ra"))
		boolean(value(RAISE)) = 1;
	if (rgetflag("ec"))
		boolean(value(ECHOCHECK)) = 1;
	if (rgetflag("be"))
		boolean(value(BEAUTIFY)) = 1;
	if (rgetflag("nb"))
		boolean(value(BEAUTIFY)) = 0;
	if (rgetflag("sc"))
		boolean(value(SCRIPT)) = 1;
	if (rgetflag("tb"))
		boolean(value(TABEXPAND)) = 1;
	if (rgetflag("vb"))
		boolean(value(VERBOSE)) = 1;
	if (rgetflag("nv"))
		boolean(value(VERBOSE)) = 0;
	if (rgetflag("ta"))
		boolean(value(TAND)) = 1;
	if (rgetflag("nt"))
		boolean(value(TAND)) = 0;
	if (rgetflag("rw"))
		boolean(value(RAWFTP)) = 1;
	if (rgetflag("hd"))
		boolean(value(HALFDUPLEX)) = 1;
	if (rgetflag("hf"))
		boolean(value(HARDWAREFLOW)) = 1;
	if (RE == NULL)
		RE = (char *)"tip.record";
	if (EX == NULL)
		EX = (char *)"\t\n\b\f";
	if (ES != NOSTR)
		(void) vstring("es", ES);
	if (FO != NOSTR)
		(void) vstring("fo", FO);
	if (PR != NOSTR)
		(void) vstring("pr", PR);
	if (RC != NOSTR)
		(void) vstring("rc", RC);
	if ((DL = rgetnum("dl")) < 0)
		DL = 0;
	if ((CL = rgetnum("cl")) < 0)
		CL = 0;
	if ((ET = rgetnum("et")) < 0)
		ET = 10;
}

char *
getremote(char *host)
{
	char *cp;
	static char *next;
	static int lookedup = 0;

	if (!lookedup) {
		if (host == NOSTR && (host = getenv("HOST")) == NOSTR) {
			(void) fprintf(stderr, "tip: no host specified\n");
			exit(3);
		}
		getremcap(host);
		next = DV;
		lookedup++;
	}
	/*
	 * We return a new device each time we're called (to allow
	 *   a rotary action to be simulated)
	 */
	if (next == NOSTR)
		return (NOSTR);
	if ((cp = strchr(next, ',')) == NULL) {
		DV = next;
		next = NOSTR;
	} else {
		*cp++ = '\0';
		DV = next;
		next = cp;
	}
	return (DV);
}
