/*
  Copyright (c) 2006-2008 Gordon Gremme <gremme@zbh.uni-hamburg.de>
  Copyright (c) 2006-2008 Center for Bioinformatics, University of Hamburg

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

#ifndef FILTER_STREAM_H
#define FILTER_STREAM_H

#include "extended/node_stream.h"

/* implements the ``genome stream'' interface */
typedef struct GtFilterStream GtFilterStream;

const GtNodeStreamClass* gt_filter_stream_class(void);
GtNodeStream*            gt_filter_stream_new(GtNodeStream*,
                                           GtStr *seqid, GtStr *typefilter,
                                           GtRange contain_range,
                                           GtRange overlap_range,
                                           GtStrand strand,
                                           GtStrand targetstrand,
                                           bool has_CDS,
                                           unsigned long max_gene_length,
                                           unsigned long max_gene_num,
                                           double min_gene_score,
                                           double max_gene_score,
                                           double min_average_splice_site_prob,
                                           unsigned long feature_num);

#endif
