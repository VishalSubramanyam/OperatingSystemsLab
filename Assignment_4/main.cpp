#include <pthread.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <set>
#include <random>
#include <queue>
#include <unistd.h>
#include <algorithm>

using namespace std;


// Thread-safe queue that uses mutexes
// and conditional var for safe access
template<typename T>
class ThreadSafeQueue {

private:
    std::queue<T> _queue;
    pthread_mutex_t queueMutex;
    pthread_cond_t emptyCondVar;

public:
    ThreadSafeQueue();

    void Enqueue(T data);

    T Dequeue();

    bool IsEmpty() {
        return _queue.empty();
    }

    void DestroyQueue();
};

template<typename T>
ThreadSafeQueue<T>::ThreadSafeQueue() {
    pthread_mutex_init(&queueMutex, nullptr);
    pthread_cond_init(&emptyCondVar, nullptr);
}

template<typename T>
void ThreadSafeQueue<T>::Enqueue(T data) {
    pthread_mutex_lock(&queueMutex);
    _queue.push(data);
    pthread_cond_signal(&emptyCondVar);
    pthread_mutex_unlock(&queueMutex);
}

template<typename T>
T ThreadSafeQueue<T>::Dequeue() {

    pthread_mutex_lock(&queueMutex);
    while (_queue.empty()) {
        pthread_cond_wait(&emptyCondVar, &queueMutex);
    }
    T elem = _queue.front();
    _queue.pop();
    pthread_mutex_unlock(&queueMutex);
    return elem;
}

template<typename T>
void ThreadSafeQueue<T>::DestroyQueue() {
    pthread_mutex_lock(&queueMutex);
    _queue = std::queue<T>();
    pthread_mutex_unlock(&queueMutex);
}

// Thread safe set that uses mutexes
// and conditional var for safe access
template<typename T>
class ThreadSafeSet {
private:
    std::set<T> _set;
    pthread_mutex_t setMutex;
    pthread_cond_t emptySetCondVar;

public:
    ThreadSafeSet();

    void Insert(T data);

    int Erase();

    bool Contains(T data);

    bool IsEmpty() {
        return _set.empty();
    }

    void DestroySet();
};

template<typename T>
ThreadSafeSet<T>::ThreadSafeSet() {
    pthread_mutex_init(&setMutex, nullptr);
    pthread_cond_init(&emptySetCondVar, nullptr);
}

template<typename T>
void ThreadSafeSet<T>::Insert(T data) {
    pthread_mutex_lock(&setMutex);
    _set.insert(data);
    pthread_cond_signal(&emptySetCondVar);
    pthread_mutex_unlock(&setMutex);
}

template<typename T>
int ThreadSafeSet<T>::Erase() {
    pthread_mutex_lock(&setMutex);
    while (_set.empty()) {
        pthread_cond_wait(&emptySetCondVar, &setMutex);
    }
    T data = *_set.begin();
    _set.erase(_set.begin());
    pthread_mutex_unlock(&setMutex);

    return data;
}

template<typename T>
bool ThreadSafeSet<T>::Contains(T data) {
    pthread_mutex_lock(&setMutex);
    bool contains = _set.find(data) != _set.end();
    pthread_mutex_unlock(&setMutex);
    return contains;
}

template<typename T>
void ThreadSafeSet<T>::DestroySet() {
    pthread_mutex_lock(&setMutex);
    _set = std::set<T>();
    pthread_mutex_unlock(&setMutex);
}

struct Action {
    int user_id;
    int action_id;
    enum ACTION_TYPE {
        POST, COMMENT, LIKE
    } action_type;
    time_t timestamp;

    friend ostream &operator<<(ostream &os, Action const &inp) {
        string type;
        if (inp.action_type == ACTION_TYPE::POST) type = "POST";
        else if (inp.action_type == ACTION_TYPE::COMMENT) type = "COMMENT";
        else type = "LIKE";
        os << "{ " << inp.user_id << ", " << inp.action_id << ", " << type << ", " << inp.timestamp << " }";

        return os;
    }
};

class Node {
    vector<int> adjList;
    int priority;
    vector<int> actionCounter = vector(3, 0);
public:
    ThreadSafeQueue<Action> wall_queue, feed_queue;

    void add_nbr(int u) {
        this->adjList.push_back(u);
    }

    int get_act_counter(int type) {
        return actionCounter[type];
    }

