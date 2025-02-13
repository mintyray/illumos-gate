/*
 * CDDL HEADER START
 *
 * The contents of this file are subject to the terms of the
 * Common Development and Distribution License (the "License").
 * You may not use this file except in compliance with the License.
 *
 * You can obtain a copy of the license at src/OPENSOLARIS.LICENSE
 * or http://www.opensolaris.org/os/licensing.
 * See the License for the specific language governing permissions
 * and limitations under the License.
 *
 * When distributing Covered Code, include this CDDL HEADER in each
 * file and include the License file at src/OPENSOLARIS.LICENSE.
 * If applicable, add the following below this CDDL HEADER, with the
 * fields enclosed by brackets "[]" replaced with your own identifying
 * information: Portions Copyright [yyyy] [name of copyright owner]
 *
 * CDDL HEADER END
 */
/*
 * Copyright(c) 1998 Sun Microsystems, Inc.
 * All rights reserved.
 */

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include "unicode_gb2312.h"
#include "common_defs.h"

#define MSB		0x80
#define	NON_ID_CHAR	'?'

typedef struct _icv_state {
	short	_ustate;
	short	saved_ustate;
	char	_cbuf[3];
} _iconv_st;

enum	_USTATE	{ U0, U1, U2, U3, U4, U5, U6 };

int unicode_to_gb_to_hz(char in_byte1, char in_byte2, char *buf, int buflen);

/*
 * Open; called from iconv_open()
 */
void *
_icv_open()
{
	_iconv_st *st;

	if ((st = (_iconv_st *)malloc(sizeof(_iconv_st))) == NULL) {
		errno = ENOMEM;
		return ((void *) -1);
	}

	st->_ustate = U0;
	st->saved_ustate = U0;
	return ((void *)st);
}


/*
 * Close; called from iconv_close()
 */
void
_icv_close(_iconv_st *st)
{
	if (st == NULL)
		errno = EBADF;
	else
		free(st);
}


/*
 * Actual conversion; called from iconv()
 */
