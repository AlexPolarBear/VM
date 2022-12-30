#include <sys/mman.h>
#include <iostream>
#include <chrono>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <vector>
#include <cstring>
#include <limits.h>
#include <exception>

int estimate_cache_line_size(int* a, int size)
{
    int steps = 1024 * 1024;

    std::vector<double> times;
    std::vector<int> strides;

    for (int stride = 4; stride < 256; stride *= 2)
    {
        // Подготоваливаем план обхода
        for (int i = 0; i < size; ++i)
        {
            a[i] = i + stride;
        }

        int i = 0;
        auto start = std::chrono::high_resolution_clock::now();
        while (i < size)
        {
            i = a[i];
        }
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> diff = (end - start);
        // std::cout << "Stride = " << stride * sizeof(int) << ", running time = " << diff.count() << " s\n";

        if (i == INT_MAX)
            throw std::out_of_range("Cannot happen, but forbids the compiler to omit the line with i = a[i]");

        times.push_back(diff.count());
        strides.push_back(stride);
    }


    for (int i = 1; i < times.size(); ++i)
    {
        if (times[i] / times[i - 1] > 1.5)
        {
            return strides[i - 1] * sizeof(int);
        }
    }

    return -1;
}

int estimate_cache_capacity(int* a, int stride_bytes)
{
    int steps = 64 * 1024 * 1024;
    int stride = stride_bytes / sizeof(int);

    std::vector<double> times;
    std::vector<int> sizes;
    for (int arr_size_bytes = 1024; arr_size_bytes <= 8 * 1024 * 1024; arr_size_bytes *= 2)
    {
        // Подготавливаем план обхода
        int arr_size = arr_size_bytes / sizeof(int);
        for (int i = 0; i < arr_size; ++i)
        {
            a[i] = (i + stride) % arr_size;
        }

        int i = 0;
        auto start = std::chrono::high_resolution_clock::now();
        for (int step = 0; step < steps; ++step)
        {
            i = a[i];
            a[i] += 1;
            a[i] -= 1;
        }

        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> diff = (end - start);
        // std::cout << "Capacity = " << arr_size_bytes / 1024 << " kB, running time = " << diff.count() << " s\n";

        if (i == INT_MAX)
            throw std::out_of_range("Cannot happen, but forbids the compiler to omit the line with i = a[i]");

        times.push_back(diff.count());
        sizes.push_back(arr_size_bytes);
    }

    for (int i = 1; i < times.size(); ++i)
    {
        if (times[i] / times[i - 1] > 1.85)
            return sizes[i - 1];
    }
    return -1;
}

int estimate_cache_associativity(int* a, int capacity_bytes)
{
    int steps = 1024 * 1024;
    int capacity = capacity_bytes / sizeof(int);

    std::vector<double> times;
    std::vector<int> associativities;
    for (int k = 1; k <= 128; k *= 2)
    {
        for (int i = 0; i < k * capacity; ++i)
        {
            if (i < (k - 1) * capacity)
                a[i] = i + capacity;
            else
                a[i] = (i + 1) % capacity;
        }

        int i = 0;
        auto start = std::chrono::high_resolution_clock::now();
        for (int step = 0; step < 1024 * 1024; ++step)
        {
            i = a[i];
        }
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> diff = (end - start);

        // std::cout << "k = " << k << ", running time = " << diff.count() << " s\n";
        if (i == INT_MAX)
            throw std::out_of_range("Cannot happen, but forbids the compiler to omit line with i = a[i]");
        associativities.push_back(k);
        times.push_back(diff.count());
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
    size_t const size = 1024 * 1024 * 1024 / sizeof(int);
    // int* a = new int[size];
    int* a = (int*)mmap(NULL, size * sizeof(int), PROT_WRITE | PROT_READ, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    int cache_line_size = -1;
    while (cache_line_size == -1)
        cache_line_size = estimate_cache_line_size(a, size);
    std::cout << "Cache line size: " << cache_line_size << " bytes\n";


    int cache_capacity = -1;
    while (cache_capacity == -1)
        cache_capacity = estimate_cache_capacity(a, cache_line_size);
    std::cout << "Capacity: " << cache_capacity / 1024 << " kB\n";


    int cache_associativity = -1;
    while (cache_associativity == -1)
        cache_associativity = estimate_cache_associativity(a, cache_capacity);
    std::cout << "Associaticity: " << cache_associativity << "\n";

    return 0;
}
