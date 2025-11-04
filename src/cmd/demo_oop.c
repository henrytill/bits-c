#include <stdio.h>
#include <stdlib.h>

#include "macro.h"

/* Base class. */
typedef struct Person Person;

/* Base class methods. */
typedef struct Personops Personops;

/* Derived class. */
typedef struct Student Student;

struct Personops
{
    void (*hello)(Person const *self);
};

struct Person
{
    Personops const *ops;
    char *name;
    int age;
    Person *next;
};

static void phello(Person const *self)
{
    printf("Hello, my name is %s, I'm %d years old.\n", self->name, self->age);
}

/* Base class vtable. */
static Personops const pops = {
    phello,
};

struct Student
{
    Person person;
    char *school;
};

/* Derived class override of Personops::hello */
static void shello(Person const *self)
{
    Student const *s = CONTAINEROF(self, Student, person);
    printf(
        "Hello, my name is %s, I'm %d years old, I'm a student of %s.\n",
        s->person.name,
        s->person.age,
        s->school);
}

/* Derived class vtable. */
static Personops const sops = {
    shello,
};

int main(void)
{
    Student carol;
    Person bob, alice;
    Person const *p;

    /* Initialize carol */
    carol.person.ops = &sops;
    carol.person.name = "Carol";
    carol.person.age = 22;
    carol.person.next = NULL;
    carol.school = "MIT";

    /* Initialize bob */
    bob.ops = &pops;
    bob.name = "Bob";
    bob.age = 21;
    bob.next = &carol.person;

    /* Initialize alice */
    alice.ops = &pops;
    alice.name = "Alice";
    alice.age = 20;
    alice.next = &bob;

    for (p = &alice; p != NULL; p = p->next)
        SEND(p, ops->hello);

    return EXIT_SUCCESS;
}
