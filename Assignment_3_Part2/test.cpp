#include <bits/stdc++.h>
#include "graph.h"

using namespace std;

int main() {
    Graph g(false);
    for (int i = 0; i < 11; i++) {
        g.add_node();
    }
    g.add_edge(0, 1);
    g.add_edge(0,3);
    g.add_edge(1,5);
    g.add_edge(1,6);
    g.add_edge(2,6);
    g.add_edge(2,8);
    g.add_edge(3,4);
    g.add_edge(4,5);
    g.add_edge(5,6);
    g.add_edge(5,7);
    g.add_edge(7,8);
    g.add_edge(9, 6);
    g.add_edge(9,7);
    g.add_edge(10,5);
}