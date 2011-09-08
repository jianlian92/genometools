/*
  Copyright (c) 2005-2007 Stefan Kurtz <kurtz@zbh.uni-hamburg.de>
  Copyright (c) 2005-2007 Center for Bioinformatics, University of Hamburg

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

#ifndef SFX_RI_DEF_H
#define SFX_RI_DEF_H
#include "core/array.h"
#include "core/error.h"
#include "core/str.h"
#include "core/logger.h"

#define SETREADINTKEYS(VALNAME,VAL,FORCEREAD)\
        gt_scannedprjkey_add(riktab,VALNAME,VAL,sizeof (*(VAL)),false,FORCEREAD)

typedef struct GtScannedprjkey GtScannedprjkey;

size_t gt_scannedprjkey_size(void);

void gt_scannedprjkey_add(GtArray *riktab,
                          const char *keystring,
                          void *valueptr,
                          size_t sizeval,
                          bool readdouble,
                          bool *readflag);

int gt_allkeysdefined(const char *indexname,
                      const char *suffix,
                      const GtArray *riktab,
                      GtLogger *logger,
                      GtError *err);

int gt_scannedprjkey_analyze(const char *indexname,
                             const char *suffix,
                             unsigned int linenum,
                             const char *linebuffer,
                             unsigned long linelength,
                             GtArray *riktab,
                             GtError *err);

#endif
