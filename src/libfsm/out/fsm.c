/* $Id$ */

#include <ctype.h>
#include <assert.h>
#include <stdio.h>

#include <adt/set.h>

#include <fsm/fsm.h>
#include <fsm/pred.h>

#include "libfsm/internal.h"

#include "libfsm/out.h"

static unsigned int
indexof(const struct fsm *fsm, const struct fsm_state *state)
{
	struct fsm_state *s;
	unsigned int i;

	assert(fsm != NULL);
	assert(state != NULL);

	for (s = fsm->sl, i = 0; s != NULL; s = s->next, i++) {
		if (s == state) {
			return i;
		}
	}

	assert(!"unreached");
	return 0;
}

static int
escputc(int c, FILE *f)
{
	size_t i;

	const struct {
		int c;
		const char *s;
	} a[] = {
		{ '\\', "\\\\" },
		{ '\"', "\\\"" },

		{ '\f', "\\f"  },
		{ '\n', "\\n"  },
		{ '\r', "\\r"  },
		{ '\t', "\\t"  },
		{ '\v', "\\v"  }
	};

	assert(c != FSM_EDGE_EPSILON);
	assert(f != NULL);

	for (i = 0; i < sizeof a / sizeof *a; i++) {
		if (a[i].c == c) {
			return fputs(a[i].s, f);
		}
	}

	if (!isprint((unsigned char) c)) {
		return fprintf(f, "\\%03o", (unsigned char) c);
	}

	return fprintf(f, "%c", c);
}

/* TODO: centralise */
static const struct fsm_state *
findany(const struct fsm_state *state)
{
	struct fsm_state *fst, *cst;
	struct fsm_edge *e;
	struct set_iter it;

	assert(state != NULL);

	e = set_first(state->edges, &it);
	fst = set_first(e->sl, &it);

	for (e = set_first(state->edges, &it); e != NULL; e = set_next(&it)) {
		if (e->symbol > UCHAR_MAX) {
			break;
		}

		if (set_empty(e->sl)) {
			return NULL;
		}

		for (cst = set_first(e->sl, &it); cst != NULL; cst = set_next(&it)) {
			if (cst != fst) {
				return NULL;
			}
		}
	}

	assert(fst != NULL);

	return fst;
}

void
fsm_out_fsm(const struct fsm *fsm, FILE *f,
	const struct fsm_outoptions *options)
{
	struct fsm_state *s, *start;
	int end;

	assert(fsm != NULL);
	assert(f != NULL);
	assert(options != NULL);

	for (s = fsm->sl; s != NULL; s = s->next) {
		struct fsm_edge *e;
		struct set_iter it;

		{
			const struct fsm_state *a;

			a = findany(s);
			if (a != NULL) {
				fprintf(f, "%-2u -> %2u ?;\n", indexof(fsm, s), indexof(fsm, a));
				continue;
			}
		}

		for (e = set_first(s->edges, &it); e != NULL; e = set_next(&it)) {
			struct fsm_state *st;

			for (st = set_first(e->sl, &it); st != NULL; st = set_next(&it)) {
				assert(st != NULL);

				fprintf(f, "%-2u -> %2u", indexof(fsm, s), indexof(fsm, st));

				/* TODO: print " ?" if all edges are equal */

				switch (e->symbol) {
				case FSM_EDGE_EPSILON:
					break;

				default:
					fputs(" \"", f);
					escputc(e->symbol, f);
					putc('\"', f);
					break;
				}

				fprintf(f, ";\n");
			}
		}
	}

	fprintf(f, "\n");

	start = fsm_getstart(fsm);
	if (start == NULL) {
		return;
	}

	fprintf(f, "start: %u;\n", indexof(fsm, start));

	end = 0;
	for (s = fsm->sl; s != NULL; s = s->next) {
		end += !!fsm_isend(fsm, s);
	}

	if (end == 0) {
		return;
	}

	fprintf(f, "end:   ");
	for (s = fsm->sl; s != NULL; s = s->next) {
		if (fsm_isend(fsm, s)) {
			end--;

			fprintf(f, "%u%s", indexof(fsm, s), end > 0 ? ", " : ";\n");
		}
	}
}

