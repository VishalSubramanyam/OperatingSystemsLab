#include <bits/stdc++.h>
#include "graph.h"

using namespace std;


int main(int argc, char *argv[]) {
    int pid = atoi(argv[1]);
    bool optimize = false;
    if (argc == 3) optimize = true;
    else optimize = false;
    ofstream out("consumer_" + string(argv[1]), ios::app);
    string outputText;

    Graph g(true);
    int oldSize = g.getNumNodes();
    set<int> sourceNodes;
    for (int i = 0; i < oldSize; i++) {
        if (i % 10 == pid)
            sourceNodes.insert(i);
    }

    using cost = int;
    using node_idx = int;

    priority_queue<pair<cost, node_idx>> pq;

    vector<bool> visited(g.getNumNodes(), false);
    map<int, int> parent;
    //vector<int> costs(g.getNumNodes(), INT_MAX);
    map<int, int> costs;
    for (int i = 0; i < oldSize; i++) {
        parent[i] = -1;
        costs[i] = INT_MAX;
    }
    for (auto src_node: sourceNodes) {
        pq.push({0, src_node});
        costs[src_node] = 0;
    }

    while (!pq.empty()) {
        while (visited[pq.top().second]) {
            pq.pop();
        }
        // the sel_node is part of the boundary
        // if it was already removed from pq in the past
        // the above while loop will eliminate it
        auto temp = pq.top();
        pq.pop();
        auto sel_node = temp.second;
        visited[sel_node] = true;
        auto c = temp.first;

        for (auto nbr: g[sel_node]) {
            if (!visited[nbr] && c + 1 < costs[nbr]) {
                pq.push({c + 1, nbr});
                parent[nbr] = sel_node;
                costs[nbr] = c + 1;
            }
        }
    }

    for (int i = 0; i < g.getNumNodes(); i++) {
        auto cur_node = i;
        outputText += "Shortest path for " + to_string(cur_node) + ": ";
        while (cur_node != -1) {
            outputText += to_string(cur_node) + " <-- ";
            cur_node = parent[cur_node];
        }
        outputText += "\n";
    }
    out << "Iteration 0: \n";
    out << outputText << flush;
    int iterCount = 1;
    while (1) {
        sleep(29);
        int newSize = g.getNumNodes();
        int numNewNodes = newSize - oldSize;
        if (numNewNodes == 0) {
            out << "Iteration " << iterCount << ": \n";
            out << outputText << flush;
        } else {
            outputText.clear();

            if (!optimize) {
                for (int i = oldSize; i < newSize; i++) {
                    if (i % 10 == pid) sourceNodes.insert(i);
                }
                priority_queue<pair<cost, node_idx>> nonOptimizePq;

                vector<bool> nonOptimizedVisited(g.getNumNodes(), false);
                vector<int> nonOptimizedParent(g.getNumNodes(), -1);
                vector<int> nonOptimizedCosts(g.getNumNodes(), INT_MAX);

                for (auto src_node: sourceNodes) {
                    nonOptimizePq.push({0, src_node});
                    nonOptimizedCosts[src_node] = 0;
                }

                while (!nonOptimizePq.empty()) {
                    while (nonOptimizedVisited[nonOptimizePq.top().second]) {
                        nonOptimizePq.pop();
                    }
                    // the sel_node is part of the boundary
                    // if it was already removed from nonOptimizePq in the past
                    // the above while loop will eliminate it
                    auto temp = nonOptimizePq.top();
                    nonOptimizePq.pop();
                    auto sel_node = temp.second;
                    nonOptimizedVisited[sel_node] = true;
                    auto c = temp.first;

                    for (auto nbr: g[sel_node]) {
                        if (!nonOptimizedVisited[nbr] && c + 1 < nonOptimizedCosts[nbr]) {
                            nonOptimizePq.push({c + 1, nbr});
                            nonOptimizedParent[nbr] = sel_node;
                            nonOptimizedCosts[nbr] = c + 1;
                        }
                    }
                }
                outputText.clear();
                outputText = "Iteration " + to_string(iterCount) + ":\n";
                for (int i = 0; i < g.getNumNodes(); i++) {
                    auto cur_node = i;
                    outputText += "Shortest path for " + to_string(cur_node) + ": ";
                    while (cur_node != -1) {
                        outputText += to_string(cur_node) + " <-- ";
                        cur_node = nonOptimizedParent[cur_node];
                    }
                    outputText += "\n";
                }
                out << outputText << flush;
            } else {
                set<int> newSourceNodes;
                set<int> newNonSourceNodes;
                for (int i = oldSize; i < newSize; i++) {
                    if (i % 10 == pid)
                        newSourceNodes.insert(i);
                    else newNonSourceNodes.insert(i);
                }
                vector<int> tempCosts(newSize, INT_MAX);
                vector<int> tempParent(newSize, -1);
                vector<bool> tempVisited(newSize, false);
                priority_queue<pair<int, int>> tempPq;
                for (auto i: newSourceNodes) {
                    tempCosts[i] = 0;
                    tempPq.push({0, i});
                }
                while (!tempPq.empty()) {
                    while (tempVisited[tempPq.top().second]) {
                        tempPq.pop();
                    }
                    // the sel_node is part of the boundary
                    // if it was already removed from pq in the past
                    // the above while loop will eliminate it
                    auto temp = tempPq.top();
                    tempPq.pop();
                    auto sel_node = temp.second;
                    tempVisited[sel_node] = true;
                    auto c = temp.first;

                    for (auto nbr: g[sel_node]) {
                        if (!tempVisited[nbr] && c + 1 < tempCosts[nbr]) {
                            tempPq.push({c + 1, nbr});
                            tempParent[nbr] = sel_node;
                            tempCosts[nbr] = c + 1;
                        }
                    }
                }
                for (int i = 0; i < oldSize; i++) {
                    if (tempCosts[i] < costs[i]) {
                        costs[i] = tempCosts[i];
                        parent[i] = tempParent[i];
                    }
                }
                for (int i = oldSize; i < newSize; i++) {
                    costs[i] = tempCosts[i];
                    parent[i] = tempParent[i];
                }
                for (auto X: newNonSourceNodes) {
                    int minCostSoFar = INT_MAX;
                    for (auto nbr: g[X]) {
                        if (costs[nbr] + 1 < minCostSoFar) {
                            minCostSoFar = costs[nbr] + 1;
                            parent[X] = nbr;
                        }
                    }
                    assert(minCostSoFar != INT_MAX);
                    costs[X] = minCostSoFar;

                    vector<int> simpleParent(newSize, -1);
                    vector<int> simpleCosts(newSize, INT_MAX);
                    vector<bool> simpleVisited(newSize, false);
                    simpleCosts[X] = 0;
                    priority_queue<pair<int, int>> simplePq;
                    simplePq.push({0, X});
                    while (!simplePq.empty()) {
                        while (simpleVisited[simplePq.top().second]) {
                            simplePq.pop();
                        }
                        // the sel_node is part of the boundary
                        // if it was already removed from nonOptimizePq in the past
                        // the above while loop will eliminate it
                        auto temp = simplePq.top();
                        simplePq.pop();
                        auto sel_node = temp.second;
                        simpleVisited[sel_node] = true;
                        auto c = temp.first;

                        for (auto nbr: g[sel_node]) {
                            if (!simpleVisited[nbr] && c + 1 < simpleCosts[nbr]) {
                                simplePq.push({c + 1, nbr});
                                simpleParent[nbr] = sel_node;
                                simpleCosts[nbr] = c + 1;
                            }
                        }
                    }
                    for(int t = 0; t < newSize; t++){
                        assert(costs.find(X) != costs.end());
                        if(simpleCosts[t] + costs[X] < costs[t]){
                            costs[t] = costs[X] + simpleCosts[t];
                            parent[t] = simpleParent[t];
                        }
                    }
                }
                string outputText;
                for (int i = 0; i < newSize; i++) {
                    auto cur_node = i;
                    outputText += "Shortest path for " + to_string(cur_node) + ": ";
                    while (cur_node != -1) {
                        outputText += to_string(cur_node) + " <-- ";
                        cur_node = parent[cur_node];
                    }
                    outputText += "\n";
                }
                out << outputText << flush;
            }
        }
        cout << "Iteration " << iterCount << ": Done\n";
        iterCount++;
        oldSize = newSize;
    }
}