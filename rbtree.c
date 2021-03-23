#include "rbtree.h"

void
panic(char *str) {
    fprintf(stderr, "%s\n", str);
    exit(1);
}

enum {
    RED,
    BLACK
};

static
void
CopyKey(void *to, void *from,
        _Map *mp) {
    memmove(to, from, mp->_keySize);
}

static
void
CopyValue(void *to, void *from,
        _Map *mp) {
    memmove(to, from, mp->_valueSize);
}

static
void
freeNode(_RBTNode *node) {
    if (node == NULL) {
        panic("freing NULL node");
    }

    free(node->_key);
    free(node->_value);
    free(node);
}

static
_RBTNode *
newNill() {
    _RBTNode *node;
    node = malloc(sizeof(_RBTNode));

    if (node == NULL) {
        return NULL;
    }

    node->_left = NULL;
    node->_right = NULL;
    node->_parent = NULL;

    node->_color = BLACK;

    return node;
}

static
int
isNill(_RBTNode *node) {
    if (node == NULL) {
        panic("checking NULL node if its nill");
    }

    return node->_left == NULL && node->_right == NULL;
}

static
_RBTNode *
insertNode(void *key, void *value,
        _RBTNode *node, _Map *mp) {
    if (node == NULL) {
        panic("inserting node in NULL tree");
    }

    node->_left = newNill();
    node->_right = newNill();

    if (node->_left == NULL || 
        node->_right == NULL) {
        free(node->_left);
        free(node->_right);

        node->_left = NULL;
        node->_right = NULL;
        return NULL;
    }

    node->_left->_parent = node;
    node->_right->_parent = node;

    node->_key = malloc(mp->_keySize);
    node->_value = malloc(mp->_valueSize);
    
    if (node->_key == NULL || 
        node->_value == NULL) {
        free(node->_left);
        free(node->_right);
        free(node->_key);
        free(node->_value);

        node->_left = NULL;
        node->_right = NULL;
        return NULL;
    }

    CopyKey(node->_key, key, mp);
    CopyValue(node->_value, value, mp);

    node->_color = RED;
    
    return node;
}

static
void
copyNode(_RBTNode *to, _RBTNode *from,
        _Map *mp) {
    if (to == NULL || from == NULL) {
        panic("copying NULL nodes");
    } else if (to->_key == NULL || from->_key == NULL) {
        panic("copying from nill");
    }

    CopyKey(to->_key, from->_key, mp);
    CopyValue(to->_value, from->_value, mp);
}

static
int
isLeft(_RBTNode *node) {
    if (node == NULL) {
        panic("checking if NULL node is left child");
    } else if (node->_parent == NULL) {
        panic("checking if root if left child");
    }

    return node == node->_parent->_left;
}

static
int
isRight(_RBTNode *node) {
    if (node == NULL) {
        panic("checking if NULL node is right child");
    } else if (node->_parent == NULL) {
        panic("checking if root is right child");
    }

    return node == node->_parent->_right;
}

static
_RBTNode *
Parent(_RBTNode *node) {
    if (node == NULL) {
        return NULL;
    }

    return node->_parent;
}

static
_RBTNode *
Grandparent(_RBTNode *node) {
    if (Parent(node) == NULL) {
        return NULL;
    }

    return Parent(node)->_parent;
}

static
_RBTNode *
Sibling(_RBTNode *node) {
    if (Parent(node) == NULL) {
        return NULL;
    }

    if (isLeft(node)) {
        return Parent(node)->_right;
    } else {
        return Parent(node)->_left;
    }
}

static
_RBTNode *
Uncle(_RBTNode *node) {
    if (Grandparent(node) == NULL) {
        return NULL;
    }

    if (isLeft(Parent(node))) {
        return Grandparent(node)->_right;
    } else {
        return Grandparent(node)->_left;
    }
}

static
_RBTNode *
Root(_RBTNode *node) {
    if (node == NULL) {
        return NULL;
    }

    while (Parent(node) != NULL) {
        node = node->_parent;
    }
    return node;
}

static
_RBTNode *
Predecessor(_RBTNode *node) {
    if (node == NULL) {
        panic("NULL node in Predecessor");
    }

    if (isNill(node)) {
        return NULL;
    }

    node = node->_left;
    while (!isNill(node)) {
        node = node->_right;
    }

    return node->_parent;
}

