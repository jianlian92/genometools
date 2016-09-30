#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <limits.h>
#include "core/assert_api.h"
#include "core/unused_api.h"
#include "core/divmodmul.h"
#include "core/chardef.h"
#include "core/divmodmul.h"
#include "core/intbits.h"
#include "core/encseq.h"
#include "match/extend-offset.h"
#include "core/ma_api.h"
#include "core/types_api.h"
#include "ft-front-prune.h"
#include "ft-trimstat.h"
#include "core/minmax.h"
#include "ft-polish.h"
#include "ft-front-generation.h"

#define GT_UPDATE_MATCH_HISTORY(FRONTVAL)\
        if ((FRONTVAL)->matchhistory_size == max_history)\
        {\
          if ((FRONTVAL)->matchhistory_bits & leftmostbit)\
          {\
            gt_assert((FRONTVAL)->matchhistory_count > 0);\
            (FRONTVAL)->matchhistory_count--;\
          }\
        } else\
        {\
          gt_assert((FRONTVAL)->matchhistory_size < max_history);\
          (FRONTVAL)->matchhistory_size++;\
        }\
        (FRONTVAL)->matchhistory_bits <<= 1

typedef uint32_t GtFtRowvaluetype;
typedef uint8_t GtFtMatchcounttype;
typedef uint8_t GtFtBackreferencetype;

typedef struct
{
  uint64_t matchhistory_bits;
  GtFtRowvaluetype row,
                   localmatch_count;
  GtFtMatchcounttype matchhistory_count,
                     matchhistory_size;
  GtFtBackreferencetype backreference;
  uint32_t max_mismatches; /* maximum number of mismatches in a path to this
                              Front-entry.*/
} GtFtFrontvalue;

#ifndef OUTSIDE_OF_GT
typedef struct
{
  const GtTwobitencoding *twobitencoding;
  const GtEncseq *encseq;
  const GtUchar *bytesequenceptr;
  GtEncseqReader *encseqreader;
  GtUchar *cache_ptr;
  GtAllocatedMemory *sequence_cache;
  GtUword substringlength,
          totallength,
          min_access_pos, /* no position accessed will be smaller than this */
          cache_num_positions; /* number of positions in cache */
  GtUword offset,
          seqstartpos;
  bool read_seq_left2right,
       dir_is_complement;
} GtFtSequenceObject;

static void ft_sequenceobject_init(GtFtSequenceObject *seq,
                                   GtExtendCharAccess extend_char_access_mode,
                                   const GtEncseq *encseq,
                                   bool rightextension,
                                   GtReadmode readmode,
                                   GtUword seqstartpos,
                                   GtUword startpos,
                                   GtUword len,
                                   GtEncseqReader *encseq_r,
                                   GtAllocatedMemory *sequence_cache,
                                   const GtUchar *bytesequence,
                                   GtUword totallength
                                   )
{
  gt_assert(seq != NULL);
  seq->encseq = NULL;
  seq->encseqreader = NULL;
  seq->twobitencoding = NULL;
  seq->cache_ptr = NULL;
  seq->sequence_cache = NULL;
  seq->bytesequenceptr = NULL;
  seq->seqstartpos = seqstartpos;
  gt_assert(seqstartpos <= startpos);
  seq->offset = GT_EXTEND_OFFSET(rightextension,
                                 readmode,
                                 totallength,
                                 seqstartpos,
                                 startpos,
                                 len);
  seq->read_seq_left2right = GT_EXTEND_READ_SEQ_LEFT2RIGHT(rightextension,
                                                           readmode);
  if (encseq != NULL && extend_char_access_mode == GT_EXTEND_CHAR_ACCESS_ANY &&
      gt_encseq_has_twobitencoding(encseq) && gt_encseq_wildcards(encseq) == 0)
  {
    seq->twobitencoding = gt_encseq_twobitencoding_export(encseq);
  }
  if (encseq != NULL && seq->twobitencoding == NULL &&
      (extend_char_access_mode == GT_EXTEND_CHAR_ACCESS_ANY ||
       extend_char_access_mode == GT_EXTEND_CHAR_ACCESS_ENCSEQ_READER))
  {
    GtUword full_totallength = gt_encseq_total_length(encseq);
    gt_encseq_reader_reinit_with_readmode(encseq_r, encseq,
                                          seq->read_seq_left2right
                                            ? GT_READMODE_FORWARD
                                            : GT_READMODE_REVERSE,
                                          seq->read_seq_left2right
                                            ? seq->offset
                                            : GT_REVERSEPOS(full_totallength,
                                                            seq->offset));
    seq->encseqreader = encseq_r;
    gt_assert(seq->encseqreader != NULL && sequence_cache != NULL);
    seq->sequence_cache = sequence_cache;
    seq->cache_ptr = (GtUchar *) sequence_cache->space;
    seq->min_access_pos = GT_UWORD_MAX; /* undefined */
    seq->cache_num_positions = 0;
  }
  if (encseq != NULL && seq->twobitencoding == NULL &&
      seq->encseqreader == NULL &&
      (extend_char_access_mode == GT_EXTEND_CHAR_ACCESS_ANY ||
       extend_char_access_mode == GT_EXTEND_CHAR_ACCESS_ENCSEQ))
  {
    seq->encseq = encseq;
  }
  if (extend_char_access_mode == GT_EXTEND_CHAR_ACCESS_DIRECT)
  {
    gt_assert(seq->twobitencoding == NULL && seq->encseqreader == NULL &&
              seq->encseq == NULL);
    seq->bytesequenceptr = bytesequence;
  }
  seq->substringlength = len;
  seq->totallength = totallength;
  seq->dir_is_complement = GT_ISDIRCOMPLEMENT(readmode) ? true : false;
  gt_assert(seq->twobitencoding != NULL || seq->encseqreader != NULL ||
            seq->encseq != NULL || seq->bytesequenceptr != NULL);
}