    void inc_act_counter(int type) {
        actionCounter[type] = actionCounter[type] + 1;
    }

    int get_deg() const {
        return this->adjList.size();
    }

    int operator[](int x) {
        return adjList[x];
    }

    decltype(adjList.begin()) begin() {
        return adjList.begin();
    }

    decltype(adjList.end()) end() {
        return adjList.end();
    }

    void setPriority(int p) {
        priority = p;
    }

    int getPriority() {
        return priority;
    }
};

class Graph {
    vector<Node> nodes;

    void add_node(int u) {
        if (u >= nodes.size()) {
            nodes.resize(u + 1);
        }
    }

public:
    vector<vector<int>> commonNeighbors;

    void add_edge(int u, int v) {
        add_node(u);
        add_node(v);
        nodes[u].add_nbr(v);
        nodes[v].add_nbr(u);
    }

    Node &operator[](int x) {
        return nodes[x];
    }

    decltype(nodes.begin()) begin() {
        return nodes.begin();
    }

    decltype(nodes.end()) end() {
        return nodes.end();
    }

    decltype(nodes.size()) size() const {
        return nodes.size();
    }

    void givePriorities() {
        random_device rd;
        mt19937 gen(rd());
        uniform_int_distribution<> dist(0, 1);
        int i = 0;
        for (; i < nodes.size(); i++) {
            int priority = dist(gen);
            nodes[i].setPriority(priority);
        }
    };

    void computeCommonNeighbors() {
        cout << "Computing common neighbors...\n";
        commonNeighbors = vector<vector<int>>(nodes.size(), vector<int>(nodes.size(), 0));

        for (auto node: nodes) {
            for (auto nbr: node) {
                for (auto nbr2: node) {
                    if (nbr != nbr2) {
                        commonNeighbors[nbr][nbr2] += 1;
                        commonNeighbors[nbr2][nbr] += 1;
                    }
                }
            }
        }
        cout << "Done computing common neighbors.\n";
    }

    int computeQueueSize(){
        // find maximum degree of nodes
        int maxDeg = 0;
        for (auto node: nodes) {
            if (node.get_deg() > maxDeg) {
                maxDeg = node.get_deg();
            }
        }

        int maxSize = 100 * ((int)log2(maxDeg)+1);

        return maxSize;
    }
};


Graph g;
ThreadSafeQueue<Action> pushUpdateQueue;
ThreadSafeSet<int> feedUpdateSet;

pthread_mutex_t stdout_mutex;
pthread_mutex_t logfile_mutex;

// a single thread that simulates 100 different users
void *userSimulator(void *arg) {
    ostringstream oss;
    ofstream logFile("sns.log", ios::app);
    auto const NUM_NODES = g.size();
    random_device rd; // Create a random device to seed the random number generator
    mt19937 gen(rd()); // Create a Mersenne Twister PRNG engine and seed it with the random device
    uniform_int_distribution<> dist(0, NUM_NODES - 1);
    uniform_int_distribution<> actionDist(0, 2);
    while (true) {
        set<int> v;
        while (v.size() < 100) {
            auto k = dist(gen);
            v.insert(k);
        }

        for (auto node: v) {
            auto deg = g[node].get_deg();
            // assume proportionality constant is 1
            int numActions = std::log2(deg);

            // Logging code
            oss << "Node: " << node << "\n"
                << "Number of actions: " << numActions << "\n"
                << "Degree of node: " << deg << "\n";
            // End of logging code
            for (int i = 0; i < numActions; i++) {
                Action newAct;
                newAct.user_id = node;
                newAct.action_type = static_cast<Action::ACTION_TYPE> (actionDist(gen));
                newAct.action_id = g[node].get_act_counter(newAct.action_type);
                g[node].inc_act_counter(newAct.action_type);
                newAct.timestamp = time(nullptr);
                g[node].wall_queue.Enqueue(newAct);
                pushUpdateQueue.Enqueue(newAct);
                // Logging code
                oss << newAct << "\n";
                // End of logging code
            }
        }

        string output_string = oss.str();

        // lock stdout and logFile
        pthread_mutex_lock(&stdout_mutex);
        pthread_mutex_lock(&logfile_mutex);
        cout << output_string;
        logFile << output_string;
        // unlock stdout and logFile
        pthread_mutex_unlock(&stdout_mutex);
        pthread_mutex_unlock(&logfile_mutex);
        oss.str("");

        sleep(120);
    }
}