static
_RBTNode *
rotateRight(_RBTNode *node) {
    _RBTNode *pivot = node->_left;

    pivot->_parent = Parent(node);
    if (Parent(node) != NULL) {
        if (isLeft(node)) {
            Parent(node)->_left = pivot;
        } else {
            Parent(node)->_right = pivot;
        }
    }

    node->_left = pivot->_right;
    if (node->_left != NULL) {
        node->_left->_parent = node;
    }

    node->_parent = pivot;
    pivot->_right = node;

    return node;
}

static
_RBTNode *
rotateLeft(_RBTNode *node) {
    _RBTNode *pivot = node->_right;

    pivot->_parent = Parent(node);
    if (Parent(node) != NULL) {
        if (isLeft(node)) {
            Parent(node)->_left = pivot;
        } else {
            Parent(node)->_right = pivot;
        }
    }

    node->_right = pivot->_left;
    if (node->_right != NULL) {
        node->_right->_parent = node;
    }

    node->_parent = pivot;
    pivot->_left = node;

    return node;
}

static
_RBTNode *
balanceInsert(_RBTNode *node) {
    if (node == NULL) {
        panic("NULL in balanceInsert");
    }

    if (Parent(node) == NULL) {
        node->_color = BLACK;
        return node;
    }

    if (Parent(node)->_color == BLACK) {
        return node;
    }

    //parent->_color == RED => grandparent != NULL

    if (Uncle(node)->_color == RED) {
        Parent(node)->_color = BLACK;
        Uncle(node)->_color = BLACK;
        Grandparent(node)->_color = RED;

        return balanceInsert(Grandparent(node));
    } else {
        if (isLeft(Parent(node)) && isRight(node)) {
            node = rotateLeft(Parent(node));
        } else if (isRight(Parent(node)) && isLeft(node)) {
            node = rotateRight(Parent(node));
        }

        Grandparent(node)->_color = RED;
        Parent(node)->_color = BLACK;

        if (isLeft(node)) {
            return rotateRight(Grandparent(node));
        } else {
            return rotateLeft(Grandparent(node));
        }
    }
}

static
_RBTNode *
add(void *key, void *value,
        _RBTNode *node, _Map *mp) {
    if (node == NULL) {
        panic("adding node to NULL tree");
    }

    int (*cmp)(void *, void *) = mp->_cmp;

    while (!isNill(node)) {
        if ((*cmp)(node->_key, key) < 0) {
            node = node->_right;
        } else if ((*cmp)(node->_key, key) > 0) {
            node = node->_left;
        } else {
            CopyValue(node->_value, value, mp);
            return node;
        }
    }

    _RBTNode *newNode = insertNode(key, value, node, mp);

    if (newNode == NULL) {
        return NULL;
    }

    return balanceInsert(newNode);
}

static
_RBTNode *
find(void *key,
        _RBTNode *node, _Map *mp) {
    if (node == NULL) {
        panic("searching nodes in NULL tree");
    }

    int (*cmp)(void *, void *) = mp->_cmp;

    while (!isNill(node)) {
        if ((*cmp)(node->_key, key) < 0) {
            node = node->_right;
        } else if ((*cmp)(node->_key, key) > 0) {
            node = node->_left;
        } else {
            return node;
        }
    }

    return NULL;
}

static
_RBTNode *
balanceRemoval(_RBTNode *node) {
    if (node == NULL) {
        panic("NULL in balanceRemoval");
    }

    if (node->_color == RED) {
        node->_color = BLACK;
        return node;
    }

    if (Parent(node) == NULL) {
        return node;
    }

    if (Sibling(node)->_color == RED) {
        Sibling(node)->_color = BLACK;
        Parent(node)->_color = RED;

        if (isLeft(node)) {
            rotateLeft(Parent(node));
        } else {
            rotateRight(Parent(node));
        }
    }

    //Sibling(node)->_color == BLACK;

    if (Sibling(node)->_left->_color == BLACK && 
        Sibling(node)->_right->_color == BLACK) {
        int parentColor = Parent(node)->_color;

        Sibling(node)->_color = RED;
        Parent(node)->_color = BLACK;

        if (parentColor == BLACK) {
            return balanceRemoval(Parent(node));
        } else {
            return node;
        }
    }

    if (isLeft(node)) {
        if (Sibling(node)->_left->_color == RED && 
            Sibling(node)->_right->_color == BLACK) {
            
            Sibling(node)->_color = RED;
            Sibling(node)->_left->_color = BLACK;

            rotateRight(Sibling(node));
        }

        Sibling(node)->_color = Parent(node)->_color;
        Parent(node)->_color = BLACK;
        Sibling(node)->_right->_color = BLACK;

        return rotateLeft(Parent(node));

    } else if (isRight(node)) {
        if (Sibling(node)->_right->_color == RED && 
            Sibling(node)->_left->_color == BLACK) {

            Sibling(node)->_color = RED;
            Sibling(node)->_right->_color = BLACK;

            rotateLeft(Sibling(node));
        }

        Sibling(node)->_color = Parent(node)->_color;
        Parent(node)->_color = BLACK;
        Sibling(node)->_left->_color = BLACK;

        return rotateRight(Parent(node));
    }
}

