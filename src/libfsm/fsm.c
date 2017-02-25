/* $Id$ */

#include <assert.h>
#include <stdlib.h>

#include <adt/set.h>

#include <fsm/fsm.h>

#include "internal.h"

static void
free_contents(struct fsm *fsm)
{
	struct fsm_state *next;
	struct fsm_state *s;

	assert(fsm != NULL);

	for (s = fsm->sl; s != NULL; s = next) {
		struct set_iter it;
		struct fsm_edge *e;
		next = s->next;

		for (e = set_first(s->edges, &it); e != NULL; e = set_next(&it)) {
			set_free(e->sl);
		}

		set_free(s->edges);
		free(s);
	}
}

struct fsm *
fsm_new(void)
{
	struct fsm *new;

	new = malloc(sizeof *new);
	if (new == NULL) {
		return NULL;
	}

	new->sl    = NULL;
	new->tail  = &new->sl;
	new->start = NULL;
	new->tidy  = 1;

#ifdef DEBUG_TODFA
	new->nfa   = NULL;
#endif

	return new;
}

void
fsm_free(struct fsm *fsm)
{
	assert(fsm != NULL);

	free_contents(fsm);

	free(fsm);
}

void
fsm_move(struct fsm *dst, struct fsm *src)
{
	assert(src != NULL);
	assert(dst != NULL);

	free_contents(dst);

	dst->sl    = src->sl;
	dst->tail  = src->tail;
	dst->start = src->start;

	free(src);
}

static int
fsm_edge_cmp(const void *a, const void *b)
{
	const struct fsm_edge *ea, *eb;

	assert(a != NULL);
	assert(b != NULL);

	ea = a;
	eb = b;

	return (ea->symbol > eb->symbol) - (ea->symbol < eb->symbol);
}

struct fsm_state *
fsm_addstate(struct fsm *fsm)
{
	struct fsm_state *new;

	assert(fsm != NULL);

	new = malloc(sizeof *new);
	if (new == NULL) {
		return NULL;
	}

	new->edges = set_create(fsm_edge_cmp);
	new->end = 0;
	new->opaque = NULL;

#ifdef DEBUG_TODFA
	new->nfasl = NULL;
#endif

	*fsm->tail = new;
	new->next = NULL;
	fsm->tail  = &new->next;

	return new;
}

void
fsm_removestate(struct fsm *fsm, struct fsm_state *state)
{
	struct fsm_state *s, **p;
	struct fsm_edge *e;
	struct set_iter it;

	assert(fsm != NULL);
	assert(state != NULL);

	for (s = fsm->sl; s != NULL; s = s->next) {
		for (e = set_first(s->edges, &it); e != NULL; e = set_next(&it)) {
			set_remove(&e->sl, state);
		}
	}

	for (e = set_first(state->edges, &it); e != NULL; e = set_next(&it)) {
		set_free(e->sl);
	}
	set_free(state->edges);

	if (fsm->start == state) {
		fsm->start = NULL;
	}

	for (p = &fsm->sl; *p != NULL; p = &(*p)->next) {
		if (*p == state) {
			struct fsm_state *next;

			next = (*p)->next;
			if (*fsm->tail == *p) {
				*fsm->tail = next;
			}
			free(*p);
			*p = next;
			break;
		}
	}
}

void
fsm_setend(struct fsm *fsm, struct fsm_state *state, int end)
{
	(void) fsm;

	assert(fsm != NULL);
	assert(state != NULL);

	state->end = !!end;
}

void
fsm_setopaque(struct fsm *fsm, struct fsm_state *state, void *opaque)
{
	(void) fsm;

	assert(fsm != NULL);
	assert(state != NULL);

	assert(state->end);

	state->opaque = opaque;
}

void *
fsm_getopaque(struct fsm *fsm, struct fsm_state *state)
{
	(void) fsm;

	assert(fsm != NULL);
	assert(state != NULL);

	assert(state->end);

	return state->opaque;
}

void
fsm_setstart(struct fsm *fsm, struct fsm_state *state)
{
	assert(fsm != NULL);

	fsm->start = state;
}

struct fsm_state *
fsm_getstart(const struct fsm *fsm)
{
	assert(fsm != NULL);

	return fsm->start;
}

