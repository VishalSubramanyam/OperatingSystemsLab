#ifndef _GRAPH_H
#define _GRAPH_H
struct node {
    int nbr; // neighbour vertex
    node *next;
};

class Graph {
    node **heads = nullptr;
    void *block[2] = {nullptr, nullptr};
    int key[2] = {237180, 936165};
    int shmid[2] = {-1, -1};
    int num_nbrs = 0;

    int num_nodes = 0;
    int max_nodes = 5500;
public:
    [[nodiscard]] const int *getKey();

    [[nodiscard]] int getNumNodes();

    void setKey(int key[2]);

    Graph();

    ~Graph();

    void add_node();

    void add_nbr(int, int);

    [[nodiscard]] node *get_nbrs(int v);

    [[nodiscard]] bool is_nbr(int v, int potential_nbr);

    void printNeighbours(int v);
};


#endif //_GRAPH_H
