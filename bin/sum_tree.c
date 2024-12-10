#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "allocator.h"
#include "feature.h" // IWYU pragma: keep

#ifdef HAS_BLOCKS
#    include <Block.h>
#endif

static int answer = -1;

typedef struct node node;

struct node
{
    int value;
    node *left;
    node *right;
};

ALLOCATOR_DEFINE(node)

node *node_create(node_allocator *alloc, int value, node *left, node *right)
{
    node *ret = node_alloc(alloc);
    ret->value = value;
    ret->left = left;
    ret->right = right;
    return ret;
}

void node_destroy(node *n)
{
    if (n == NULL)
    {
        return;
    }
    node_destroy(n->left);
    node_destroy(n->right);
    free(n);
}

/* recursive */

int recursive_sum(node *n)
{
    if (n == NULL)
    {
        return 0;
    }
    const int suml = recursive_sum(n->left);
    const int sumr = recursive_sum(n->right);
    return suml + sumr + n->value;
}

#ifdef HAS_NESTED_FUNCTIONS
/* cps with gcc nested functions */

void nested_sum_impl(node *n, void k(int))
{
    if (n == NULL)
    {
        k(0);
    }
    else
    {
        void k1(int s0)
        {
            void k2(int s1) { k(s0 + s1 + n->value); }
            nested_sum_impl(n->right, k2);
        }
        nested_sum_impl(n->left, k1);
    }
}

void nested_sum(node *n)
{
    extern int answer;

    void k3(int s) { answer = s; }
    nested_sum_impl(n, k3);
}
#endif

#ifdef HAS_BLOCKS
/* cps with clang blocks */

typedef void (^kontb)(int);

void blocks_sum_impl(node *n, kontb k)
{
    if (n == NULL)
    {
        k(0);
    }
    else
    {
        kontb k1 = Block_copy(^(int s0) {
            kontb k2 = Block_copy(^(int s1) { k(s0 + s1 + n->value); });
            blocks_sum_impl(n->right, k2);
            Block_release(k2);
        });
        blocks_sum_impl(n->left, k1);
        Block_release(k1);
    }
}

void blocks_sum(node *n)
{
    extern int answer;

    kontb k3 = ^(int s) { answer = s; };
    blocks_sum_impl(n, k3);
}
#endif

/* defunctionalization */

typedef enum kont_tag kont_tag;

enum kont_tag
{
    K1,
    K2,
    K3,
};

typedef struct kont kont;

struct kont
{
    kont_tag tag;
    union
    {
        struct
        {
            node *n;
            kont *k;
        } k1;
        struct
        {
            int s0;
            node *n;
            kont *k;
        } k2;
    } u;
};

ALLOCATOR_DEFINE(kont)

kont *defunc_kont_k1(kont_allocator *alloc, node *n, kont *k)
{
    kont *k1 = kont_alloc(alloc);
    k1->tag = K1;
    k1->u.k1.n = n;
    k1->u.k1.k = k;
    return k1;
}

kont *defunc_kont_k2(kont_allocator *alloc, int s0, node *n, kont *k)
{
    kont *k2 = kont_alloc(alloc);
    k2->tag = K2;
    k2->u.k2.s0 = s0;
    k2->u.k2.n = n;
    k2->u.k2.k = k;
    return k2;
}

kont *defunc_kont_k3(kont_allocator *alloc)
{
    kont *k3 = kont_alloc(alloc);
    k3->tag = K3;
    return k3;
}

void defunc_apply(kont_allocator *alloc, kont *k, int s);

void defunc_sum_impl(kont_allocator *alloc, node *n, kont *k)
{
    if (n == NULL)
    {
        defunc_apply(alloc, k, 0);
    }
    else
    {
        kont *k1 = defunc_kont_k1(alloc, n, k);
        defunc_sum_impl(alloc, n->left, k1);
    }
}