static GtUchar gt_twobitencoding_char_at_pos(
                                      const GtTwobitencoding *twobitencoding,
                                      GtUword pos)
{
  return (twobitencoding[GT_DIVBYUNITSIN2BITENC(pos)] >>
          GT_MULT2(GT_UNITSIN2BITENC - 1 - GT_MODBYUNITSIN2BITENC(pos))) & 3;
}

static GtUchar ft_sequenceobject_get_char(GtFtSequenceObject *seq,GtUword idx)
{
  GtUchar cc;
  GtUword accesspos;

  if (seq->twobitencoding != NULL)
  {
    gt_assert (seq->read_seq_left2right || seq->offset >= idx);
    accesspos = seq->read_seq_left2right ? seq->offset + idx
                                         : seq->offset - idx;
    gt_assert(accesspos < seq->seqstartpos + seq->totallength);
    cc = gt_twobitencoding_char_at_pos(seq->twobitencoding, accesspos);
    return seq->dir_is_complement ? GT_COMPLEMENTBASE(cc) : cc;
  }
  if (seq->encseqreader != NULL)
  {
    gt_assert(idx < seq->substringlength);
    if (idx >= seq->cache_num_positions)
    {
      GtUword idx, tostore;
      const GtUword addamount = 256UL;

      tostore = MIN(seq->cache_num_positions + addamount,
                    seq->substringlength);
      if (tostore > seq->sequence_cache->allocated)
      {
        seq->sequence_cache->allocated += addamount;
        seq->sequence_cache->space
          = gt_realloc(seq->sequence_cache->space,
                       sizeof (GtUchar) * seq->sequence_cache->allocated);
        seq->cache_ptr = (GtUchar *) seq->sequence_cache->space;
      }
      for (idx = seq->cache_num_positions; idx < tostore; idx++)
      {
        seq->cache_ptr[idx]
          = gt_encseq_reader_next_encoded_char(seq->encseqreader);
      }
      seq->cache_num_positions = tostore;
    }
    gt_assert(seq->cache_ptr != NULL && idx < seq->cache_num_positions);
    cc = seq->cache_ptr[idx];
  } else
  {
    accesspos = seq->read_seq_left2right ? seq->offset + idx
                                         : seq->offset - idx;
    if (seq->encseq != NULL)
    {
      cc = gt_encseq_get_encoded_char(seq->encseq,accesspos,
                                      GT_READMODE_FORWARD);
    } else
    {
      gt_assert(seq->bytesequenceptr != NULL);
      cc = seq->bytesequenceptr[accesspos];
    }
  }
  if (seq->dir_is_complement && !ISSPECIAL(cc))
  {
    return GT_COMPLEMENTBASE(cc);
  }
  return cc;
}

