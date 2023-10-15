#include <sys/shm.h>
#include <iostream>
#include <string.h>
#include "graph.h"

Graph::Graph() {
    // create shared memory for storing pointers to heads of the linked lists
    shmid[0] = shmget(key[0], max_nodes * sizeof(node *), IPC_CREAT | 0666);
    // create shared memory for storing neighbours for every node
    shmid[1] = shmget(key[1], max_nodes * max_nodes * sizeof(node), IPC_CREAT | 0666);
    if (shmid[0] == -1 || shmid[1] == -1) {
        perror("shmget() failed");
    }
    block[0] = shmat(shmid[0], nullptr, 0);
    block[1] = shmat(shmid[1], nullptr, 0);
    if (block[0] == (void *) -1 || block[1] == (void *) -1) {
        perror("shmat() failed");
    }
    heads = static_cast<node **>(block[0]);
    memset(block[0], 0, max_nodes);
}

Graph::~Graph() {
    shmdt(block[0]);
    shmdt(block[1]);
    shmctl(shmid[0], IPC_RMID, nullptr);
    shmctl(shmid[1], IPC_RMID, nullptr);
}

void Graph::add_node() {
    num_nodes++; // creation of linked list will happen inside "add_nbr"
}

void Graph::add_nbr(int v, int nbr) {
    node *new_node = static_cast<node *>(block[1]) + num_nbrs;
    new_node->nbr = nbr;
    new_node->next = heads[v];
    heads[v] = new_node;
    num_nbrs++;
}

node *Graph::get_nbrs(int v) {
    if (v >= num_nodes || v < 0) return nullptr;
    return heads[v];
}

bool Graph::is_nbr(int v, int potential_nbr) {
    node *head = heads[v];
    while (head) {
        if (head->nbr == potential_nbr) return true;
        else head = head->next;
    }
    return false;
}

void Graph::setKey(int key[2]) {
    Graph::key[0] = key[0];
    Graph::key[1] = key[1];
}

int Graph::getNumNodes() {
    return num_nodes;
}

const int *Graph::getKey() {
    return key;
}

void Graph::printNeighbours(int v) {
    auto list = get_nbrs(v);
    std::cout << "Neighbours of " << v << " are:\n";
    while (list) {
        std::cout << list->nbr << "\n";
        list = list->next;
    }
}