static
_RBTNode *
erase(void *key,
        _RBTNode *node, _Map *mp) {
    if (node == NULL) {
        panic("removing node from NULL tree");
    }

    int (*cmp)(void *, void *) = mp->_cmp;

    while (!isNill(node)) {
        if ((*cmp)(node->_key, key) < 0) {
            node = node->_right;
        } else if ((*cmp)(node->_key, key) > 0) {
            node = node->_left;
        } else {
            if (!isNill(node->_left) && !isNill(node->_right)) {
                _RBTNode *pred = Predecessor(node);
                
                copyNode(node, pred, mp);

                key = pred->_key;
                node = node->_left;
            } else {
                _RBTNode *child, *unusedNill;

                if (!isNill(node->_left)) {
                    child = node->_left;
                    unusedNill = node->_right;
                } else {
                    child = node->_right;
                    unusedNill = node->_left;
                }

                child->_parent = Parent(node);
                if (Parent(node) != NULL) {
                    if (isLeft(node)) {
                        Parent(node)->_left = child;
                    } else {
                        Parent(node)->_right = child;
                    }
                }

                int erasedColor = node->_color;

                freeNode(node);
                freeNode(unusedNill);

                if (erasedColor == BLACK) {
                    return balanceRemoval(child);
                }
                return child;
            }
        }
    }

    return NULL;
}

static
void
freeTree(_RBTNode *node) {
    if (node == NULL) {
        return;
    }

    freeTree(node->_left);
    freeTree(node->_right);
    free(node);
}

static
void
PrintTree(_RBTNode *node, void (*printFunc)(void *, void *)) {
    if (isNill(node)) {
        return;
    }

    PrintTree(node->_left, printFunc);
    printFunc(node->_key, node->_value);
    PrintTree(node->_right, printFunc);
}

_Map *
MapNew(int keySize, int valueSize, int (*cmp)(void *, void *)) {
    _Map *map = malloc(sizeof(_Map));

    if (map == NULL) {
        return NULL;
    }

    map->_keySize = keySize;
    map->_valueSize = valueSize;
    map->_cmp = cmp;

    map->_tree = newNill();
    if (map->_tree == NULL) {
        free(map);
        return NULL;
    }

    return map;
}

int
MapInsert(_Map *mp, void *key, void *value) {
    if (mp == NULL) {
        return -1;
    }

    _RBTNode *node = add(key, value, mp->_tree, mp);

    if (node == NULL) {
        return -2;
    }

    mp->_tree = Root(node);
    return 0;
}

int
MapFind(_Map *mp, void *key, void *value) {
    if (mp == NULL) {
        return -1;
    }

    _RBTNode *node = find(key, mp->_tree, mp);

    if (node == NULL) {
        return -2;
    }

    CopyValue(value, node->_value, mp);
    return 0;
}

int
MapRemove(_Map *mp, void *key) {
    if (mp == NULL) {
        return -1;
    }

    _RBTNode *node = erase(key, mp->_tree, mp);

    if (node == NULL) {
        return -2;
    }

    mp->_tree = Root(node);
    return 0;
}

int
MapDestroy(_Map *mp) {
    if (mp == NULL) {
        return -1;
    }

    freeTree(mp->_tree);
    free(mp);
    return 0;
}

void
MapPrint(_Map *mp, void (*printFunc)(void *, void *)) {
    if (mp == NULL) {
        return;
    }
    PrintTree(mp->_tree, printFunc);
}