#undef SKDEBUG
#ifdef SKDEBUG
static char *gt_ft_sequencebject_get(GtFtSequenceObject *seq)
{
  GtUword idx;
  char *buffer;
  char *map = "acgt";

  gt_assert(seq != NULL);
  buffer = gt_malloc(sizeof *buffer * (seq->substringlength+1));
  for (idx = 0; idx < seq->substringlength; idx++)
  {
    GtUchar cc = ft_sequenceobject_get_char(seq,idx);

    if (cc == WILDCARD)
    {
      buffer[idx] = '#';
    } else
    {
      if (cc == SEPARATOR)
      {
        buffer[idx] = '$';
      } else
      {
        gt_assert(cc < 4);
        buffer[idx] = map[cc];
      }
    }
  }
  buffer[seq->substringlength] = '\0';
  return buffer;
}

static void gt_greedy_show_context(bool rightextension,
                                   GtFtSequenceObject *useq,
                                   GtFtSequenceObject *vseq)
{
  char *uptr = gt_ft_sequencebject_get(useq);
  char *vptr = gt_ft_sequencebject_get(vseq);
  printf(">%sextension:\n>%s\n>%s\n",rightextension ? "right" : "left",
         uptr,vptr);
  gt_free(uptr);
  gt_free(vptr);
}
#endif

#else
typedef struct
{
  const GtUchar *sequence_ptr;
  GtUword substringlength;
} GtFtSequenceObject;

static void ft_sequenceobject_init(GtFtSequenceObject *seq,
                                   const GtUchar *ptr,
                                   GtUword seqstartpos,
                                   GtUword startpos,
                                   GtUword len)
{
  gt_assert(seq != NULL);
  seq->sequence_ptr = ptr + seqstartpos + startpos;
  seq->substringlength = len;
}
#endif

#define FRONT_DIAGONAL(FRONTPTR) (GtWord) ((FRONTPTR) - midfront)

static bool ft_sequenceobject_symbol_match(GtFtSequenceObject *useq,
                                           GtUword upos,
                                           GtFtSequenceObject *vseq,
                                           GtUword vpos)
{
#ifndef OUTSIDE_OF_GT
  GtUchar cu = ft_sequenceobject_get_char(useq,upos);
  if (ISSPECIAL(cu))
  {
    return false;
  }
  return cu == ft_sequenceobject_get_char(vseq,vpos) ? true : false;
#else
  GtUchar cu = useq->sequence_ptr[upos];
  return cu == vseq->sequence_ptr[vpos] ? true : false;
#endif
}

static void inline front_prune_add_matches(GtFtFrontvalue *midfront,
                                           GtFtFrontvalue *fv,
                                           uint64_t leftmostbit,
                                           GtUword max_history,
                                           GtFtSequenceObject *useq,
                                           GtFtSequenceObject *vseq)
{
  GtUword upos, vpos;

  for (upos = fv->row, vpos = fv->row + FRONT_DIAGONAL(fv);
       upos < useq->substringlength && vpos < vseq->substringlength &&
       ft_sequenceobject_symbol_match(useq,upos,vseq,vpos);
       upos++, vpos++)
  {
    if (fv->matchhistory_size == max_history)
    {
      if (!(fv->matchhistory_bits & leftmostbit))
      {
        gt_assert(fv->matchhistory_count < INT8_MAX);
        fv->matchhistory_count++;
      }
    } else
    {
      gt_assert(fv->matchhistory_size < max_history);
      fv->matchhistory_size++;
      fv->matchhistory_count++;
    }
    fv->matchhistory_bits = (fv->matchhistory_bits << 1) | (uint64_t) 1;
  }
  fv->localmatch_count = upos - fv->row;
  fv->row = upos;
}

