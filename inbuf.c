/*=				       	-*- c-file-style: "bsd" -*-
 * rproxy -- dynamic caching and delta update in HTTP
 * $Id$
 *
 * Copyright (C) 2000 by Martin Pool <mbp@linuxcare.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/*
     XXX
      XXX
       XXX
        XXX    THIS FILE IS GOING AWAY SOON!
         XXX   inbufs are deprecated in favour of mapptr
          XXX
           XXX
            XXX
  */

#include "includes.h"


int const       _hs_inbuf_dogtag = 0x6a656e;


/**
 * Allocate and return a new hs_inbuf_t structure.
 **/
_hs_inbuf_t    *
_hs_new_inbuf(void)
{
    _hs_inbuf_t    *inbuf;

    inbuf = calloc(1, sizeof *inbuf);

    /* must be at least two blocks; shouldn't be too small. */
    inbuf->len = 8192;
    inbuf->buf = malloc(inbuf->len);
    inbuf->amount = 0;
    inbuf->abspos = 0;
    inbuf->tag = _hs_inbuf_dogtag;

    return inbuf;
}


/**
 * Release all resources used by this inbuf.
 **/
void
_hs_free_inbuf(_hs_inbuf_t * inbuf)
{
    assert(inbuf->tag == _hs_inbuf_dogtag);
    if (inbuf->buf)
	free(inbuf->buf);
    hs_bzero(inbuf, sizeof *inbuf);
    free(inbuf);
}


int
_hs_fill_inbuf(_hs_inbuf_t * inbuf, hs_read_fn_t read_fn, void *readprivate)
{
    int             ret;

    assert(inbuf->tag == _hs_inbuf_dogtag);

    if (inbuf->amount > inbuf->len) {
	abort();
    } else if (inbuf->amount < 0) {
	abort();
    }

    ret = _hs_read_loop(read_fn, readprivate,
			inbuf->buf + inbuf->amount,
			inbuf->len - inbuf->amount);
    if (ret < 0) {
	_hs_fatal(__FILE__ "error reading into input buffer");
	return ret;
    } else if (ret == 0) {
	_hs_trace("reached end of input file");
    } else {
	_hs_trace("read %d bytes into input buffer"
		  " at abspos=%-10d", ret, inbuf->abspos);
    }
    inbuf->amount += ret;
    return ret;
}



int
_hs_slide_inbuf(_hs_inbuf_t * inbuf)
{
    assert(inbuf->tag == _hs_inbuf_dogtag);
    /* Copy the remaining data into the front of the buffer */
    if (inbuf->cursor != 0) {
	if (inbuf->amount != inbuf->cursor) {
	    memcpy(inbuf->buf,
		   inbuf->buf + inbuf->cursor, inbuf->amount - inbuf->cursor);
	    _hs_trace("slide %d bytes down to the start of the buffer",
		      inbuf->amount - inbuf->cursor);
	}

	inbuf->amount -= inbuf->cursor;
	inbuf->abspos += inbuf->cursor;
	inbuf->cursor = 0;
    }

    return 0;
}
