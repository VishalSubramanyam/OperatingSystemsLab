#include "graph.h"
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
Graph *Graph::graph = nullptr;
Graph::Graph(bool connect){
    graph = this;
    // resizable
    int headFD = shm_open(HEADS_KEY, O_CREAT | O_RDWR, 0666);
    if (headFD < 0) perror("Couldn't access shmem segment for linked list heads");

    // resizable
    int nodesFD = shm_open(NODES_KEY, O_CREAT | O_RDWR, 0666);
    if (nodesFD < 0) perror("Couldn't access shmem segment for nbrs");

    // non-resizable
    int metaDataFD = shm_open(METADATA_KEY, O_CREAT | O_RDWR, 0666);
    if (metaDataFD < 0) perror("Couldn't access shmem segment for metadata");

    // resize the shared mem segment for metadata to 4 ints
    // only when the graph is created for the first time
    if (!connect)
        ftruncate(metaDataFD, 4 * sizeof(int));

    num_nodes_remote = static_cast<int *> (mmap(nullptr, 4 * sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED,
                                                metaDataFD, 0));
    max_nodes_remote = num_nodes_remote + 1;
    num_nbrs_remote = max_nodes_remote + 1;
    max_nbrs_remote = num_nbrs_remote + 1;

    if (!connect) {
        max_nodes = 5500;
        *max_nodes_remote = max_nodes;
        max_nbrs = 95000 * 2; // initially, we have space for
        *max_nbrs_remote = max_nbrs;
        *num_nodes_remote = 0;
        *num_nbrs_remote = 0;
        // resize shared memory segments for the first time
        // when graph is created
        ftruncate(headFD, (*max_nodes_remote) * sizeof(linked_list));
        ftruncate(nodesFD, (*max_nbrs_remote) * sizeof(neighbor));
    } else{
        max_nodes = *max_nodes_remote;
        max_nbrs = *max_nbrs_remote;
    }

    // map all these segments into the address space
    nbrs = static_cast<neighbor *> (mmap(nullptr, (*max_nbrs_remote) * sizeof(neighbor), PROT_READ | PROT_WRITE, MAP_SHARED, nodesFD, 0));
    heads = static_cast<linked_list *> (mmap(nullptr, (*max_nodes_remote) * sizeof(linked_list), PROT_READ | PROT_WRITE, MAP_SHARED, headFD, 0));
}