static GtUword front_next_inplace(GtFtFrontvalue *midfront,
                                  GtFtFrontvalue *lowfront,
                                  GtFtFrontvalue *highfront,
                                  GtUword max_history,
                                  GtFtSequenceObject *useq,
                                  GtFtSequenceObject *vseq)
{
  GtUword alignedlen, maxalignedlen;
  const uint64_t leftmostbit = ((uint64_t) 1) << (max_history-1);
  GtFtFrontvalue bestfront, insertion_value, replacement_value, *frontptr;

  insertion_value = *lowfront; /* from previous diag -(d-1) => -d => DELETION */
  bestfront = insertion_value;
  bestfront.row++;
  GT_UPDATE_MATCH_HISTORY(&bestfront);
  *lowfront = bestfront;
  lowfront->backreference = FT_EOP_DELETION;
  front_prune_add_matches(midfront,lowfront,leftmostbit,max_history,useq,vseq);
  maxalignedlen = GT_MULT2(lowfront->row) + FRONT_DIAGONAL(lowfront);

  replacement_value = *(lowfront+1);
  if (bestfront.row < replacement_value.row + 1)
  {
    bestfront = replacement_value;
    bestfront.backreference = FT_EOP_DELETION;
    bestfront.row++;
    GT_UPDATE_MATCH_HISTORY(&bestfront);
  } else
  {
    bestfront.backreference = FT_EOP_MISMATCH;
    bestfront.max_mismatches++;
    if (bestfront.row == replacement_value.row + 1)
    {
      bestfront.backreference |= FT_EOP_DELETION;
      if (bestfront.max_mismatches < replacement_value.max_mismatches)
      {
        bestfront.max_mismatches = replacement_value.max_mismatches;
      }
    }
  }
  *(lowfront+1) = bestfront;
  front_prune_add_matches(midfront,lowfront + 1,leftmostbit,max_history,
                          useq,vseq);
  alignedlen = GT_MULT2((lowfront+1)->row) + FRONT_DIAGONAL(lowfront + 1);
  if (maxalignedlen < alignedlen)
  {
    maxalignedlen = alignedlen;
  }
  for (frontptr = lowfront+2; frontptr <= highfront; frontptr++)
  {
    bestfront = insertion_value;
    bestfront.backreference = FT_EOP_INSERTION;
    if (frontptr <= highfront - 1)
    {
      if (bestfront.row < replacement_value.row + 1)
      {
        bestfront = replacement_value;
        bestfront.backreference = FT_EOP_MISMATCH;
        bestfront.max_mismatches++;
        bestfront.row++;
      } else
      {
        if (bestfront.row == replacement_value.row + 1)
        {
          bestfront.backreference |= FT_EOP_MISMATCH;
          if (bestfront.max_mismatches < replacement_value.max_mismatches + 1)
          {
            bestfront.max_mismatches = replacement_value.max_mismatches + 1;
          }
        }
      }
    }
    if (frontptr <= highfront - 2)
    {
      if (bestfront.row < frontptr->row + 1)
      {
        bestfront = *frontptr;
        bestfront.backreference = FT_EOP_DELETION;
        bestfront.row++;
      } else
      {
        if (bestfront.row == frontptr->row + 1)
        {
          bestfront.backreference |= FT_EOP_DELETION;
        }
      }
    }
    GT_UPDATE_MATCH_HISTORY(&bestfront);
    if (frontptr < highfront)
    {
      insertion_value = replacement_value;
      replacement_value = *frontptr;
    }
    *frontptr = bestfront;
    front_prune_add_matches(midfront,frontptr,leftmostbit,max_history,
                            useq,vseq);
    alignedlen = GT_MULT2(frontptr->row) + FRONT_DIAGONAL(frontptr);
    if (maxalignedlen < alignedlen)
    {
      maxalignedlen = alignedlen;
    }
  }
  return maxalignedlen;
}

static GtUword front_second_inplace(GtFtFrontvalue *midfront,
                                    GtFtFrontvalue *lowfront,
                                    GtUword max_history,
                                    GtFtSequenceObject *useq,
                                    GtFtSequenceObject *vseq)
{
  GtUword alignedlen, maxalignedlen;
  const uint64_t leftmostbit = ((uint64_t) 1) << (max_history-1);

  *(lowfront+1) = *(lowfront+2) = *lowfront;
  lowfront->row++;
  lowfront->backreference = FT_EOP_DELETION;
  GT_UPDATE_MATCH_HISTORY(lowfront);
  front_prune_add_matches(midfront,lowfront,leftmostbit,max_history,useq,vseq);
  maxalignedlen = GT_MULT2(lowfront->row) + FRONT_DIAGONAL(lowfront);

  (lowfront+1)->row++;
  (lowfront+1)->backreference = FT_EOP_MISMATCH;
  (lowfront+1)->max_mismatches++;
  GT_UPDATE_MATCH_HISTORY(lowfront+1);
  front_prune_add_matches(midfront,lowfront + 1,leftmostbit,max_history,
                          useq,vseq);
  alignedlen = GT_MULT2((lowfront+1)->row) + FRONT_DIAGONAL(lowfront + 1);
  if (maxalignedlen < alignedlen)
  {
    maxalignedlen = alignedlen;
  }

  (lowfront+2)->backreference = FT_EOP_INSERTION;
  GT_UPDATE_MATCH_HISTORY(lowfront+2);
  front_prune_add_matches(midfront,lowfront + 2,leftmostbit,max_history,useq,
                          vseq);
  alignedlen = GT_MULT2((lowfront+2)->row) + FRONT_DIAGONAL(lowfront + 2);
  if (maxalignedlen < alignedlen)
  {
    maxalignedlen = alignedlen;
  }
  return maxalignedlen;
}

