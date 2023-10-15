#include <pthread.h>
#include <bits/stdc++.h>
#include <semaphore.h>

using std::vector, std::cout, std::cin, std::endl, std::map;
using std::string;
struct Room {
    int count = 0; // number of times the room has been occupied since last cleaning
    int timeStayedIn = 0; // sum of time stayed in the room by all guests since last cleaning
    // need the above for calculating sleep time for cleanerHandler
    int tid = -1; // thread id of the guest currently occupying the room
    int &priority = tid; // priority of the room is the thread id of the guest currently occupying the room
};

auto roomCmp = [](const Room *a, const Room *b) {
    return a->priority < b->priority;
};

// thread safe queue using semaphores
class ThreadSafeQueue {
    std::queue<Room *> q;
    sem_t sem;
public:
    ThreadSafeQueue() {
        sem_init(&sem, 0, 1);
    }

    void push(Room *x) {
        sem_wait(&sem);
        q.push(x);
        sem_post(&sem);
    }

    Room *pop() {
        sem_wait(&sem);
        Room *x = q.front();
        q.pop();
        sem_post(&sem);
        return x;
    }

    bool empty() {
        sem_wait(&sem);
        bool x = q.empty();
        sem_post(&sem);
        return x;
    }

    // size of the queue
    int size() {
        sem_wait(&sem);
        int x = q.size();
        sem_post(&sem);
        return x;
    }
};


// Thread safe set using semaphores
class ThreadSafeSet {
    std::set<Room *, decltype(roomCmp)> s;
    sem_t sem;
public:
    ThreadSafeSet() : s(roomCmp) {
        sem_init(&sem, 0, 1);
    }

    void insert(Room *x) {
        sem_wait(&sem);
        s.insert(x);
        sem_post(&sem);
    }

    void erase(Room *x) {
        sem_wait(&sem);
        s.erase(x);
        sem_post(&sem);
    }

    bool empty() {
        sem_wait(&sem);
        bool x = s.empty();
        sem_post(&sem);
        return x;
    }

    Room *getMin() {
        sem_wait(&sem);
        Room *x = *s.begin();
        sem_post(&sem);
        return x;
    }
};

// X -> num of cleaners
// Y -> num of guests
// N -> num of rooms
int X, Y, N; // Y > N > X > 1
ThreadSafeQueue availableQueue;
ThreadSafeSet occupiedSet;
ThreadSafeQueue dirtyQueue;
pthread_cond_t cleanerCond = PTHREAD_COND_INITIALIZER;
pthread_cond_t guestCond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t guestMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t queueTransferMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t cleanerMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t guestSleepMutex = PTHREAD_MUTEX_INITIALIZER;
map<pthread_t, pthread_cond_t> awakenGuestCond;

pthread_mutex_t printMutex = PTHREAD_MUTEX_INITIALIZER;

void printLog(string s){
    pthread_mutex_lock(&printMutex);
    cout << s << endl;
    pthread_mutex_unlock(&printMutex);
}

