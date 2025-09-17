#include <stdio.h>
#include <stdlib.h>

#include "macro.h"

/* Base class. */
typedef struct person person;

/* Base class methods. */
typedef struct person_ops person_ops;

/* Derived class. */
typedef struct student student;

struct person_ops {
	void (*hello)(person const *self);
};

struct person {
	person_ops const *ops;
	char *name;
	int age;
	person *next;
};

static void
person_hello(person const *self)
{
	printf("Hello, my name is %s, I'm %d years old.\n",
		self->name,
		self->age);
}

/* Base class vtable. */
static person_ops const PERSON_OPS = {
	person_hello};

struct student {
	person person;
	char *school;
};

/* Derived class override of person_ops::hello */
static void
student_hello(person const *self)
{
	student const *s = CONTAINER_OF(self, student, person);
	printf("Hello, my name is %s, I'm %d years old, I'm a student of %s.\n",
		s->person.name,
		s->person.age,
		s->school);
}

/* Derived class vtable. */
static person_ops const STUDENT_OPS = {
	student_hello};

int
main(void)
{
	student carol;
	person bob, alice;
	person const *p;

	/* Initialize carol */
	carol.person.ops = &STUDENT_OPS;
	carol.person.name = "Carol";
	carol.person.age = 22;
	carol.person.next = NULL;
	carol.school = "MIT";

	/* Initialize bob */
	bob.ops = &PERSON_OPS;
	bob.name = "Bob";
	bob.age = 21;
	bob.next = &carol.person;

	/* Initialize alice */
	alice.ops = &PERSON_OPS;
	alice.name = "Alice";
	alice.age = 20;
	alice.next = &bob;

	for(p = &alice; p != NULL; p = p->next) {
		SEND(p, ops->hello);
	}

	return EXIT_SUCCESS;
}
