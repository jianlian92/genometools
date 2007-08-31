/*
  Copyright (c) 2005-2007 Gordon Gremme <gremme@zbh.uni-hamburg.de>
  Copyright (c) 2005-2007 Center for Bioinformatics, University of Hamburg
  See LICENSE file or http://genometools.org/license.html for license details.
*/

#ifndef ARRAY_H
#define ARRAY_H

#include <stdlib.h>
#include <libgtcore/env.h>

typedef struct Array Array;

Array*        array_new(size_t, Env*);
Array*        array_clone(const Array*, Env*);
void*         array_get(const Array*, unsigned long);
void*         array_get_first(const Array*);
void*         array_get_last(const Array*);
void*         array_pop(Array*);
void*         array_get_space(const Array*);
#define       array_add(a, elem, env)\
              array_add_elem(a, &(elem), sizeof (elem), env)
void          array_add_elem(Array*, void*, size_t, Env*);
void          array_add_array(Array*, const Array*, Env*);
void          array_rem(Array*, unsigned long); /* O(n) */
void          array_reverse(Array*, Env*);
void          array_set_size(Array*, unsigned long);
void          array_reset(Array*);
size_t        array_elem_size(const Array*);
unsigned long array_size(const Array*);
int           array_example(Env*);
int           array_unit_test(Env*);
void          array_delete(Array*, Env*);

#endif
