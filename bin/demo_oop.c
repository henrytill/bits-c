#include <stdio.h>
#include <stdlib.h>

#include "macro.h"

/// Base class.
typedef struct person person;

/// Base class methods.
typedef struct person_operations person_operations;

/// Derived class.
typedef struct student student;

struct person_operations {
	void (*hello)(person const *self);
};

struct person {
	person_operations const *ops;
	char                    *name;
	int                      age;
	person                  *next;
};

static void person_hello(person const *self)
{
	printf("Hello, my name is %s, I'm %d years old.\n",
		self->name,
		self->age);
}

/// Base class vtable.
static person_operations const PERSON_OPS = {
	.hello = person_hello,
};

struct student {
	person person;
	char  *school;
};

/// Derived class override of person_operations::hello
static void student_hello(person const *self)
{
	student const *s = CONTAINER_OF(self, student, person);
	printf("Hello, my name is %s, I'm %d years old, I'm a student of %s.\n",
		s->person.name,
		s->person.age,
		s->school);
}

/// Derived class vtable.
static person_operations const STUDENT_OPS = {
	.hello = student_hello,
};

int main(void)
{
	student carol = {
		.person = {
			.ops  = &STUDENT_OPS,
			.name = "Carol",
			.age  = 22,
			.next = NULL,
		},
		.school = "MIT",
	};
	person bob = {
		.ops  = &PERSON_OPS,
		.name = "Bob",
		.age  = 21,
		.next = &carol.person,
	};
	person alice = {
		.ops  = &PERSON_OPS,
		.name = "Alice",
		.age  = 20,
		.next = &bob,
	};

	for (person const *p = &alice; p != NULL; p = p->next) {
		SEND(p, ops->hello);
	}

	return EXIT_SUCCESS;
}