void defunc_apply(kont_allocator *alloc, kont *k, int s)
{
    extern int answer;

    if (k->tag == K1)
    {
        kont *k2 = defunc_kont_k2(alloc, s, k->u.k1.n, k->u.k1.k);
        defunc_sum_impl(alloc, k->u.k1.n->right, k2);
    }
    else if (k->tag == K2)
    {
        defunc_apply(alloc, k->u.k2.k, k->u.k2.s0 + s + k->u.k2.n->value);
    }
    else if (k->tag == K3)
    {
        answer = s;
    }
}

#define KONT_POOL_SIZE 128

void defunc_sum(node *n)
{
    kont_allocator *alloc = kont_allocator_create(KONT_POOL_SIZE);
    kont *k3 = defunc_kont_k3(alloc);
    defunc_sum_impl(alloc, n, k3);
    kont_allocator_destroy(alloc);
}

/*
 * optimized version
 * + tail-call elimination of apply
 * + inlined apply
 * + tail-call elimination of sum
 */
void opt_sum_impl(kont_allocator *alloc, node *n, kont *k)
{
    extern int answer;

    while (true)
    {
        if (n == NULL)
        {
            int s = 0;
            while (true)
            {
                if (k->tag == K1)
                {
                    n = k->u.k1.n->right;
                    k = defunc_kont_k2(alloc, s, k->u.k1.n, k->u.k1.k);
                    break;
                }
                if (k->tag == K2)
                {
                    s = k->u.k2.s0 + s + k->u.k2.n->value;
                    k = k->u.k2.k;
                }
                else if (k->tag == K3)
                {
                    answer = s;
                    return;
                }
            }
        }
        else
        {
            k = defunc_kont_k1(alloc, n, k);
            n = n->left;
        }
    }
}

void opt_sum(node *n)
{
    kont_allocator *alloc = kont_allocator_create(KONT_POOL_SIZE);
    kont *k3 = defunc_kont_k3(alloc);
    opt_sum_impl(alloc, n, k3);
    kont_allocator_destroy(alloc);
}

/* use stack */

typedef struct vkont vkont;

struct vkont
{
    kont_tag tag;
    union
    {
        struct
        {
            node *n;
        } k1;
        struct
        {
            int s0;
            node *n;
        } k2;
    } u;
};

struct vkont vkont_k1(node *n)
{
    struct vkont ret = {.tag = K1, .u = {.k1 = {.n = n}}};
    return ret;
}

struct vkont vkont_k2(int s0, node *n)
{
    struct vkont ret = {.tag = K2, .u = {.k2 = {.s0 = s0, .n = n}}};
    return ret;
}

struct vkont vkont_k3(void)
{
    struct vkont ret = {.tag = K3};
    return ret;
}

typedef struct vkont_stack vkont_stack;

struct vkont_stack
{
    size_t capacity;
    size_t top;
    struct vkont konts[];
};

vkont_stack *vkont_stack_create(size_t initial_capacity)
{
    vkont_stack *stack = calloc(1, sizeof(vkont_stack) + initial_capacity * sizeof(struct vkont));
    assert(stack != NULL);
    stack->capacity = initial_capacity;
    stack->top = 0;
    return stack;
}

void vkont_stack_destroy(vkont_stack *stack)
{
    free(stack);
}

bool vkont_stack_is_empty(vkont_stack *stack)
{
    return stack->top == 0;
}

void vkont_stack_resize(vkont_stack **stack_ptr)
{
    vkont_stack *stack = *stack_ptr;
    size_t new_capacity = stack->capacity * 2;
    vkont_stack *new_stack = realloc(stack, sizeof(vkont_stack) + new_capacity * sizeof(struct vkont));
    assert(new_stack != NULL);
    new_stack->capacity = new_capacity;
    *stack_ptr = new_stack;
}

void vkont_stack_push(vkont_stack **stack_ptr, struct vkont vk)
{
    vkont_stack *stack = *stack_ptr;
    if (stack->top == stack->capacity)
    {
        vkont_stack_resize(stack_ptr);
        stack = *stack_ptr;
    }
    stack->konts[stack->top++] = vk;
}

struct vkont vkont_stack_pop(vkont_stack *stack)
{
    assert(!vkont_stack_is_empty(stack));
    return stack->konts[--stack->top];
}

