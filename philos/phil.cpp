#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <thread>
#include <vector>
#include <iostream>
#include <fstream>
#include <chrono>


using namespace std;

const int NUM_PHILOSOPHERS = 5;
sem_t forks[NUM_PHILOSOPHERS];
sem_t philosopher_semaphores[NUM_PHILOSOPHERS];
pthread_mutex_t output_mutex;

chrono::steady_clock::time_point start_time;
chrono::milliseconds total_time;
chrono::milliseconds phil_time;

void eat(int id) {
    sem_wait(&forks[id]);
    sem_wait(&forks[(id + 1) % NUM_PHILOSOPHERS]);

    auto take_time = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - start_time);

    if (take_time > total_time + phil_time) {
        exit(0); // Exit the function
    }

    pthread_mutex_lock(&output_mutex);
    std::cout << take_time.count() << ":" << id + 1 << ":T->E\n";
    pthread_mutex_unlock(&output_mutex);

    std::this_thread::sleep_for(phil_time);

    sem_post(&forks[id]);
    sem_post(&forks[(id + 1) % NUM_PHILOSOPHERS]);
    sem_post(&philosopher_semaphores[id]);
}

void think(int id) {
    auto take_time = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - start_time);
    pthread_mutex_lock(&output_mutex);
    std::cout << take_time.count() << ":" << id + 1 << ":E->T\n";
    pthread_mutex_unlock(&output_mutex);
    std::this_thread::sleep_for(phil_time);
}

void* philosopher(void* param) {
    int id = *((int*) param);

    auto start_time = std::chrono::steady_clock::now();
    int elapsed_time = 0;

    while (elapsed_time < total_time.count() - phil_time.count()) {
        sem_wait(&philosopher_semaphores[id]);

        eat(id);
        think(id);

        elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - start_time).count();
    }

    return nullptr;
}//lflfdadps ya sdelala eto

int main(int argc, char *argv[]) {

    start_time = chrono::steady_clock::now();
    total_time = chrono::milliseconds(stoi(argv[1]));
    phil_time = chrono::milliseconds(stoi(argv[2]));

    pthread_mutex_init(&output_mutex, nullptr);

    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        sem_init(&forks[i], 0, 1);
        sem_init(&philosopher_semaphores[i], 0, 1);
    }

    pthread_t philosopher_threads[NUM_PHILOSOPHERS];
    int philosopher_ids[NUM_PHILOSOPHERS];

    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        philosopher_ids[i] = i;
        pthread_create(&philosopher_threads[i], nullptr, philosopher, &philosopher_ids[i]);
    }

    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        pthread_join(philosopher_threads[i], nullptr);
    }

    pthread_mutex_destroy(&output_mutex);

    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        sem_destroy(&forks[i]);
        sem_destroy(&philosopher_semaphores[i]);
    }

    return 0;
}