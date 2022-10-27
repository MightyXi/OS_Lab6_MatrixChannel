#include <iostream>
#include <thread>
#include <vector>
#include "channel.h"

int** new_matrix(int n) {
    int** matrix = new int*[n];
    for (int i = 0; i < n; ++i) {
        matrix[i] = new int[n];
        for (int j = 0; j < n; ++j) {
            matrix[i][j] = rand() % 10;
        }
    }
    return matrix;
}

void blocksMult(int** a, int** b, int** c, int n, Channel<std::pair<int, int>> channel) {
    std::pair<std::pair<int, int>, bool> temp = channel.Recv();
        c[temp.first.first][temp.first.second] = 0;

            for (int k = 0; k < n; ++k) {
                c[temp.first.first][temp.first.second] += a[temp.first.first][k] * b[k][temp.first.second];
            }
}

void pushChannel(int n, int block, Channel<std::pair<int, int>> channel, int I, int J){
    for (int i = I; i < std::min(n, I + block); ++i) {
        for (int j = J; j < std::min(n, J + block); ++j) {
            channel.Send(std::make_pair(i, j));
        }
    }
}


int thread_mult(int** a, int** b, int** c, int n, int block, Channel<std::pair<int, int>> channel) {
    std::vector<std::thread> threads;
    for (int I = 0; I < n; I += block) {
        for (int J = 0; J < n; J += block) {
            pushChannel(n, block, channel, I, J);
            auto thr = std::thread(blocksMult, a, b, c, n, channel);
            threads.push_back(std::move(thr));
        }
    }
    for (auto& thr : threads) {
        thr.join();
    }
    return threads.size();
}

void deleteMatrix(int** matrix) {
    delete[] matrix[0];
    delete[] matrix;
}

void single_thread_mult(int** a, int** b, int** c, int n, int block,  const Channel<std::pair<int, int>>& channel) {
    for (int I = 0; I < n; I += block) {
        for (int J = 0; J < n; J += block) {
            pushChannel(n, block, channel, I, J);
            blocksMult(a, b, c, n, channel);
        }
    }
}


    void threadMult_experement(int n, Channel<std::pair<int, int>> channel, int i){
        int** a = new_matrix(n);
        int** b = new_matrix(n);
        int** c = new_matrix(n);
            auto start = std::chrono::steady_clock::now();
            int thread = thread_mult(a, b, c, n, i, channel);
            auto end = std::chrono::steady_clock::now();
            auto start2 = std::chrono::steady_clock::now();
            single_thread_mult(a, b, c, n, i, channel);
            auto end2 = std::chrono::steady_clock::now();
            std::cout << "Size of block = " << i << ", thread amount = " << thread << ", duration = " <<
                      std::chrono::duration_cast<std::chrono::microseconds>(end - start).count()
                      << ", single thread duration = " <<
                      std::chrono::duration_cast<std::chrono::microseconds>(end2 - start2).count() << '\n';

        deleteMatrix(a);
        deleteMatrix(b);
        deleteMatrix(c);
}

int main() {
    Channel<std::pair<int, int>> channel1(2);
    Channel<std::pair<int, int>> channel2(122);
    Channel<std::pair<int, int>> channel3(21*21+1);
    Channel<std::pair<int, int>> channel4(31*31+1);
    Channel<std::pair<int, int>> channel5(41*41+1);
    threadMult_experement(50, channel1, 1);
    threadMult_experement(50, channel2, 11);
    threadMult_experement(50, channel3, 21);
    threadMult_experement(50, channel4, 31);
    threadMult_experement(50, channel5, 41);
std::cout << "\n" << "\n";
}
