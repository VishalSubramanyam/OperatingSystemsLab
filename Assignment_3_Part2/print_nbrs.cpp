#include <bits/stdc++.h>
#include "graph.h"
using namespace std;
int main(){
    Graph g(true);
    while(true){
        cout << "Enter neighbor: ";
        int x; cin >> x;
        if(x == -1) break;
        for(auto nbr : g[x]){
            cout << nbr << " ";
        }
        cout << endl;
    }
}