size_t
_icv_iconv(_iconv_st *st, char **inbuf, size_t*inbytesleft,
			char **outbuf, size_t*outbytesleft)
{
	char	c1, c2;
	int	n;

	if (st == NULL) {
		errno = EBADF;
		return ((size_t)-1);
	}

	if (inbuf == NULL || *inbuf == NULL) { /* Reset request. */
		st->_ustate = U0;
		return ((size_t)0);
	}

	errno = 0;
	while (*inbytesleft > 0 && *outbytesleft > 0) {

	    uchar_t  first_byte;

	    switch (st->_ustate) {
	    case U0:
		if (**inbuf & MSB && st->saved_ustate ==U0) {
			if(*outbytesleft >=2) {
			**outbuf = '~';
			*(*outbuf+1) = '{';
			(*outbuf) += 2, (*outbytesleft) -= 2;
			} else {
                        errno = E2BIG;
                        return (size_t)-1;
			}
		}
		if ((**inbuf & MSB) == 0) {	/* ASCII */
		    if (st->saved_ustate == U1 || st->saved_ustate == U3)
		    {
		    if(*outbytesleft >=2) {
                        **outbuf = '~';
                        *(*outbuf+1) = '}';
                        (*outbuf) += 2, (*outbytesleft) -= 2;
                        }else {
                            errno = E2BIG;
                            return (size_t)-1;
                        }
                    }
		    st->saved_ustate = U0;
		    if(*outbytesleft >=1) {
		    **outbuf = **inbuf;
		    (*outbuf)++; (*outbytesleft)--;
                    }else {
                        errno = E2BIG;
                        return (size_t)-1;
                    }
		    if (**inbuf == '~') {
		        if(*outbytesleft >=1) {
                        **outbuf = '~';
                        (*outbuf)++, (*outbytesleft)--;
                        }else {
                            errno = E2BIG;
                            return (size_t)-1;
                        }
                    }
	        } else if ((**inbuf & 0xe0) == 0xc0) { /* 0xc2..0xbf */

		    /* invalid sequence if the first char is either 0xc0 or 0xc1 */
		    if ( number_of_bytes_in_utf8_char[((uchar_t)**inbuf)] == (char)ICV_TYPE_ILLEGAL_CHAR )
		         errno = EILSEQ;
		    else {
		         st->_ustate = U1;
		         st->_cbuf[0] = **inbuf;
		    }
		} else if ((**inbuf & 0xf0) == 0xe0) { /* 0xe0..0xef */
		    st->_ustate = U2;
		    st->_cbuf[0] = **inbuf;
		} else {
		    /* four bytes of UTF-8 sequences */
		    if ( number_of_bytes_in_utf8_char[((uchar_t)**inbuf)] == (char)ICV_TYPE_ILLEGAL_CHAR )
		         errno = EILSEQ;
		    else
		     {
			st->_ustate = U4;
			st->_cbuf[0] = **inbuf;
		     }
		}
		break;
	    case U1:
		if ((**inbuf & 0xc0) == MSB) {	/* Two-byte UTF */
		    c1 = (st->_cbuf[0]&0x1c)>>2;
		    c2 = ((st->_cbuf[0]&0x03)<<6) | ((**inbuf)&0x3f);
		    n = unicode_to_gb_to_hz(c1, c2, *outbuf, *outbytesleft);
		    if (n > 0) {
			(*outbuf) += n, (*outbytesleft) -= n;
		    } else {
			errno = E2BIG;
			return ((size_t) -1);
		    }
		    st->saved_ustate = U1;
		    st->_ustate = U0;
		} else {
		    errno = EILSEQ;
		}
		break;
	    case U2:
		st->saved_ustate = U2;

	        first_byte = st->_cbuf[0];

	        /* if the first byte is 0xed, it is illegal sequence if the second
		 * one is between 0xa0 and 0xbf because surrogate section is ill-formed
		 */
	        if (((uchar_t)**inbuf) < valid_min_2nd_byte[first_byte] ||
		    ((uchar_t)**inbuf) > valid_max_2nd_byte[first_byte] )
		    errno = EILSEQ;
	        else {
		    st->_ustate = U3;
		    st->_cbuf[1] = **inbuf;
		}
		break;
	    case U3:
		if ((**inbuf & 0xc0) == MSB) {	/* Three-byte UTF */
		    c1 = ((st->_cbuf[0]&0x0f)<<4) | ((st->_cbuf[1]&0x3c)>>2);
		    c2 = ((st->_cbuf[1]&0x03)<<6) | ((**inbuf)&0x3f);
		    n = unicode_to_gb_to_hz(c1, c2, *outbuf, *outbytesleft);
		    if (n > 0) {
			(*outbuf) += n, (*outbytesleft) -= n;
		    } else if ( n == -1 ) { /* unicode is either 0xFFFE or 0xFFFF */
		        errno = EILSEQ;
		    } else {
			errno = E2BIG;
			return ((size_t)-1);
		    }
		    st->saved_ustate = U3;
		    st->_ustate = U0;
		} else {
		    errno = EILSEQ;
		    break;
		}
		break;
	    case U4:

	       first_byte = st->_cbuf[0];

	       /* if the first byte is 0xf0, it is illegal sequence if
		* the second one is between 0x80 and 0x8f
		* for Four-Byte UTF: U+10000..U+10FFFF
		*/
	       if (((uchar_t)**inbuf) < valid_min_2nd_byte[first_byte] ||
		   ((uchar_t)**inbuf) > valid_max_2nd_byte[first_byte] )
		    errno = EILSEQ;
	       else
		 {
		    st->_ustate = U5;
		    st->_cbuf[1] = **inbuf;
		    st->saved_ustate = U4;
		 }
	       break;
	    case U5:
	       if ((**inbuf & 0xc0) == MSB) /* 0x80..0xbf */
		 {
		    st->_ustate = U6;
		    st->_cbuf[2] = **inbuf;
		    st->saved_ustate = U5;
		 }
	       else
		 errno = EILSEQ;
	       break;
	    case U6:
	       if ((**inbuf & 0xc0) == MSB) /* 0x80..0xbf */
		 {
		    /* replace with double NON_ID_CHARs */
		    if ( *outbytesleft < 2 )
		       errno = E2BIG;
		    else
		      {
			 **outbuf = NON_ID_CHAR;
			 *(*outbuf+1) = NON_ID_CHAR;
			 (*outbytesleft) -= 2;

			 st->_ustate = U0;
			 st->saved_ustate = U6;
		      }
		 }
	       else
		 errno = EILSEQ;
	       break;
	    }

	    if (errno)
			return ((size_t)-1);
	    (*inbuf)++; (*inbytesleft)--;
	}

        if (*inbytesleft == 0 && st->_ustate != U0)
          {
	     errno = EINVAL;
	     return ((size_t) -1);
          }

	if (*inbytesleft > 0 && *outbytesleft == 0) {
		errno = E2BIG;
		return ((size_t)-1);
	}
	return ((size_t)(*inbytesleft));
}

/* return value: 0 - no enough space to hold the HZ-GB-2312 code
 *              -1 - illegal sequence
 *              >0 - buffer length
 */
int unicode_to_gb_to_hz(in_byte1, in_byte2, buf, buflen)
char	in_byte1, in_byte2;
char	*buf;
int	buflen;
{
	int	gb, unicode;
	int	i, l, h;

	if (buflen < 2)
		return 0;
	unicode = ((in_byte1 & 0xff) << 8) + (in_byte2 & 0xff);

        /* 0xfffe and 0xffff should not be allowed */
        if ( unicode == 0xFFFE || unicode == 0xFFFF ) return -1;

	for (l = 0, h = UNICODEMAX; l < h; ) {
		if (unicode_gb_tab[l].key == unicode) {
			i = l;
			break;
		}
		if (unicode_gb_tab[h].key == unicode) {
			i = h;
			break;
		}
		i = (l + h) / 2;
		if (unicode_gb_tab[i].key == unicode)
			break;
		if (unicode_gb_tab[i].key < unicode)
			l = i + 1;
		else	h = i - 1;
	}
	if (unicode == unicode_gb_tab[i].key) {
		gb = unicode_gb_tab[i].value;
		*buf = ((gb & 0xff00) >> 8) & 0x7f;
		*(buf+1) = (gb & 0xff) & 0x7f;
	} else {
		*buf = NON_ID_CHAR;
		*(buf+1) = NON_ID_CHAR;
	}
	return 2;
}
