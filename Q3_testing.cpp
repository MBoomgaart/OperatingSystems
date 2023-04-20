#include <iostream>
#include <thread>
#include <semaphore.h>
#include <queue>
#include <string>
#include <chrono>
#include <ctime>

using namespace std;

struct requestStructure {
    int request_id;
    string ip_address;
    string page_requested;
};

const int BUFFER_SIZE = 5;
queue<requestStructure> msg_queue;
string webPages[10] = {"google.com", "yahoo.com", "bing.com", "facebook.com", "twitter.com", "linkedin.com", "amazon.com", "instagram.com", "reddit.com", "netflix.com"};

sem_t empty_slots;
sem_t full_slots;
sem_t mutex_lock;


int request_id = 0;

void listen() {
    while (true) {

        this_thread::sleep_for(chrono::seconds(1));

        requestStructure new_request;
        new_request.request_id = request_id;
        new_request.ip_address = "";
        new_request.page_requested = webPages[rand() % 10];

        sem_wait(&empty_slots);
        sem_wait(&mutex_lock);

        msg_queue.push(new_request);
        cout << "Request " << request_id << " added to queue." << endl;
        request_id++;

        sem_post(&mutex_lock);
        sem_post(&full_slots);
    }
}

void do_request(int thread_id) {
    while (true) {
        sem_wait(&full_slots);
        sem_wait(&mutex_lock);
        if (msg_queue.empty()) {
            sem_post(&mutex_lock);
            continue;
        }
        requestStructure req = msg_queue.front();
        msg_queue.pop();
        sem_post(&mutex_lock);
        cout << "thread " << thread_id << " completed request " << req.request_id << " requesting webpage " << req.page_requested << endl;
    }
}

int main() {
    // initialize semaphores
    sem_init(&empty_slots, 0, BUFFER_SIZE);
    sem_init(&full_slots, 0, 0);
    sem_init(&mutex_lock, 0, 1);

    // create listen thread
    thread listen_thread(listen);

    // create worker threads
    int num_threads = 4;
    thread worker_threads[num_threads];
    for (int i = 0; i < num_threads; i++) {
        worker_threads[i] = thread(do_request, i);
    }

    // wait for threads to finish (should never happen)
    listen_thread.join();
    for (int i = 0; i < num_threads; i++) {
        worker_threads[i].join();
    }

    // destroy semaphores
    sem_destroy(&empty_slots);
    sem_destroy(&full_slots);
    sem_destroy(&mutex_lock);

    return 0;
}
