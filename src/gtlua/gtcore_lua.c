/*
  Copyright (c) 2007-2008 Gordon Gremme <gremme@zbh.uni-hamburg.de>
  Copyright (c) 2007-2008 Center for Bioinformatics, University of Hamburg

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

#include <assert.h>
#include "gtlua/alpha_lua.h"
#include "gtlua/bittab_lua.h"
#include "gtlua/gtcore_lua.h"
#include "gtlua/range_lua.h"
#include "gtlua/score_matrix_lua.h"
#include "gtlua/translate_lua.h"

int gt_lua_open_core(lua_State *L)
{
  assert(L);
  gt_lua_open_alpha(L);
  gt_lua_open_bittab(L);
  gt_lua_open_range(L);
  gt_lua_open_score_matrix(L);
  gt_lua_open_translate(L);
  return 1;
}
