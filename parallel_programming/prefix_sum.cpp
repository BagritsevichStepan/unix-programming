#include <iostream>
#include <vector>
#include <omp.h>
#include <chrono>
#include <sstream>
#include <fstream>

using namespace std;

vector<int> prefSum(const vector<int> &a) {
    int n = (int)a.size();
    vector<int> prevPrefSum = a, curPrefSum = a;

    for (int step = 0; (1 << step) < n; step++) {
        for (int i = 0; i < n - (1 << step); i++) {
            curPrefSum[i + (1 << step)] += prevPrefSum[i];
        }

        prevPrefSum = curPrefSum;
    }

    return curPrefSum;
}

vector<int> prefSumParallelAlgorithm(const vector<int> &a, const int threadsCnt) {
    int n = (int)a.size();
    vector<int> prevPrefSum = a, curPrefSum = a;

    if (threadsCnt) {
        omp_set_num_threads(threadsCnt);
    }

    for (int step = 0; (1 << step) < n; step++) {
#pragma omp for schedule(static)
        for (int i = 0; i < n - (1 << step); i++) {
            curPrefSum[i + (1 << step)] += prevPrefSum[i];
        }

        prevPrefSum = curPrefSum;
    }

    return curPrefSum;
}

int main(int argc, char *argv[]) {
    const int firstIndex = 1;
    const int argCnt = argc - firstIndex;

    if (argCnt < 2) {
        cout << "Error: Not enough arguments";
        return 0;
    }

    int threadsCnt;
    stringstream convert(argv[firstIndex]);
    if (!(convert >> threadsCnt)) {
        cout << "Error: First argument isn't integer";
        return 0;
    }

    ifstream fin(argv[firstIndex + 1]);
    if (!fin.is_open()) {
        cout << "Error: Input file not found";
        return 0;
    }

    int n;
    if (!(fin >> n)) {
        cout << "Error: N isn't integer";
        return 0;
    }

    vector<int> a(n);
    for (int i = 0; i < n; i++) {
        if (!(fin >> a[i])) {
            cout << "Error: Element " + to_string(i + 1) + " isn't integer";
            return 0;
        }
    }

    auto sequentialAlgorithmStart = chrono::steady_clock::now();
    vector<int> prefSequentialAlgorithm = prefSum(a);
    auto sequentialAlgorithmEnd = chrono::steady_clock::now();

    auto parallelAlgorithmStart = chrono::steady_clock::now();
    vector<int> prefParallelAlgorithm = prefSumParallelAlgorithm(a, threadsCnt);
    auto parallelAlgorithmEnd = chrono::steady_clock::now();

    if (argCnt > 2) {
        ofstream fout(argv[firstIndex + 2]);
        if (fout.fail()) {
            cout << "Error: Fail with output file";
            return 0;
        }

        fout << n << '\n';
        for (const int &x : prefParallelAlgorithm) {
            fout << x << ' ';
        }
        fout << '\n';
    } else {
        cout << n << '\n';
        for (const int &x : prefParallelAlgorithm) {
            cout << x << ' ';
        }
        cout << '\n';
    }

    cout << "Time: " << chrono::duration_cast<chrono::milliseconds>(sequentialAlgorithmEnd - sequentialAlgorithmStart).count() << "ms\n";
    cout << "Time (" << threadsCnt << " thread" << (threadsCnt > 1 ? "s" : "") << "): " << chrono::duration_cast<chrono::milliseconds>(parallelAlgorithmEnd - parallelAlgorithmStart).count() << "ms\n";
    return 0;
}

