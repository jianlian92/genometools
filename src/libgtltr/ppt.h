/*
  Copyright (c) 2008 Sascha Steinbiss <ssteinbiss@zbh.uni-hamburg.de>
  Copyright (c) 2008 Center for Bioinformatics, University of Hamburg

  Permission to use, copy, modify, and distribute this software for any
  purpose with or without fee is hereby granted, provided that the above
  copyright notice and this permission notice appear in all copies.

  THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
  WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
  MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
  ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
  WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
  ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
  OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/

#ifndef PPT_H
#define PPT_H

#include "libgtcore/alpha.h"
#include "libgtext/hmm.h"
#include "libgtltr/repeattypes.h"
#include "libgtltr/ltrharvest-opt.h"
#include "libgtcore/undef.h"

/* This enumeration defines the states in the PPT detection HMM. */
typedef enum {
  PPT_IN,
  PPT_OUT,
  PPT_UBOX,
  PPT_NOF_STATES
} PPT_States;

/* This struct holds information about a PPT or U-box region.
   See ppt.c for HMM parameters. */
typedef struct PPT_Hit PPT_Hit;

struct PPT_Hit {
  unsigned long start, end;
  double score;
  PPT_States state;
  PPT_Hit *ubox;
};

/* Initializes a new HMM with PPT/U-box finding capability. */
HMM*     ppt_hmm_new(const Alpha *alpha);

/* Score function for PPT candidates. */
double   ppt_score(unsigned long posdiff, unsigned int width);

/* Searches for PPTs in the given sequence and returns the index of the
   highest scored PPT in the results array. */
unsigned long  ppt_find(LTRboundaries *ltr,
                        Array *results,
                        unsigned int radius,
                        const char *seq,
                        LTRharvestoptions *lo,
                        double score_func(unsigned long, unsigned int));

#endif
