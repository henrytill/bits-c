#include <stdio.h>
#include <stdlib.h>

#include "feature.h"

#ifdef HAS_BLOCKS
#  include <Block.h>
#endif

typedef struct node node;

struct node {
  int value;
  node *left;
  node *right;
};

node *node_create(int value, node *left, node *right) {
  node *ret = calloc(1, sizeof(*ret));
  if (ret == NULL) {
    return NULL;
  }
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

int node_sum(node *n) {
  if (n == NULL) {
    return 0;
  }
  const int suml = node_sum(n->left);
  const int sumr = node_sum(n->right);
  return suml + sumr + n->value;
}

#ifdef HAS_NESTED_FUNCTIONS
void node_sum_cps_impl(node *n, void k(int)) {
  if (n == NULL) {
    k(0);
  } else {
    void k1(int s0) {
      void k2(int s1) { k(s0 + s1 + n->value); }
      node_sum_cps_impl(n->right, k2);
    }
    node_sum_cps_impl(n->left, k1);
  }
}

int node_sum_cps(node *n) {
  int answer = -1;
  void k(int s) {
    answer = s;
  }
  node_sum_cps_impl(n, k);
  return answer;
}
#endif

#ifdef HAS_BLOCKS
typedef void (^kont)(int);

void node_sum_cps_blocks_impl(node *n, kont k) {
  if (n == NULL) {
    k(0);
  } else {
    kont k1 = Block_copy(^(int s0) {
      kont k2 = Block_copy(^(int s1) { k(s0 + s1 + n->value); });
      node_sum_cps_blocks_impl(n->right, k2);
      Block_release(k2);
    });
    node_sum_cps_blocks_impl(n->left, k1);
    Block_release(k1);
  }
}

int node_sum_cps_blocks(node *n) {
  __block int answer = -1;
  node_sum_cps_blocks_impl(n, ^(int s) { answer = s; });
  return answer;
}
#endif

typedef struct stack_node stack_node;

struct stack_node {
  node *node;
  stack_node *next;
};

void push(stack_node **top, node *n) {
  stack_node *new = calloc(1, sizeof(*new));
  if (new == NULL) {
    exit(EXIT_FAILURE);
  }
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

int node_sum_iterative(node *root) {
  if (root == NULL) {
    return 0;
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

  return sum;
}

#define BRANCH(value, left, right) node_create(value, left, right)
#define LEAF(value)                node_create(value, NULL, NULL)

typedef struct algo algo;

struct algo {
  char *name;
  int (*f)(node *n);
};

int main(void) {
  algo algos[] = {
    {"recursive", &node_sum},
#ifdef HAS_NESTED_FUNCTIONS
    {"cps (w/nested functions)", &node_sum_cps},
#endif
#ifdef HAS_BLOCKS
    {"cps (w/blocks)", &node_sum_cps_blocks},
#endif
    {"iterative", &node_sum_iterative},
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
  int sum = 0;

  for (size_t i = 0; (a = algos[i]).name != NULL; ++i) {
    printf("=== %s ===\n", a.name);
    for (size_t j = 0; (n = ns[j]) != NULL; ++j) {
      sum = a.f(n);
      printf("sum: %d\n", sum);
    }
  }

  for (size_t i = 0; (n = ns[i]) != NULL; ++i) {
    node_destroy(n);
  }

  return EXIT_SUCCESS;
}