void *guestHandler(void *args) {
    // randomly generate a number between 10 and 20 (inclusive)
    // using C++11 random number generator
    printLog("Started guest handler " + std::to_string(pthread_self()));
    std::uniform_int_distribution<int> dist(10, 20); // for generating random sleep time
    std::uniform_int_distribution<int> dist2(10, 30); // for generating random time to stay
    std::random_device rd;
    std::mt19937 gen(rd());
    while (1) {
        pthread_mutex_lock(&guestMutex);
        while (!dirtyQueue.empty()) {
            // wait for the dirtyQueue to be empty
            pthread_cond_wait(&guestCond, &guestMutex);
        }
        pthread_mutex_unlock(&guestMutex);
        while (1) {
            int time = dist(gen);
            sleep(time); // guest sleeps initially

            // generate a "time to stay" between 10 and 30 (inclusive)
            int timeToStay = dist2(gen);
            // if room not available, see if the room with the lowest priority has priority lower
            // than current thread id, if so, change the tid to the current thread id (since this
            // thread has higher priority).
            // if room available, occupy it by moving room from availableQueue to occupiedSet
            // if room not available and priority is not sufficient, continue to next iteration
            pthread_mutex_lock(&queueTransferMutex);
            if (availableQueue.empty() && !occupiedSet.empty() && occupiedSet.getMin()->priority < pthread_self()) {
                // Awaken the thread that is occupying the room with the lowest priority
                auto temp = occupiedSet.getMin();
                printLog("Evicted guest with thread ID: " + std::to_string(temp->tid) + " from room with priority: " + std::to_string(temp->priority) + " to make room for guest with thread ID: " + std::to_string(pthread_self()) + " with priority: " + std::to_string(pthread_self()) + " for " + std::to_string(timeToStay) + " seconds");
                pthread_mutex_lock(&guestSleepMutex);
                pthread_cond_signal(&awakenGuestCond[temp->tid]);
                pthread_mutex_unlock(&guestSleepMutex);

                occupiedSet.erase(temp);
                pthread_mutex_unlock(&queueTransferMutex);

                // change the tid of the room with the lowest priority
                temp->tid = pthread_self();
                // change the priority of the room with the lowest priority
                temp->priority = temp->tid;
                // Add the time to stay to the time stayed in the room
                temp->timeStayedIn += timeToStay;
                // increment the count of the room
                int count = temp->count++; // use the room now
                // If the count is 2, add the room to dirtyQueue
                if (count == 2) {
                    sleep(timeToStay); // simple sleep since no one can access this room
                    // pointer to this room not available in occupiedSet or availableQueue
                    // add the room to dirtyQueue
                    printLog("Room occupied by tid: " + std::to_string(temp->tid) + " is dirty");
                    dirtyQueue.push(temp); // no race condition since only cleanerHandler can access dirtyQueue
                    // and no one has access to this room pointed by temp, so no duplication
                    if (dirtyQueue.size() == N) {
                        printLog("Guest threads are done, waking up cleaner threads");
                        // broadcast
                        pthread_cond_broadcast(&cleanerCond);
                        break;
                    }
                } else {
                    // perform a conditional wait for timeToStay seconds
                    struct timespec ts;
                    // fill ts with timeToStay
                    clock_gettime(CLOCK_REALTIME, &ts);
                    ts.tv_sec += timeToStay;
                    pthread_mutex_lock(&guestSleepMutex);
                    occupiedSet.insert(temp); // reinsert at right location in the BST
                    auto x = pthread_cond_timedwait(&awakenGuestCond[temp->tid], &guestSleepMutex, &ts);
                    pthread_mutex_unlock(&guestSleepMutex);
                    if(x == ETIMEDOUT){
                        // if timed out, add the room to availableQueue
                        pthread_mutex_lock(&queueTransferMutex);
                        availableQueue.push(temp);
                        printLog("Guest with thread ID: " + std::to_string(temp->tid) + " with priority: " + std::to_string(temp->priority) + " has left the room after staying for " + std::to_string(temp->timeStayedIn) + " seconds");
                        pthread_mutex_unlock(&queueTransferMutex);
                    }

                }
            } else if (!availableQueue.empty()) {
                // occupy the room by moving room from availableQueue to occupiedSet
                Room *room = availableQueue.pop();
                pthread_mutex_unlock(&queueTransferMutex);

                room->tid = pthread_self();
                room->priority = room->tid;
                // perform a conditional wait for timeToStay seconds
                struct timespec ts;
                // fill ts with timeToStay
                clock_gettime(CLOCK_REALTIME, &ts);
                ts.tv_sec += timeToStay;
                pthread_mutex_lock(&guestSleepMutex);
                occupiedSet.insert(room); // reinsert at right location in the BST
                auto x = pthread_cond_timedwait(&awakenGuestCond[room->tid], &guestSleepMutex, &ts);
                pthread_mutex_unlock(&guestSleepMutex);
                if(x == ETIMEDOUT){
                    // if timed out, add the room to availableQueue
                    pthread_mutex_lock(&queueTransferMutex);
                    availableQueue.push(room);
                    printLog("Guest with thread ID: " + std::to_string(room->tid) + " with priority: " + std::to_string(room->priority) + " has left the room after staying for " + std::to_string(room->timeStayedIn) + " seconds");
                    pthread_mutex_unlock(&queueTransferMutex);
                }

            } else {
                pthread_mutex_unlock(&queueTransferMutex);
                if(dirtyQueue.size() == N){
                    break;
                }
            }
        }
    }

}

void *cleanerHandler(void *args) {
    while (1) {
        pthread_mutex_lock(&cleanerMutex);
        while (dirtyQueue.size() < N) {
            // wait for the dirtyQueue to be full
            pthread_cond_wait(&cleanerCond, &cleanerMutex);
        }
        pthread_mutex_unlock(&cleanerMutex);
        // clean the rooms
        while (!dirtyQueue.empty()) {
            Room *room = dirtyQueue.pop();
            sleep(room->timeStayedIn);
            room->timeStayedIn = 0;
            room->count = 0;
            room->tid = -1;
            room->priority = -1;
            // move the room from dirtyQueue to availableQueue
            availableQueue.push(room);
        }
        printLog("Cleaner threads are done, waking up guest threads");
        // broadcast
        pthread_mutex_lock(&guestMutex);
        pthread_cond_broadcast(&guestCond);
        pthread_mutex_unlock(&guestMutex);
    }
}

int main() {
    std::cin >> X >> Y >> N;
    // initialize the availableQueue
    for (int i = 0; i < N; i++) {
        availableQueue.push(new Room());
    }

    vector<pthread_t> guestThreads(Y), cleanerThreads(X);
    for (int i = 0; i < X; i++) {
        pthread_create(&cleanerThreads[i], nullptr, &cleanerHandler, nullptr);

    }
    for (int i = 0; i < Y; i++) {
        pthread_create(&guestThreads[i], nullptr, &guestHandler, nullptr);
        pthread_cond_init(&awakenGuestCond[guestThreads[i]], nullptr);
    }

    // join everything
    for (int i = 0; i < X; i++) {
        pthread_join(cleanerThreads[i], nullptr);
    }
    for (int i = 0; i < Y; i++) {
        pthread_join(guestThreads[i], nullptr);
    }


    return 0;
}
