/* $Id$ */

#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>

#include <adt/set.h>
#include <adt/xalloc.h>

#include <fsm/fsm.h>

#include "internal.h"

struct fsm_edge *
fsm_addedge(struct fsm_state *from, struct fsm_state *to, enum fsm_edge_type type)
{
	struct fsm_edge *e, new;

	assert(from != NULL);
	assert(to != NULL);

	new.symbol = type;
	e = set_contains(from->edges, &new);
	if (e == NULL) {
		e = malloc(sizeof *e);
		if (e == NULL) {
			return NULL;
		}

		e->symbol = type;
		e->sl = NULL;
	}

	(void)set_add(&from->edges, e);
	if (set_add(&e->sl, to) == NULL) {
		return NULL;
	}

	return e;
}

struct fsm_edge *
fsm_addedge_epsilon(struct fsm *fsm,
	struct fsm_state *from, struct fsm_state *to)
{
	assert(fsm != NULL);
	assert(from != NULL);
	assert(to != NULL);

	(void) fsm;

	return fsm_addedge(from, to, FSM_EDGE_EPSILON);
}

struct fsm_edge *
fsm_addedge_any(struct fsm *fsm,
	struct fsm_state *from, struct fsm_state *to)
{
	struct fsm_edge *e;
	int i;

	assert(fsm != NULL);
	assert(from != NULL);
	assert(to != NULL);

	(void) fsm;

	for (i = 0; i <= UCHAR_MAX; i++) {
		if (!(e = fsm_addedge(from, to, i))) {
			break;
		}
	}

	return e;
}

struct fsm_edge *
fsm_addedge_literal(struct fsm *fsm,
	struct fsm_state *from, struct fsm_state *to,
	char c)
{
	assert(fsm != NULL);
	assert(from != NULL);
	assert(to != NULL);

	(void) fsm;

	return fsm_addedge(from, to, (unsigned char) c);
}

struct fsm_edge *
fsm_hasedge(const struct fsm_state *s, int c)
{
	struct fsm_edge *e, search;

	search.symbol = c;
	e = set_contains(s->edges, &search);
	if (e == NULL || set_empty(e->sl)) {
		return NULL;
	}

	return e;
}
