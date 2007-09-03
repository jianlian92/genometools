/*
** Copyright (C) 2007 Thomas Jahns <Thomas.Jahns@gmx.net>
**  
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**  
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**  
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
**  
*/
#ifndef ENCIDXSEQ_H_INCLUDED
#define ENCIDXSEQ_H_INCLUDED

/**
 * \file encidxseq.h
 * Interface definitions for encoded indexed sequences.
 */

#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif /* HAVE_STDINT_H */
#ifdef HAVE_INTTYPES_H
#include <inttypes.h>
#endif /* HAVE_INTTYPES_H */

#include <libgtcore/bitpackstring.h>
#include <libgtcore/env.h>
#include <libgtcore/str.h>
#include <libgtmatch/seqpos-def.h>

#include "mrangealphabet.h"

/**
 * callback function to insert variable width data into encidx
 * construction
 * @param dest bitstring to append data to (can hold at least as many
 * bits as specified in the constructor)
 * @param sOffset position in dest at which the callee may start
 * writing
 * @param start the data will be written together with the encoding of
 * symbols at position start up to position start + len
 * @param len see parameter start for description
 * @param cbState is passed on every call back of bitInsertFunc to
 * pass information that is kept across individual calls
 * @param env passes information about allocator etc
 * @return number of bits actually written, or (BitOffset)-1 if an
 * error occured
 */
typedef BitOffset (*bitInsertFunc)(BitString dest, BitOffset sOffset,
                                   Seqpos start, Seqpos len, void *cbState,
                                   Env *env);

/* typedef int_fast64_t Seqpos; */
enum rangeStoreMode {
  DIRECT_SYM_ENCODE,
  BLOCK_COMPOSITION_INCLUDE,
  REGIONS_LIST,
};

typedef union EISHint *EISHint;

extern struct encIdxSeq *
newBlockEncIdxSeq(const Str *projectName, unsigned blockSize,
                  bitInsertFunc biFunc, BitOffset maxBitsPerPos, void *cbState,
                  Env *env);
extern struct encIdxSeq *
loadBlockEncIdxSeq(const Str *projectName, Env *env);

extern int
searchBlock2IndexPair(const struct encIdxSeq *seqIdx,
                      const Symbol *block,
                      size_t idxOutput[2], Env *env);

extern void
deleteEncIdxSeq(struct encIdxSeq *seq, Env *env);

staticifinline inline const MRAEnc *
EISGetAlphabet(const struct encIdxSeq *seq);

staticifinline inline Seqpos
EISRank(struct encIdxSeq *seq, Symbol sym, Seqpos pos, union EISHint *hint,
        Env *env);

staticifinline inline Seqpos
EISSymTransformedRank(struct encIdxSeq *seq, Symbol msym, Seqpos pos,
                      union EISHint *hint, Env *env);

extern Seqpos
EISSelect(struct encIdxSeq *seq, Symbol sym, Seqpos count);

staticifinline inline Seqpos
EISLength(struct encIdxSeq *seq);

staticifinline inline Symbol
EISGetSym(struct encIdxSeq *seq, Seqpos pos, EISHint hint, Env *env);

staticifinline inline EISHint
newEISHint(struct encIdxSeq *seq, Env *env);

staticifinline inline void
deleteEISHint(struct encIdxSeq *seq, EISHint hint, Env *env);

extern int
verifyIntegrity(struct encIdxSeq *seqIdx,
                Str *projectName, int tickPrint, FILE *fp, Env *env);

#ifdef HAVE_WORKING_INLINE
#include "../encidxseqsimpleop.c"
#endif

#endif /* ENCIDXSEQ_H_INCLUDED */
