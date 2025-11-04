#pragma once

#include <stddef.h>
#include <stdint.h>

typedef struct Message Message;
typedef struct Channel Channel;

struct Message {
    enum {
        Tnone = 0,
        Tsome = 1,
        Tclose = 2
    } tag;
    intptr_t value;
};

Channel *channelcreate(uint8_t capacity);
void channeldestroy(Channel *c);
int channelput(Channel *c, Message *in);
int channelget(Channel *c, Message *out);
int channelsize(Channel *c);

uint64_t fnv(size_t datalen, unsigned char const *data);

typedef struct Table Table;

Table *tablecreate(size_t columns_len);
void tabledestroy(Table *t, void finalize(void *));
int tableput(Table *t, char const *key, void *value);
void *tableget(Table *t, char const *key);
int tabledel(Table *t, char const *key, void finalize(void *));
void tablecompact(Table *t);

void *aalloc(int n, int t);
void areset(int t);
void afree(int t);
