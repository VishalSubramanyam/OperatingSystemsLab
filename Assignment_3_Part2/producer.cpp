#include <bits/stdc++.h>
#include "graph.h"
using namespace std;

#define TIME_PERIOD 5
int main(){
    Graph g(true);
    int itercount = 0;
    while(1){
        cout << "At iteration " << itercount << endl;
        itercount++;
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> mDist(10, 30);  // Define the range of the distribution
        std::uniform_int_distribution<> kDist(1, 20);
        int m = mDist(gen);
        int curSize = g.getNumNodes();
        for(int i = 0; i < m; i++){
            g.add_node();
        }
        vector<int> weights;
        for(int i = 0; i < curSize; i++){
            weights.push_back(g[i].size);
        }
        std::discrete_distribution<> d(weights.begin(), weights.end());
        for(int i = 0; i < m; i++){
            std::set<int> already_used;
            int k = kDist(gen);
            while(already_used.size() < k){
                already_used.insert(d(gen));
            }
            for(auto j : already_used){
                g.add_edge(j, curSize + i);
            }
        }
        sleep(51);
    }
}