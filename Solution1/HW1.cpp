//#include <sys/mman.h>
#include <iostream>
#include <chrono>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <vector>
#include <cstring>

int estimate_cache_line_size()
{
    size_t const size = 1024 * 1024 * 1024;

    // char* a = (char*)mmap(NULL, size * sizeof(char), PROT_WRITE | PROT_READ, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    char* a = new char[size];
    memset(a, 0, size);

    std::vector<double> times;
    std::vector<int> strides;

    for (int stride = 16; stride < 1024; stride *= 2)
    {
        auto start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < size; i += stride)
        {
            a[i]++;
        }
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> diff = (end - start);
        // std::cout << "Stride = " << stride << ", running time = " << diff.count() << " s\n";
        times.push_back(diff.count());
        strides.push_back(stride);
    }

    for (int i = 1; i < times.size(); ++i)
    {
        if (times[i] / times[i - 1] < 0.9)
        {
            return strides[i - 1] * sizeof(char);
        }
    }

    delete[] a;

    return -1;
}

int estimate_cache_capacity(int stride_bytes)
{
    int steps = 1024 * 1024;
    int stride = stride_bytes / sizeof(char);

    std::vector<double> times;
    std::vector<int> sizes;
    for (int arr_size = 1024; arr_size <= 4 * 1024 * 1024; arr_size *= 2)
    {
        char* a = new char[arr_size];
        memset(a, 0, arr_size);

        auto start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < steps; ++i)
        {
            a[(i*stride) & (arr_size-1)]++;
        }
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> diff = (end - start);
        // std::cout << "Capacity = " << arr_size / 1024 << " kB, running time = " << diff.count() << " s\n";
        times.push_back(diff.count());
        sizes.push_back(arr_size / 1024);
        delete[] a;
    }

    for (int i = 1; i < times.size(); ++i)
    {
        if (times[i] / times[i - 1] > 1.85)
            return sizes[i - 1] * sizeof(char);
    }
    return -1;
}

int estimate_cache_associativity(int capacity_bytes)
{
    int steps = 1024 * 1024;
    int capacity = capacity_bytes / sizeof(int);
    
    std::vector<double> times;
    std::vector<int> associativities;
    for (int k = 1; k <= 128; k *= 2)
    {
        int* a = new int[k * capacity];
        for (int i = 0; i < k * capacity; ++i)
        {
            if (i < (k - 1) * capacity)
                a[i] = i + capacity;
            else
                a[i] = (i + 1) % capacity;
        }

        int i = 0;
        auto start = std::chrono::high_resolution_clock::now();
        for (int step = 0; step < 1024*1024; ++step)
        {
            i = a[i];
        }
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> diff = (end - start);
        
        // std::cout << "k = " << k << ", running time = " << diff.count() << " s\n";
        if (i == -1)
            throw std::exception("Cannot happen, but forbids the compiler to omit line with i = a[i]");
        associativities.push_back(k);
        times.push_back(diff.count());
        
        delete[] a;
    }

    for (int i = 1; i < times.size(); ++i)
    {
        if (times[i] / times[i - 1] > 2.0)
            return associativities[i - 1];
    }

    return -1;
}

int main()
{
    int cache_line_size = -1;
    while (cache_line_size == -1)
        cache_line_size = estimate_cache_line_size();
    std::cout << "Cache line size: " << cache_line_size << " bytes\n";

    int cache_capacity = -1;
    while (cache_capacity == -1)
        cache_capacity = estimate_cache_capacity(256);
    std::cout << "Capacity: " << cache_capacity << " kB\n";

    int cache_associativity = -1;
    while (cache_associativity == -1)
        cache_associativity = estimate_cache_associativity(cache_capacity * 1024);
    std::cout << "Associaticity: " << cache_associativity << "\n";

    return 0;
}
