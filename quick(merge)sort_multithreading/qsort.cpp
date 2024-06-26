#define _CRT_SECURE_NO_WARNINGS
#include <fstream>
#include <iostream>
#include <vector>
#include <windows.h>
#include <time.h>

using namespace std;

vector<int> arr;
int num_threads = 0;
int arr_size = 0; 

void measure_time(clock_t start_time, const char* filename) {
    clock_t end_time = clock();
    double duration = (double)(end_time - start_time) / CLOCKS_PER_SEC * 1000;
    ofstream time(filename);
    time << duration << endl;
}

int read_input() {
    FILE* input = fopen("input.txt", "r");
    if (!input) {
        cerr << "Error opening input file." << endl;
        return -1;
    }

    fscanf(input, "%d %d", &num_threads, &arr_size);

    arr.resize(arr_size);

    for (int i = 0; i < arr_size; i++) {
        fscanf(input, "%d", &arr[i]);
    }

    fclose(input);

    return 0;
}

void write_output() {
    FILE* output = fopen("output.txt", "w");
    if (!output) {
        cerr << "Error opening output file." << endl;
        exit(EXIT_FAILURE);
    }

    fprintf(output, "%d\n", num_threads);
    fprintf(output, "%d\n", arr_size);
    for (int i = 0; i < arr_size; i++) {
        fprintf(output, "%d ", arr[i]);
    }
    fprintf(output, "\n");

    fclose(output);
}

void swap(int& a, int& b) {
    int temp = a;
    a = b;
    b = temp;
}

int partition(int left, int right) {
    int pivot = arr[(left + right) / 2];
    int i = left, j = right;
    while (i <= j) {
        while (arr[i] < pivot) i++;
        while (arr[j] > pivot) j--;
        if (i <= j) {
            swap(arr[i], arr[j]);
            i++;
            j--;
        }
    }
    return i;
}

void make_qsort(int left, int right) {
    if (left < right) {
        int pivotIndex = partition(left, right);
        make_qsort(left, pivotIndex - 1);
        make_qsort(pivotIndex, right);
    }
}

DWORD WINAPI thread_entry(void* param) {
    int idx = (int)param;
    int left = idx * (arr.size() / num_threads);
    int right = (idx == num_threads - 1) ? arr.size() - 1 : (idx + 1) * (arr.size() / num_threads) - 1;
    make_qsort(left, right);
    return 0;
}

void multithread_qsort() {
    if (arr.size() < 1000) {
        make_qsort(0, arr.size() - 1);
    }
    else {
        HANDLE* thread_massive = new HANDLE[num_threads];

        for (int i = 0; i < num_threads; i++) {
            thread_massive[i] = CreateThread(0, 0, thread_entry, (void*)i, 0, 0);
        }
        clock_t start_time = clock();
        for (int i = 0; i < num_threads; i++)
        {
            WaitForSingleObject(thread_massive[i], INFINITE);
            CloseHandle(thread_massive[i]);
        }

        make_qsort(0, arr.size() - 1);

        clock_t end_time = clock();
        measure_time(start_time, "time.txt");

        delete[] thread_massive;
    }
}

int main() {
    if (read_input() != 0) {
        cerr << "Error reading input file." << endl;
        return EXIT_FAILURE;
    }

    multithread_qsort();

    write_output();

    return 0;
}