struct vkont *vkont_stack_peek(vkont_stack *stack)
{
    if (vkont_stack_is_empty(stack))
    {
        return NULL; // Or handle error as appropriate
    }
    return &stack->konts[stack->top - 1];
}

void stack_sum_impl(node *n, vkont_stack *ks)
{
    extern int answer;

    while (true)
    {
        if (n == NULL)
        {
            int s = 0;
            while (true)
            {
                vkont *k = vkont_stack_peek(ks);
                if (k->tag == K1)
                {
                    n = k->u.k1.n->right;
                    *k = vkont_k2(s, k->u.k1.n);
                    break;
                }
                if (k->tag == K2)
                {
                    s = k->u.k2.s0 + s + k->u.k2.n->value;
                    vkont_stack_pop(ks);
                }
                else if (k->tag == K3)
                {
                    answer = s;
                    return;
                }
            }
        }
        else
        {
            vkont_stack_push(&ks, vkont_k1(n));
            n = n->left;
        }
    }
}

void stack_sum(node *n)
{
    vkont_stack *ks = vkont_stack_create(128);
    vkont_stack_push(&ks, vkont_k3());
    stack_sum_impl(n, ks);
    vkont_stack_destroy(ks);
}

/* traditional stack iteration */

typedef struct stack_node stack_node;

struct stack_node
{
    node *node;
    stack_node *next;
};

void push(stack_node **top, node *n)
{
    stack_node *new = calloc(1, sizeof(*new));
    assert(new != NULL);
    new->node = n;
    new->next = *top;
    *top = new;
}

node *pop(stack_node **top)
{
    if (*top == NULL)
    {
        return NULL;
    };
    stack_node *tmp = *top;
    node *ret = tmp->node;
    *top = (*top)->next;
    free(tmp);
    return ret;
}

void iterative_sum(node *root)
{
    extern int answer;

    if (root == NULL)
    {
        answer = 0;
    }

    int sum = 0;
    stack_node *stack = NULL;
    push(&stack, root);

    node *curr = NULL;
    while (stack != NULL)
    {
        curr = pop(&stack);
        sum += curr->value;
        if (curr->right)
        {
            push(&stack, curr->right);
        }
        if (curr->left)
        {
            push(&stack, curr->left);
        }
    }

    answer = sum;
}

typedef struct algo algo;

struct algo
{
    char *name;
    void (*f)(node *n);
};

int main(void)
{
    extern int answer;

    algo algos[] = {
#ifdef HAS_NESTED_FUNCTIONS
        {"cps (w/nested functions)", &nested_sum},
#endif
#ifdef HAS_BLOCKS
        {"cps (w/blocks)", &blocks_sum},
#endif
        {"defunc", &defunc_sum},
        {"opt", &opt_sum},
        {"stack", &stack_sum},
        {"iterative", &iterative_sum},
        {NULL, NULL},
    };

#define NODE_POOL_SIZE 128

    node_allocator *alloc = node_allocator_create(NODE_POOL_SIZE);

#define BRANCH(value, left, right) node_create(alloc, value, left, right)
#define LEAF(value)                node_create(alloc, value, NULL, NULL)

    node *ns[] = {
        LEAF(123),
        BRANCH(1, LEAF(2), NULL),
        BRANCH(1, NULL, LEAF(2)),
        BRANCH(1, LEAF(200), LEAF(3)),
        BRANCH(1, LEAF(2), BRANCH(3, LEAF(4), LEAF(5))),
        BRANCH(1, BRANCH(3, LEAF(4), LEAF(5)), LEAF(2)),
        NULL,
    };

#undef LEAF
#undef BRANCH

    algo a = {0};
    node *n = NULL;
    int ref = 0;

    for (size_t i = 0; (a = algos[i]).name != NULL; ++i)
    {
        printf("=== %s ===\n", a.name);
        for (size_t j = 0; (n = ns[j]) != NULL; ++j)
        {
            ref = recursive_sum(n);
            answer = -1;
            a.f(n);
            assert(ref == answer);
            printf("sum: %d\n", answer);
        }
    }

    node_allocator_destroy(alloc);

    return EXIT_SUCCESS;
}
