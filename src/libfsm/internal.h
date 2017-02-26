/* $Id$ */

#ifndef FSM_INTERNAL_H
#define FSM_INTERNAL_H

#include <limits.h>

#include <fsm/fsm.h>

struct set;

/* TODO: +2 for SOL, EOL */
/* TODO: +lots for FSM_EDGE_* */
enum fsm_edge_type {
	FSM_EDGE_EPSILON = UCHAR_MAX + 1
};

#define FSM_EDGE_MAX FSM_EDGE_EPSILON

struct fsm_edge {
	struct set *sl;
	enum fsm_edge_type symbol;
};

struct fsm_state {
	unsigned int end:1;

	struct set *edges;

	void *opaque;

#ifdef DEBUG_TODFA
	struct set *nfasl;
#endif

	struct fsm_state *next;
};


struct fsm {
	struct fsm_state *sl;
	struct fsm_state **tail; /* tail of .sl */
	struct fsm_state *start;

	unsigned int tidy:1;

#ifdef DEBUG_TODFA
	struct fsm *nfa;
#endif
};

struct fsm_edge *
fsm_hasedge(const struct fsm_state *s, int c);

struct fsm_edge *
fsm_addedge(struct fsm_state *from, struct fsm_state *to, enum fsm_edge_type type);

#endif