static bool trimthisentry(GtUword distance,
                          GtFtRowvaluetype row,
                          GtWord diagonal,
                          const GtFtFrontvalue *fv,
                          GtUword minmatchpercentage,
                          GtUword minlenfrommaxdiff,
                          bool showfrontinfo)
{
  if (fv->matchhistory_count < (fv->matchhistory_size * minmatchpercentage)/100)
  {
    if (showfrontinfo)
    {
      GtUword alignedlen = GT_MULT2(row) + diagonal;
      double identity = 100.0 * (1.0 - 2.0 * (double) distance/alignedlen);
      printf("aligned=" GT_WU ",diagonal=" GT_WD ", distance=" GT_WU
             ", row=%" PRIu32 ", identity=%.2f, hist_size=%d, matches=%d "
             "< " GT_WU "=minmatches\n",
             alignedlen,
             diagonal,
             distance,
             row,
             identity,
             (int) fv->matchhistory_size,
             (int) fv->matchhistory_count,
             fv->matchhistory_size * minmatchpercentage/100);
    }
    return true;
  }
  if (GT_MULT2(row) + diagonal < minlenfrommaxdiff)
  {
    if (showfrontinfo)
    {
      printf(GT_WD "&" GT_WU "&%" PRIu32 "&2: i'+j'=" GT_WU "<"
             GT_WU "=i+j-lag\n",
             diagonal,distance,row,GT_MULT2(row) + diagonal,minlenfrommaxdiff);
    }
    return true;
  }
  if (showfrontinfo)
  {
    printf(GT_WD "&" GT_WU "&%" PRIu32 "\n", diagonal,distance,row);
  }
  return false;
}

static GtUword trim_front(bool upward,
                          GtUword distance,
                          GtUword ulen,
                          GtUword vlen,
                          GtUword minmatchpercentage,
                          GtUword minlenfrommaxdiff,
                          GtTrimmingStrategy trimstrategy,
                          const GtFtPolished_point *best_polished_point,
                          const GtFtFrontvalue *midfront,
                          const GtFtFrontvalue *from,
                          const GtFtFrontvalue *stop,
                          bool showfrontinfo)
{
  const GtFtFrontvalue *frontptr;
  GtUword trim = 0;

  if (trimstrategy == GT_OUTSENSE_TRIM_NEVER ||
      (trimstrategy == GT_OUTSENSE_TRIM_ON_NEW_PP &&
       best_polished_point != NULL &&
       best_polished_point->distance + 1 < distance &&
       best_polished_point->distance + 30 >= distance))
  {
    return 0;
  }
  gt_assert ((upward && from < stop) || (!upward && stop < from));
  for (frontptr = from; frontptr != stop; frontptr = upward ? (frontptr + 1)
                                                            : (frontptr - 1))
  {
    if (frontptr->row > ulen ||
        frontptr->row + FRONT_DIAGONAL(frontptr) > vlen ||
        trimthisentry(distance,
                      frontptr->row,
                      FRONT_DIAGONAL(frontptr),
                      frontptr,
                      minmatchpercentage,
                      minlenfrommaxdiff,
                      showfrontinfo))
    {
      trim++;
    } else
    {
      break;
    }
  }
  return trim;
}

static void frontspace_check(GT_UNUSED const GtFtFrontvalue *from,
                             GT_UNUSED const GtFtFrontvalue *to,
                             GT_UNUSED const GtFtFrontvalue *ptr)
{
  gt_assert (ptr >= from && ptr <= to);
}

static GtFtFrontvalue *frontspace_allocate(GtUword minsizeforshift,
                                       GtUword trimleft,
                                       GtUword valid,
                                       GtAllocatedMemory *fs)
{
  if (trimleft - fs->offset + valid >= fs->allocated)
  {
    fs->allocated = 255UL + MAX(fs->allocated * 1.2,
                                           trimleft - fs->offset + valid);
    gt_assert(fs->allocated > trimleft - fs->offset + valid);
    fs->space = gt_realloc(fs->space,sizeof (GtFtFrontvalue) * fs->allocated);
    gt_assert(fs->space != NULL);
  }
  gt_assert(trimleft >= fs->offset);
  if (trimleft - fs->offset > MAX(valid,minsizeforshift))
  {
    memcpy(fs->space,((GtFtFrontvalue *) fs->space) + trimleft - fs->offset,
           sizeof (GtFtFrontvalue) * valid);
    fs->offset = trimleft;
  }
  return ((GtFtFrontvalue *) fs->space) - fs->offset;
}

