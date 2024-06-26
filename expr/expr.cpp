#include <pthread.h>
#include <iostream>
#include <fstream>
#include <ctime>
#include <vector>

using namespace std;

vector<int> coefficients;
int target = 0;
int num_threads = 0;
int solution_count = 0;
pthread_mutex_t mtx;
int coef_size = 0;

clock_t start_time = 0;
clock_t end_time = 0;

void generate_and_check(int idx, int current_sum, int used_count) {
    if (idx == coef_size) {
        if (current_sum == target && used_count == coef_size) {
            pthread_mutex_lock(&mtx);
            solution_count++;
            pthread_mutex_unlock(&mtx);
        }
        return;
    }
    generate_and_check(idx + 1, current_sum + coefficients[idx], used_count + 1);
    generate_and_check(idx + 1, current_sum - coefficients[idx], used_count + 1);
}

void* solve_task(void* param) {
    int thread_id = *((int*)param);

    int chunk_size = coef_size / num_threads;
    int start_index = thread_id * chunk_size;
    int end_index = (thread_id == num_threads - 1) ? coef_size : (thread_id + 1) * chunk_size;
    
    for (int i = start_index; i < end_index; ++i) {
        generate_and_check(i + 1, coefficients[i], 1);
    }
    return nullptr;
}

void read_input_file() {
    ifstream input_file("input.txt");

    input_file >> num_threads;
    input_file >> coef_size;
    coefficients.resize(coef_size);

    for (int i = 0; i < coef_size; ++i) {
        input_file >> coefficients[i];
    }

    input_file >> target;

    input_file.close();
}

void write_output_file(double duration) {
    ofstream output_file("output.txt");
    ofstream time_file("time.txt");

    output_file << num_threads << endl;
    output_file << coef_size << endl;
    output_file << solution_count << endl;

    time_file << duration << endl;

    output_file.close();
    time_file.close();
}

int main() {
    read_input_file();

    pthread_mutex_init(&mtx, nullptr);

    pthread_t* threads = new pthread_t[num_threads];
    int* thread_args = new int[num_threads];

    start_time = clock();
    for (int i = 0; i < num_threads; ++i) {
        thread_args[i] = i;

        pthread_create(&threads[i], nullptr, solve_task, &thread_args[i]);
    }

    
    for (int i = 0; i < num_threads; ++i) {
        pthread_join(threads[i], nullptr);
    }

    end_time = clock();
    pthread_mutex_destroy(&mtx);

    double duration = (double)(end_time - start_time) / CLOCKS_PER_SEC * 1000;

    write_output_file(duration);

    delete[] threads;
    delete[] thread_args;

    return 0;
}