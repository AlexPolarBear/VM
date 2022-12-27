#include<bits/stdc++.h>
#include <chrono>
#include <ctime>
#include <ratio>
#include <chrono>
#include <unistd.h>
using namespace std;

int rand(int n)
{
    return rand() % n ;
}

void f(int N, std::ofstream& of) {
    // cout << N << endl;
    int* a = new int[N];
    for (int i = 0; i < N; i++) {
        a[i] = i;
    }
    random_shuffle(a, a + N, pointer_to_unary_function<int, int>(rand));
    int* b = new int[N];
    for (int i = 0; i < N; i++) {
        b[a[i]] = a[(i + 1)%N];
    }

    delete a;
    a = b;


    auto start = std::chrono::high_resolution_clock::now();

    int i = 0;
    do {
       i = a[i];
    } while (i != 0);

    auto end = std::chrono::high_resolution_clock::now();
    of << N << ", " << setprecision(6) << std::chrono::duration_cast<std::chrono::duration<double>>(end - start).count() << endl;

    delete a;
}

void f2(int N, int K, std::ofstream& of) {
    int* a = new int[N];
    
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < N; i += K) {
        a[i]++;
    }
    auto end = std::chrono::high_resolution_clock::now();
    of << K << ", " << setprecision(6) << std::chrono::duration_cast<std::chrono::duration<double>>(end - start).count() << endl;

    delete a;
}

int main(int argc, char *argv[]) {
    std::ofstream out_f;
    out_f.open ("result.csv");
    
    int page_size = getpagesize();
    
    for (int i = 1; i < (1 << 20); i *= 2 ) {
        f(i, out_f);
        cout << i  << "/" << (1 << 30) << endl;
    }

    // for (int i = 1; i < (1 << 30); i *= 2 ) {
    //     f2((1 << 30), i, out_f);
    //     cout << i  << "/" << (1 << 25) << endl;
    // }

    // for (int i = (1 << 13); i < (1 << 16); i += page_size ) {
    //     f(i, out_f);
    // }
    
    out_f.close();
}  