static void update_trace_and_polished(GtFtPolished_point *best_polished_point,
#ifndef OUTSIDE_OF_GT
                                      GtUword *minrow,
                                      GtUword *mincol,
#endif
                                      GtFronttrace *front_trace,
                                      const GtFtPolishing_info *pol_info,
                                      GtUword distance,
                                      GtUword trimleft,
                                      GtFtFrontvalue *midfront,
                                      GtFtFrontvalue *lowfront,
                                      GtFtFrontvalue *highfront,
                                      bool showfrontinfo)
{
  const GtFtFrontvalue *frontptr;

#ifndef OUTSIDE_OF_GT
  *minrow = GT_UWORD_MAX;
  *mincol = GT_UWORD_MAX;
#endif
  for (frontptr = lowfront; frontptr <= highfront; frontptr++)
  {
    GtUword alignedlen = GT_MULT2(frontptr->row) + FRONT_DIAGONAL(frontptr);
    uint64_t filled_matchhistory_bits;

#ifndef OUTSIDE_OF_GT
    GtUword currentcol;

    if (*minrow > frontptr->row)
    {
      *minrow = frontptr->row;
    }
    gt_assert(FRONT_DIAGONAL(frontptr) >= 0 ||
              frontptr->row >= -FRONT_DIAGONAL(frontptr));
    currentcol = frontptr->row + FRONT_DIAGONAL(frontptr);
    if (*mincol > currentcol)
    {
      *mincol = currentcol;
    }
#endif
    if (frontptr->matchhistory_size >= GT_MULT2(pol_info->cut_depth))
    {
      filled_matchhistory_bits = frontptr->matchhistory_bits;
    } else
    {
      int shift = GT_MULT2(pol_info->cut_depth) - frontptr->matchhistory_size;
      uint64_t fill_bits = ((uint64_t) 1 << shift) - 1;
      filled_matchhistory_bits = frontptr->matchhistory_bits |
                                 (fill_bits << frontptr->matchhistory_size);
    }
    if (alignedlen > best_polished_point->alignedlen &&
        GT_HISTORY_IS_POLISHED(pol_info,filled_matchhistory_bits))
    {
      best_polished_point->alignedlen = alignedlen;
      best_polished_point->row = frontptr->row;
      best_polished_point->distance = distance;
      best_polished_point->trimleft = trimleft;
      best_polished_point->max_mismatches = (GtUword) frontptr->max_mismatches;
      if (showfrontinfo)
      {
        printf("new polished point (alignlen=" GT_WU ",row=%" PRIu32
               ",distance=" GT_WU ")\n",alignedlen,frontptr->row,distance);
      }
    }
    if (front_trace != NULL)
    {
      front_trace_add_trace(front_trace,frontptr->backreference,
                            frontptr->localmatch_count);
    }
  }
}

static void showcurrentfront(const GtFtFrontvalue *validbasefront,
                             GtUword trimleft,
                             GtUword valid,
                             GtUword distance)
{
  const GtFtFrontvalue *ptr, *ptr_maxalignedlen = NULL,
                       *midfront = validbasefront + distance;
  GtUword maxalignedlen = 0;

  for (ptr = validbasefront + trimleft;
       ptr < validbasefront + trimleft + valid; ptr++)
  {
    GtUword thisalignedlen = GT_MULT2(ptr->row) + FRONT_DIAGONAL(ptr);
    if (thisalignedlen > maxalignedlen)
    {
      ptr_maxalignedlen = ptr;
      maxalignedlen = thisalignedlen;
    }
  }
  for (ptr = validbasefront + trimleft;
       ptr < validbasefront + trimleft + valid; ptr++)
  {
    GtUword thisalignedlen = GT_MULT2(ptr->row) + FRONT_DIAGONAL(ptr);
    printf("front[h=" GT_WD "]=%" PRIu32 ",localmatchqual=%.2f,alignedlen="
           GT_WU ",back=",
                   FRONT_DIAGONAL(ptr),
                   ptr->row,
                   100.0 *
                   (double) ptr->matchhistory_count/ptr->matchhistory_size,
                   thisalignedlen);
    if (ptr->backreference & FT_EOP_DELETION)
    {
      printf("D");
    }
    if (ptr->backreference & FT_EOP_INSERTION)
    {
      printf("I");
    }
    if (ptr->backreference & FT_EOP_MISMATCH)
    {
      printf("R");
    }
    if (ptr == ptr_maxalignedlen)
    {
      printf("*****");
    }
    printf("\n");
  }
}

