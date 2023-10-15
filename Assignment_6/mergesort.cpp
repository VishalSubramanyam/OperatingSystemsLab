#include <goodmalloc.h>
#include <iostream>

using std::cout, std::endl;

int calls = 0;

// merge sort
void merge_sort(struct listNode *start, int length) {
    std::cout << "Call number " << ++calls << " with length " << length << std::endl;
    startScope();
    if (length == 1) {
        createList("result", 1);
        assignVal("result", 0, start->val);
        endScope();
        return;
    }
    createList("result", length);
    int mid = length / 2;
    merge_sort(start, mid); // 0 to mid - 1
    renameListScope("result", getCurrentScope() + 1, "left", getCurrentScope());
    merge_sort(getListElement(start, mid), length - mid); // mid to length - 1
    renameListScope("result", getCurrentScope() + 1, "right", getCurrentScope());
    // merge using left and right lists created above
    int i = 0, j = 0, k = 0;
    auto left = getList("left", getCurrentScope());
    auto right = getList("right", getCurrentScope());
    // merge the two lists
    while (i < mid && j < length - mid) {
        if (left->val < right->val) {
            assignVal("result", k++, left->val);
            left = left->next;
            i++;
        } else {
            assignVal("result", k++, right->val);
            right = right->next;
            j++;
        }
    }
    // copy the remaining elements
    while (i < mid) {
        assignVal("result", k++, left->val);
        left = left->next;
        i++;
    }
    while (j < length - mid) {
        assignVal("result", k++, right->val);
        right = right->next;
        j++;
    }
    // free children lists
    freeElem("left");
    freeElem("right");
    endScope();
}


int main() {
    int const NUM_ELEMS = 50000;
    startScope();
    createMem(250 * 1024 * 1024);
    createList("list", NUM_ELEMS);
    for (int i = 0; i < NUM_ELEMS; i++) {
        assignVal("list", i, rand() % 100000);
    }
    merge_sort(getList("list", getCurrentScope()), NUM_ELEMS);
    auto result = getList("result", getCurrentScope() + 1);
    for (int i = 0; i < NUM_ELEMS; i++) {
        cout << result->val << " ";
        result = result->next;
    }
    cout << endl;
    freeElem();
    endScope();
    return 0;
}