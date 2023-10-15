#ifndef _GRAPH_H
#define _GRAPH_H

#include <string>

using nbr_ptr = int;

class Graph {
    static Graph *graph;
    struct linked_list;
public:
    class iterator {
        nbr_ptr ptr;
    public:
        iterator(nbr_ptr const &ptr) : ptr(ptr) {}

        int operator*() const {
            return graph->nbrs[ptr].nbr;
        }

        iterator &operator++() {
            if (ptr != -1)
                ptr = graph->nbrs[ptr].next;
            return (*this);
        }

        iterator operator++(int) {
            iterator retval = *this;
            ++(*this);
            return retval;
        }

        bool operator!=(iterator const &other) const { return ptr != other.ptr; }

        bool operator==(iterator const &other) const { return ptr == other.ptr; }
    };

    explicit Graph(bool connect = false);

    void add_node() {
        new(&heads[*num_nodes_remote]) linked_list();
        *num_nodes_remote += 1;
    }

    void add_edge(int u, int v) {
        heads[u].insert(v);
        heads[v].insert(u);
    }

    linked_list const &operator[](int u) const {
        return heads[u];
    }

    int getNumNodes() {
        return *num_nodes_remote;
    }

private:
    struct neighbor {
        int nbr; // neighbour vertex
        nbr_ptr next;

        neighbor(int nbr, nbr_ptr next) : nbr(nbr), next(next) {}
    };

    struct linked_list {
        nbr_ptr head;
        int size;

        linked_list() : head(-1), size(0) {
        }

        iterator begin() const {
            return {head};
        }

        iterator end() const {
            return {-1};
        }

        void insert(int v) {
            new(&graph->nbrs[*graph->num_nbrs_remote]) neighbor(v, head);
            head = *graph->num_nbrs_remote;
            *graph->num_nbrs_remote += 1;
            size += 1;
        }
    };

    //void *block[2] = {nullptr, nullptr}; // process-local pointers into graph
    neighbor *nbrs = nullptr;
    linked_list *heads = nullptr;


    char const *const HEADS_KEY = "heads";
    char const *const NODES_KEY = "nbrs";
    char const *const METADATA_KEY = "metadata";

    // Process-local stack variable
    int max_nodes;
    int max_nbrs;

    // Process-local pointers into shared memory
    // revealing state of graph
    int *num_nodes_remote; // seg size = 4 bytes
    int *max_nodes_remote; // seg size = 4 bytes
    int *num_nbrs_remote; // seg size = 4 byte
    int *max_nbrs_remote; // seg size = 4 bytes
    // Above three variables stored consecutively in a single shared memory
    // segment
};
#endif //_GRAPH_H