void *pushUpdate(void *arg) {
    ostringstream oss;

    ofstream logFile("sns.log", ios::app);
    while (true) {
        auto act = pushUpdateQueue.Dequeue(); // will block if empty
        auto associatedNode = act.user_id;
        // Logging code
        oss << "Action dequeued: " << act << "\n"
            << "Taken from user: " << act.user_id << "\n";
        // End of logging code
        for (auto neighbor: g[associatedNode]) {
            g[neighbor].feed_queue.Enqueue(act);
            feedUpdateSet.Insert(neighbor);
            // Logging code
            oss << "Pushed into feed queue of " << neighbor << "\n";
            // End of logging code
        }

        string output_string = oss.str();

        // lock stdout and logFile
        pthread_mutex_lock(&stdout_mutex);
        pthread_mutex_lock(&logfile_mutex);
        cout << output_string;
        logFile << output_string;
        // unlock stdout and logFile
        pthread_mutex_unlock(&stdout_mutex);
        pthread_mutex_unlock(&logfile_mutex);

        oss.str("");
    }
}

bool cmp_timestamp(const Action &a, const Action &b) {
    return a.timestamp < b.timestamp;
}

bool cmp_priority(const Action &a, const Action &b, const vector<int> &commonNeighbors) {
    return commonNeighbors[a.user_id] > commonNeighbors[b.user_id];
}

void *readPost(void *arg) {
    ostringstream oss;

    ofstream logFile("sns.log", ios::app);
    while (true) {
        auto user_id = feedUpdateSet.Erase();
        vector<Action> localFeed;
        int user_mode = g[user_id].getPriority();
        while (true) {
            if (g[user_id].feed_queue.IsEmpty()) break;
            auto act = g[user_id].feed_queue.Dequeue();
            localFeed.push_back(act);
        }

        if (!localFeed.empty()) {
            vector<int> commonNeighbors = g.commonNeighbors[user_id];

            oss << "\nFor user: " << user_id << "\n";

            if (user_mode == 0) {
                oss << "Priority mode\n";
                sort(localFeed.begin(), localFeed.end(), [&](const Action &a, const Action &b) {
                    return cmp_priority(a, b, commonNeighbors);
                });
            } else {
                oss << "Chronological mode\n";
                sort(localFeed.begin(), localFeed.end(), cmp_timestamp);
            }

            for (auto act: localFeed) {
                // Logging code
                oss << "Read action " << act << " from user " << act.user_id << "\n";
                // End of logging code
            }

            string output_string = oss.str();

            // lock stdout and logFile
            pthread_mutex_lock(&stdout_mutex);
            pthread_mutex_lock(&logfile_mutex);
            cout << output_string;
            logFile << output_string;
            // unlock stdout and logFile
            pthread_mutex_unlock(&stdout_mutex);
            pthread_mutex_unlock(&logfile_mutex);
        }

        oss.str("");
    }
}

int main() {

    pthread_mutex_init(&logfile_mutex, nullptr);
    pthread_mutex_init(&stdout_mutex, nullptr);

    std::ifstream inpFile("musae_git_edges.csv");
    std::string line;
    getline(inpFile, line);
    cout << "Populating graph...\n";
    while (inpFile >> line) {
        int u, v;
        sscanf(line.c_str(), "%d,%d", &u, &v);
        g.add_edge(u, v);
    }
    cout << "Graph populated\n";
    g.givePriorities();
    g.computeCommonNeighbors();

    // Create a userSimulator thread
    pthread_t userSimulatorThread;
    pthread_create(&userSimulatorThread, nullptr, &userSimulator, nullptr);

    // Create a pool of 25 pushUpdate threads
    pthread_t pushUpdateThreads[25];
    for (int i = 0; i < 25; i++) {
        pthread_create(&pushUpdateThreads[i], nullptr, &pushUpdate, nullptr);
    }

    // Create a pool of 10 readPost threads
    pthread_t readPostThreads[10];
    for (int i = 0; i < 10; i++) {
        pthread_create(&readPostThreads[i], nullptr, &readPost, nullptr);
    }

    pthread_join(userSimulatorThread, nullptr);
    for (int i = 0; i < 25; i++) {
        pthread_join(pushUpdateThreads[i], nullptr);
    }
    for (int i = 0; i < 10; i++) {
        pthread_join(readPostThreads[i], nullptr);
    }
}