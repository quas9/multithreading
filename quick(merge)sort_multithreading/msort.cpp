#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <fstream>
#include <vector>
#include <windows.h>
#include <time.h>

using namespace std;

vector<int> arr;
int num_threads = 0;
int arr_size = 0;
HANDLE mutex;

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

struct TASK {

    int low;
    int high;
    int* a;
};

void merge(int* a, int low, int mid, int high)
{
    int n1 = mid - low + 1;
    int n2 = high - mid;

    int* left = (int*)malloc(n1 * sizeof(int));
    int* right = (int*)malloc(n2 * sizeof(int));

    int i;
    int j;

    for (i = 0; i < n1; i++) left[i] = a[i + low];
    for (i = 0; i < n2; i++) right[i] = a[i + mid + 1];

    int k = low;

    i = j = 0;

    while (i < n1 && j < n2)
    {
        if (left[i] <= right[j])
            a[k++] = left[i++];
        else
            a[k++] = right[j++];
    }

    while (i < n1) a[k++] = left[i++];
    while (j < n2) a[k++] = right[j++];

    free(left);
    free(right);
}

void merge_sort(int* a, int low, int high)
{
    int mid = low + (high - low) / 2;

    if (low < high)
    {
        merge_sort(a, low, mid);
        merge_sort(a, mid + 1, high);
        merge(a, low, mid, high);
    }
}

DWORD WINAPI merge_sort_thread(void* arg)
{
    TASK* task = (TASK*)arg;
    int low;
    int high;
    low = task->low;
    high = task->high;

    int mid = low + (high - low) / 2;

    if (low < high)
    {
        WaitForSingleObject(mutex, INFINITE);
        merge_sort(task->a, low, mid);
        merge_sort(task->a, mid + 1, high);
        merge(task->a, low, mid, high);
        ReleaseMutex(mutex);

    }
    return 0;
}

void multi_threaded_merge_sort()
{
    mutex = CreateMutex(NULL, FALSE, NULL);
    int len = arr_size / num_threads;
    TASK* tasklist = (TASK*)malloc(sizeof(TASK) * num_threads);
    HANDLE* threads = (HANDLE*)malloc(sizeof(HANDLE) * num_threads);

    int low = 0;

    for (int i = 0; i < num_threads; i++, low += len)
    {
        tasklist[i].low = low;
        tasklist[i].high = low + len - 1;
        if (i == (num_threads - 1))
            tasklist[i].high = arr_size - 1;
    }

    clock_t start_time = clock();
    for (int i = 0; i < num_threads; i++)
    {
        tasklist[i].a = &arr[0];
        threads[i] = CreateThread(NULL, 0, merge_sort_thread, &tasklist[i], 0, NULL);
    }

    WaitForMultipleObjects(num_threads, threads, TRUE, INFINITE);
    TASK* taskm = &tasklist[0];
    for (int i = 1; i < num_threads; i++)
    {
        TASK* task = &tasklist[i];
        merge(taskm->a, taskm->low, task->low - 1, task->high);

    }
    measure_time(start_time, "time.txt");
    
    CloseHandle(mutex);
    free(tasklist);
    free(threads);

}

int main() {
    

    read_input();
    multi_threaded_merge_sort();
    write_output();

    
}