#include <stdlib.h>
#include <string.h>
#include <stdio.h>

void
panic(char *str);

typedef struct _RBTNode {
    struct _RBTNode *_left, *_right, *_parent;

    void *_key, *_value;
    unsigned char _color;
} _RBTNode;

typedef struct _Map {
    _RBTNode *_tree;

    int _keySize, _valueSize;
    int (*_cmp)(void *, void *);
} _Map;

_Map *
MapNew(int keySize, int valueSize, int(*cmp)(void *, void*));

int
MapInsert(_Map *mp, void *key, void *value);

int
MapFind(_Map *mp, void *key, void *value);

int
MapRemove(_Map *mp, void *key);

int
MapDestroy(_Map *mp);

void
MapPrint(_Map *mp, void (*printFunc)(void *, void *));
