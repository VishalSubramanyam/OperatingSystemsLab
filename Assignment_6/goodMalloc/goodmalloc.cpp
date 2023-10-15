#include "goodmalloc.h"
#include <bits/stdc++.h>

using std::map, std::pair, std::string;
static int currentScope = 0;
void *memStart;
struct freeListNode *freeList = nullptr;
map<pair<string, int>, struct listNode *> symbolTable;
struct freeListNode {
    struct listNode *blockAddr;
    struct freeListNode *next;
};
struct listNode *freeListPop() {
    auto temp = freeList;
    freeList = temp->next;
    auto res = temp->blockAddr;
    free(temp);
    return res;
}
void freeListPush(struct listNode *node) {
    auto temp = (struct freeListNode *) malloc(sizeof(struct freeListNode));
    temp->blockAddr = node;
    assert(temp->blockAddr != (struct listNode *) 0x4);
    temp->next = freeList;
    freeList = temp;
}

struct listNode *getNextFreeBlock() {
    std::cout << "Inside getNextFreeBlock()" << std::endl;
    if (freeList == nullptr) {
        fprintf(stderr, "Out of memory\n");
        exit(-1);
    }
    auto res = freeListPop();
    std::cout << "Returning from getNextFreeBlock()" << std::endl;
    return res;
}

void createMem(int size) {
    std::cout << "Creating a " << size << "MB block of memory" << std::endl;
    memStart = malloc(size);
    // initialize symbol table
    // initialize free list
    auto numNodes = size / sizeof(struct listNode);
    for (int i = 0; i < numNodes; i++) {
        freeListPush(static_cast<struct listNode *>(memStart) + i);
    }
    std::cout << "Memory created" << std::endl;
}

void startScope() {
    currentScope++;
}

void endScope() {
    currentScope--;
}

void createList(std::string const &name, int numElems) {
    std::cout << "Creating the list " << name << " in scope " << currentScope << " with " << numElems << " elements" << std::endl;
    auto newList = getNextFreeBlock();
    struct listNode *oldTemp = nullptr;
    auto temp = newList;
    for (int i = 0; i < numElems - 1; i++) {
        temp->prev = oldTemp;
        temp->next = getNextFreeBlock();
        oldTemp = temp;
        temp = temp->next;
    }
    temp->prev = oldTemp;
    temp->next = nullptr;
    if (symbolTable.find({name, currentScope}) != symbolTable.end()) {
        std::cerr << "Error: variable " << name << " redefinition" << std::endl;
        exit(-1);
    }
    symbolTable.insert({{name, currentScope}, newList});
    std::cout << "List { " << name << ", " << currentScope << " } created" << std::endl;
}

// implement assignVal
void assignVal(std::string const &name, int index, int val) {
    std::cout << "Assigning " << val << " to the element at " << index << " of list { " << name << ", " << currentScope << " }" << std::endl;
    auto it = symbolTable.find({name, currentScope});
    if (it == symbolTable.end()) {
        std::cerr << "Error: variable " << name << " not found" << std::endl;
        exit(-1);
    }
    auto temp = it->second; // get the head of the list
    int i;
    for (i = 0; i < index; i++) {
        temp = temp->next; // get the node at the index
    }
    temp->val = val; // assign the value
    std::cout << "Value assigned" << std::endl;
}
static void deleteList(struct listNode *head) {
    if (head == nullptr) {
        return;
    }
    deleteList(head->next);
    freeListPush(head);
}
// implement freeElem
void freeElem(std::string const &name) {
    if (name.empty()) {
        std::cout << "Freeing all the nodes in the current scope and above" << std::endl;
        // free all the nodes in the current scope and above
        for (auto it = symbolTable.begin(); it != symbolTable.end();) {
            if (it->first.second >= currentScope) {
                deleteList(it->second);
                it = symbolTable.erase(it);
            } else {
                it++;
            }
        }
        std::cout << "All nodes freed" << std::endl;
        return;
    }
    std::cout << "Freeing the list { " << name << ", " << currentScope << " }" << std::endl;
    auto it = symbolTable.find({name, currentScope});
    if (it == symbolTable.end()) {
        std::cerr << "Error: variable " << name << " not found" << std::endl;
        exit(-1);
    }
    auto temp = it->second; // get the head of the list
    deleteList(temp);
    symbolTable.erase(it);
    std::cout << "List freed" << std::endl;
}

// implement renameListScope function
void renameListScope(std::string const &name, int presentScope, std::string const &newName, int newScope) {
    std::cout << "Renaming the list { " << name << ", " << presentScope << " } to { " << newName << ", " << newScope << " }" << std::endl;
    // find the list in the symbolTable, delete it, reinsert it using the
    // same name but newScope
    auto it = symbolTable.find({name, presentScope});
    if (it == symbolTable.end()) {
        std::cerr << "Error: variable " << name << " not found" << std::endl;
        exit(-1);
    }
    auto temp = it->second;
    symbolTable.erase(it);
    symbolTable.insert({{newName, newScope}, temp});
    std::cout << "List renamed" << std::endl;
}

// implement getList using symbolTable
struct listNode *getList(std::string const &name, int const scope) {
    return symbolTable[{name, scope}];
}

int getCurrentScope() {
    return currentScope;
}

struct listNode *getListElement(struct listNode *start, int index) {
    std::cout << "Getting the element at " << index << " of the list" << std::endl;
    for (int i = 0; start != nullptr && i < index; i++) {
        start = start->next;
    }
    if (start == nullptr) {
        std::cerr << "Error: index out of bounds" << std::endl;
        exit(-1);
    }
    return start;
}