GtUword front_prune_edist_inplace(
#ifndef OUTSIDE_OF_GT
                         bool rightextension,
                         GtAllocatedMemory *frontspace,
#endif
                         GtFtTrimstat *trimstat,
                         GtFtPolished_point *best_polished_point,
                         GtFronttrace *front_trace,
                         const GtFtPolishing_info *pol_info,
                         GtTrimmingStrategy trimstrategy,
                         GtUword max_history,
                         GtUword minmatchpercentage,
                         GtUword maxalignedlendifference,
                         bool showfrontinfo,
                         GtUword seedlength,
                         FTsequenceResources *ufsr,
                         GtUword ustart,
                         GtUword ulen,
                         GtUword vseqstartpos,
                         FTsequenceResources *vfsr,
                         GtUword vstart,
                         GtUword vlen)
{
  const GtUword sumseqlength = ulen + vlen,
                minsizeforshift = sumseqlength/1000;
  /* so the space for allocating the fronts is
     sizeof (GtFtFrontvalue) * ((m+n)/1000 + maxvalid), where maxvalid is a
     small constant. */
  GtUword distance, trimleft = 0, valid = 1UL, maxvalid = 0, sumvalid = 0;
  const uint64_t leftmostbit = ((uint64_t) 1) << (max_history-1);
  GtFtFrontvalue *validbasefront;
  bool diedout = false;
  GtFtSequenceObject useq, vseq;

#ifdef OUTSIDE_OF_GT
  GtAllocatedMemory *frontspace = gt_malloc(sizeof *frontspace);
  frontspace->space = NULL;
  frontspace->allocated = 0;
  frontspace->offset = 0;
  ft_sequenceobject_init(&useq,useqptr,0,ustart,ulen);
  ft_sequenceobject_init(&vseq,vseqptr,vseqstartpos,vstart,vlen);
#else
  /*
  printf("%sextension:useq->readmode=%s,vseq->readmode=%s\n",
          rightextension ? "right" : "left",
          gt_readmode_show(ufsr->readmode),gt_readmode_show(ufsr->readmode));
  */
  ft_sequenceobject_init(&useq,
                         ufsr->extend_char_access,
                         ufsr->encseq,
                         rightextension,
                         ufsr->readmode,
                         0,
                         ustart,
                         ulen,
                         ufsr->encseq_r,
                         ufsr->sequence_cache,
                         NULL,
                         ufsr->totallength);
  ft_sequenceobject_init(&vseq,
                         vfsr->extend_char_access,
                         vfsr->encseq,
                         rightextension,
                         vfsr->readmode,
                         vseqstartpos,
                         vstart,
                         vlen,
                         vfsr->encseq_r,
                         vfsr->sequence_cache,
                         vfsr->bytesequence,
                         vfsr->totallength);
#ifdef SKDEBUG
  gt_greedy_show_context(rightextension,&useq,&vseq);
#endif
  frontspace->offset = 0;
#endif
  for (distance = 0, valid = 1UL; /* Nothing */; distance++, valid += 2)
  {
    GtUword trim, maxalignedlen, minlenfrommaxdiff;

    if (showfrontinfo)
    {
      printf("distance=" GT_WU ",full=" GT_WU ",trimleft=" GT_WU
             ",valid=" GT_WU,distance, GT_MULT2(distance) + 1, trimleft,valid);
      if (best_polished_point != NULL)
      {
        printf(",best pp=(align=" GT_WU ", row=" GT_WU ", distance=" GT_WU
                 ", max_mismatches=" GT_WU ")",
                  best_polished_point->alignedlen,
                  best_polished_point->row,
                  best_polished_point->distance,
                  best_polished_point->max_mismatches);
      }
      printf("\n");
    }
    gt_assert(valid <= GT_MULT2(distance) + 1);
    sumvalid += valid;
    if (maxvalid < valid)
    {
      maxvalid = valid;
    }
    validbasefront = frontspace_allocate(minsizeforshift,trimleft,valid,
                                         frontspace);
    if (distance == 0)
    {
      validbasefront->row = 0;
      if (seedlength >= sizeof (validbasefront->matchhistory_bits) * CHAR_BIT)
      {
        validbasefront->matchhistory_bits = ~((uint64_t) 0);
      } else
      {
        validbasefront->matchhistory_bits = (((uint64_t) 1) << seedlength) - 1;
      }
      validbasefront->matchhistory_size
        = validbasefront->matchhistory_count = MIN(max_history,seedlength);
      validbasefront->backreference = 0; /* No back reference */
      validbasefront->max_mismatches = 0;
      front_prune_add_matches(validbasefront + distance,validbasefront,
                              leftmostbit,max_history,&useq,&vseq);
      maxalignedlen = GT_MULT2(validbasefront->row);
    } else
    {
      gt_assert(valid >= 3UL);
      frontspace_check((const GtFtFrontvalue *) frontspace->space,
                       ((const GtFtFrontvalue *) frontspace->space)
                        + frontspace->allocated - 1,
                       validbasefront + trimleft);
      frontspace_check((const GtFtFrontvalue *) frontspace->space,
                       ((const GtFtFrontvalue *) frontspace->space)
                         + frontspace->allocated - 1,
                       validbasefront + trimleft + valid - 1);
      if (valid == 3UL)
      {
        maxalignedlen
          = front_second_inplace(validbasefront + distance,
                                 validbasefront + trimleft,
                                 max_history,
                                 &useq,
                                 &vseq);
      } else
      {
        maxalignedlen
          = front_next_inplace(validbasefront + distance,
                               validbasefront + trimleft,
                               validbasefront + trimleft + valid - 1,
                               max_history,
                               &useq,
                               &vseq);
      }
    }
    gt_assert(valid > 0);
    minlenfrommaxdiff = maxalignedlen >= maxalignedlendifference
                          ? maxalignedlen - maxalignedlendifference
                          : 0;
    if (showfrontinfo)
    {
      printf("maxalignedlen=" GT_WU ",maxlenfrommaxdiff=" GT_WU "\n",
             maxalignedlen,minlenfrommaxdiff);
      showcurrentfront(validbasefront, trimleft, valid,distance);
    }
    trim = trim_front(true,
                      distance,
                      ulen,
                      vlen,
                      minmatchpercentage,
                      minlenfrommaxdiff,
                      trimstrategy,
                      best_polished_point,
                      validbasefront + distance,
                      validbasefront + trimleft,
                      validbasefront + trimleft + valid,
                      showfrontinfo);
    if (trim > 0)
    {
      if (showfrontinfo)
      {
        printf("trim on left=" GT_WU "\n",trim);
      }
      trimleft += trim;
      gt_assert(valid >= trim);
      valid -= trim;
    }
    if (valid > 0)
    {
      trim = trim_front(false,
                        distance,
                        ulen,
                        vlen,
                        minmatchpercentage,
                        minlenfrommaxdiff,
                        trimstrategy,
                        best_polished_point,
                        validbasefront + distance,
                        validbasefront + trimleft + valid - 1,
                        validbasefront + trimleft - 1,
                        showfrontinfo);
      gt_assert(trim < valid);
      if (trim > 0)
      {
        if (showfrontinfo > 0)
        {
          printf("trim on right=" GT_WU "\n",trim);
        }
        gt_assert(valid >= trim);
        valid -= trim;
      }
    }
    if (valid == 0)
    {
      diedout = true;
      break;
    }
    if (front_trace != NULL)
    {
      front_trace_add_gen(front_trace,trimleft,valid);
    }
    update_trace_and_polished(best_polished_point,
#ifndef OUTSIDE_OF_GT
                              &useq.min_access_pos,
                              &vseq.min_access_pos,
#endif
                              front_trace,
                              pol_info,
                              distance,
                              trimleft,
                              validbasefront + distance,
                              validbasefront + trimleft,
                              validbasefront + trimleft + valid - 1,
                              showfrontinfo);
    if ((vlen > ulen && vlen - ulen <= distance) ||
        (vlen <= ulen && ulen - vlen <= distance))
    {
      if (distance + vlen - ulen >= trimleft &&
          distance + vlen - ulen <= trimleft + valid - 1 &&
          validbasefront[distance + vlen - ulen].row == ulen)
      {
        break;
      }
    }
    if (distance >= sumseqlength)
    {
      break;
    }
  }
  gt_ft_trimstat_add(trimstat,diedout,sumvalid,maxvalid,distance,
                     sizeof (GtFtFrontvalue) * frontspace->allocated,
#ifndef OUTSIDE_OF_GT
                     (useq.sequence_cache != NULL &&
                      vseq.sequence_cache != NULL)
                       ? MAX(useq.sequence_cache->allocated,
                             vseq.sequence_cache->allocated)
                       : 0
#else
                     0
#endif
              );
#ifdef OUTSIDE_OF_GT
  gt_free(frontspace->space);
  gt_free(frontspace);
#endif
  return diedout ? sumseqlength + 1 : distance;
}
