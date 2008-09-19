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

#ifndef GREEDY_ASSEMBLY_H
#define GREEDY_ASSEMBLY_H

#include "core/bioseq.h"
#include "exercise/fragment_overlaps.h"

typedef struct GreedyAssembly GreedyAssembly;

GreedyAssembly* greedy_assembly_new(GtBioseq *fragments,
                                    GtFragmentOverlaps *sorted_overlaps);
void            greedy_assembly_delete(GreedyAssembly*);
void            greedy_assembly_show(const GreedyAssembly*,
                                     GtBioseq *fragments);
void            greedy_assembly_show_path(const GreedyAssembly*);

#endif
