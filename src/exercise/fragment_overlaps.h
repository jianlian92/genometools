/*
  Copyright (c) 2008 Gordon Gremme <gremme@zbh.uni-hamburg.de>
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

#ifndef FRAGMENT_OVERLAPS_H
#define FRAGMENT_OVERLAPS_H

#include <stdbool.h>
#include "core/bioseq.h"

typedef struct GtFragmentOverlaps GtFragmentOverlaps;

typedef struct {
  unsigned long start,  /* the start fragment / vertex of the edge */
                weight, /* the weight of the edge */
                end;    /* the end fragment / vertex of the edge */
} Overlap;

/* Create a new fragment overlaps object which stores the overlaps between the
   given <fragments> (if they are equal or larger than <minlength). */
GtFragmentOverlaps* gt_fragment_overlaps_new(GtBioseq *fragments,
                                        unsigned long minlength);
void              gt_fragment_overlaps_delete(GtFragmentOverlaps*);
void              gt_fragment_overlaps_sort(GtFragmentOverlaps*);
bool              gt_fragment_overlaps_are_sorted(const GtFragmentOverlaps*);
void              gt_fragment_overlaps_show(const GtFragmentOverlaps*);
const Overlap*    gt_fragment_overlaps_get(const GtFragmentOverlaps*, unsigned long);
unsigned long     gt_fragment_overlaps_size(const GtFragmentOverlaps*);

#endif
