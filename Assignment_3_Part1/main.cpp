#include <bits/stdc++.h>
#include <sys/shm.h>
#include "graph.h"

using std::cout, std::cin, std::endl;


int main() {
    char const *filename = "facebook_combined.txt";
    std::ifstream inp(filename);

    Graph g;
    int x, y;
    while (inp >> x >> y) {
        int cur_nodes = g.getNumNodes();
        while (x >= cur_nodes || y >= cur_nodes) {
            g.add_node();
            cur_nodes = g.getNumNodes();
        }
        g.add_nbr(x, y);
        g.add_nbr(y, x);
    }
}