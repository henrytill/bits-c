#include <assert.h>
#include <limits.h>
#include <stdlib.h>

#include "bits.h"

struct Expr {
	enum {
		Tvar,
		Tlam,
		Tapp
	} tag;

	union {
		struct {
			char *name;
		} var;

		struct {
			char *param;
			struct Expr *body;
		} lam;

		struct {
			struct Expr *fun;
			struct Expr *arg;
		} app;
	} u;
};

static struct Expr *
varcreate(char const *name)
{
	struct Expr *e;
	size_t len;

	e = aalloc(sizeof(*e), 0);
	if(e == NULL)
		return NULL;

	len = strlen(name) + 1;
	e->u.var.name = aalloc((assert(len <= INT_MAX), (int)len), 0);
	if(e->u.var.name == NULL)
		return NULL;

	memcpy(e->u.var.name, name, len);
	e->tag = Tvar;

	return e;
}

static struct Expr *
lamcreate(char const *param, struct Expr *body)
{
	struct Expr *e;
	size_t len;

	e = aalloc(sizeof(*e), 0);
	if(e == NULL)
		return NULL;

	len = strlen(param) + 1;
	e->u.lam.param = aalloc((assert(len <= INT_MAX), (int)len), 0);
	if(e->u.lam.param == NULL)
		return NULL;

	memcpy(e->u.lam.param, param, len);
	e->u.lam.body = body;
	e->tag = Tlam;

	return e;
}

static struct Expr *
appcreate(struct Expr *fun, struct Expr *arg)
{
	struct Expr *e;

	e = aalloc(sizeof(*e), 0);
	if(e == NULL)
		return NULL;

	e->u.app.fun = fun;
	e->u.app.arg = arg;
	e->tag = Tapp;

	return e;
}

struct Stackitem {
	enum {
		Aexpr,
		Alamclose,
		Aappspace,
		Aappclose
	} action;
	struct Expr *expr;
	struct Stackitem *next;
};

static int
push(struct Stackitem **stack, int action, struct Expr *expr)
{
	struct Stackitem *item;

	item = aalloc(sizeof(*item), 1);
	if(item == NULL) {
		eprintf("allocation failed");
		return -1;
	}
	item->action = action;
	item->expr = expr;
	item->next = *stack;
	*stack = item;
	return 0;
}

static void
show(struct Expr *e, FILE *out) /* NOLINT(readability-function-cognitive-complexity) */
{
	struct Stackitem *stack = NULL;
	struct Stackitem *item;
	int rc;

	rc = push(&stack, Aexpr, e);
	if(rc < 0)
		return;

	while(stack != NULL) {
		item = stack;
		stack = stack->next;

		switch(item->action) {
		case Aexpr:
			switch(item->expr->tag) {
			case Tvar:
				(void)fprintf(out, "%s", item->expr->u.var.name);
				break;
			case Tlam:
				(void)fprintf(out, "(\\%s . ", item->expr->u.lam.param);

				rc = push(&stack, Alamclose, NULL);
				if(rc < 0)
					return;

				rc = push(&stack, Aexpr, item->expr->u.lam.body);
				if(rc < 0)
					return;

				break;
			case Tapp:
				(void)fprintf(out, "(");

				rc = push(&stack, Aappclose, NULL);
				if(rc < 0)
					return;

				rc = push(&stack, Aexpr, item->expr->u.app.arg);
				if(rc < 0)
					return;

				rc = push(&stack, Aappspace, NULL);
				if(rc < 0)
					return;

				rc = push(&stack, Aexpr, item->expr->u.app.fun);
				if(rc < 0)
					return;

				break;
			}
			break;
		case Alamclose:
			(void)fprintf(out, ")");
			break;
		case Aappspace:
			(void)fprintf(out, " ");
			break;
		case Aappclose:
			(void)fprintf(out, ")");
			break;
		}
	}

	(void)fprintf(out, "\n");
	areset(1);
}

int
main(void)
{
	int ret = EXIT_FAILURE;
	struct Expr *xvar, *yvar, *tvar;
	struct Expr *id, *kinner, *k;
	struct Expr *app;

	/* (\x . x) */
	xvar = varcreate("x");
	if(xvar == NULL) {
		eprintf("failed to create variable");
		goto freearenas;
	}
	id = lamcreate("x", xvar);
	if(id == NULL) {
		eprintf("failed to create lambda");
		goto freearenas;
	}

	show(id, stdout);

	/* (\t . (\f . t)) */
	tvar = varcreate("t");
	if(tvar == NULL) {
		eprintf("failed to create variable");
		goto freearenas;
	}
	kinner = lamcreate("f", tvar);
	if(kinner == NULL) {
		eprintf("failed to create lambda");
		goto freearenas;
	}
	k = lamcreate("t", kinner);
	if(k == NULL) {
		eprintf("failed to create lambda");
		goto freearenas;
	}

	show(k, stdout);

	/* ((\x . x) y) */
	yvar = varcreate("y");
	if(yvar == NULL) {
		eprintf("failed to create variable");
		goto freearenas;
	}
	app = appcreate(id, yvar);
	if(app == NULL) {
		eprintf("failed to create application");
		goto freearenas;
	}

	show(app, stdout);

	ret = EXIT_SUCCESS;

freearenas:
	afree(1);
	afree(0);
	return ret;
}
