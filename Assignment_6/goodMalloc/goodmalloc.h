#ifndef ASSIGNMENT6_GOODMALLOC_H
#define ASSIGNMENT6_GOODMALLOC

#include <string>

// listNode struct for doubly linked list
struct listNode {
    int val;
    struct listNode *next;
    struct listNode *prev;
};


void createMem(int size);

void createList(std::string const &name, int numElems);

void assignVal(std::string const &name, int index, int val);

void freeElem(std::string const &name = "");

void startScope();

void endScope();

void renameListScope(std::string const &name, int presentScope, std::string const &newName, int newScope);

struct listNode *getListElement(struct listNode* start, int index);
int getCurrentScope();
struct listNode *getList(std::string const &name, int scope);
#endif //ASSIGNMENT6_GOODMALLOC_H
