/* $Id$ */

#include <assert.h>
#include <stdlib.h>

#include <adt/set.h>

#include <fsm/fsm.h>

#include "internal.h"

struct fsm_state *
fsm_mergestates(struct fsm *fsm, struct fsm_state *a, struct fsm_state *b)
{
	struct fsm_edge *e, *f;
	struct fsm_state *s;
	struct set_iter it;

	/* edges from b */
	for (e = set_first(b->edges, &it); e != NULL; e = set_next(&it)) {
		f = fsm_hasedge(a, e->symbol);
		if (f == NULL) {
			continue;
		}

		set_merge(&e->sl, f->sl);
		f->sl = NULL;
	}

	/* edges to b */
	for (s = fsm->sl; s != NULL; s = s->next) {
		for (e = set_first(s->edges, &it); e != NULL; e = set_next(&it)) {
			if (set_contains(e->sl, b)) {
				fsm_addedge(s, a, e->symbol);
				set_remove(&e->sl, b);
			}
		}
	}

	fsm_removestate(fsm, b);

	return a;
}

