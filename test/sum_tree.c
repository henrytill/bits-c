#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "feature.h"

#ifdef HAS_BLOCKS
#  include <Block.h>
#endif

static int answer = -1;

typedef struct node node;

struct node {
  int value;
  node *left;
  node *right;
};

node *node_create(int value, node *left, node *right) {
  node *ret = calloc(1, sizeof(*ret));
  assert(ret != NULL);
  ret->value = value;
  ret->left = left;
  ret->right = right;
  return ret;
}

void node_destroy(node *n) {
  if (n == NULL) {
    return;
  }
  node_destroy(n->left);
  node_destroy(n->right);
  free(n);
}

/* recursive */

int recursive_sum(node *n) {
  if (n == NULL) {
    return 0;
  }
  const int suml = recursive_sum(n->left);
  const int sumr = recursive_sum(n->right);
  return suml + sumr + n->value;
}

#ifdef HAS_NESTED_FUNCTIONS
/* cps with gcc nested functions */

void nested_sum_impl(node *n, void k(int)) {
  if (n == NULL) {
    k(0);
  } else {
    void k1(int s0) {
      void k2(int s1) { k(s0 + s1 + n->value); }
      nested_sum_impl(n->right, k2);
    }
    nested_sum_impl(n->left, k1);
  }
}

void nested_sum(node *n) {
  void k3(int s) { answer = s; }
  nested_sum_impl(n, k3);
}
#endif

#ifdef HAS_BLOCKS
/* cps with clang blocks */

typedef void (^kontb)(int);

void blocks_sum_impl(node *n, kontb k) {
  if (n == NULL) {
    k(0);
  } else {
    kontb k1 = Block_copy(^(int s0) {
      kontb k2 = Block_copy(^(int s1) { k(s0 + s1 + n->value); });
      blocks_sum_impl(n->right, k2);
      Block_release(k2);
    });
    blocks_sum_impl(n->left, k1);
    Block_release(k1);
  }
}

void blocks_sum(node *n) {
  kontb k3 = ^(int s) { answer = s; };
  blocks_sum_impl(n, k3);
}
#endif

/* traditional stack iteration */

typedef struct stack_node stack_node;

struct stack_node {
  node *node;
  stack_node *next;
};

void push(stack_node **top, node *n) {
  stack_node *new = calloc(1, sizeof(*new));
  assert(new != NULL);
  new->node = n;
  new->next = *top;
  *top = new;
}

node *pop(stack_node **top) {
  if (*top == NULL) {
    return NULL;
  };
  stack_node *tmp = *top;
  node *ret = tmp->node;
  *top = (*top)->next;
  free(tmp);
  return ret;
}

void iterative_sum(node *root) {
  if (root == NULL) {
    answer = 0;
  }

  int sum = 0;
  stack_node *stack = NULL;
  push(&stack, root);

  node *curr = NULL;
  while (stack != NULL) {
    curr = pop(&stack);
    sum += curr->value;
    if (curr->right) {
      push(&stack, curr->right);
    }
    if (curr->left) {
      push(&stack, curr->left);
    }
  }

  answer = sum;
}

#define BRANCH(value, left, right) node_create(value, left, right)
#define LEAF(value)                node_create(value, NULL, NULL)

typedef struct algo algo;

struct algo {
  char *name;
  void (*f)(node *n);
};

int main(void) {
  algo algos[] = {
#ifdef HAS_NESTED_FUNCTIONS
    {"cps (w/nested functions)", &nested_sum},
#endif
#ifdef HAS_BLOCKS
    {"cps (w/blocks)", &blocks_sum},
#endif
    {"iterative", &iterative_sum},
    {NULL, NULL},
  };

  node *ns[] = {
    LEAF(123),
    BRANCH(1, LEAF(2), NULL),
    BRANCH(1, NULL, LEAF(2)),
    BRANCH(1, LEAF(200), LEAF(3)),
    BRANCH(1, LEAF(2), BRANCH(3, LEAF(4), LEAF(5))),
    BRANCH(1, BRANCH(3, LEAF(4), LEAF(5)), LEAF(2)),
    NULL,
  };

  algo a = {0};
  node *n = NULL;
  int ref = 0;

  for (size_t i = 0; (a = algos[i]).name != NULL; ++i) {
    printf("=== %s ===\n", a.name);
    for (size_t j = 0; (n = ns[j]) != NULL; ++j) {
      ref = recursive_sum(n);
      answer = -1;
      a.f(n);
      assert(ref == answer);
      printf("sum: %d\n", answer);
    }
  }

  for (size_t i = 0; (n = ns[i]) != NULL; ++i) {
    node_destroy(n);
  }

  return EXIT_SUCCESS;
}
