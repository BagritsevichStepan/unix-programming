#include <iostream>
#include <vector>
#include <cmath>
#include <omp.h>
#include <chrono>
#include <sstream>
#include <fstream>

float detWithoutOmp(int n, std::vector<std::vector<float>> a) {
    float det = 1;
    for (int i = 0; i < n; i++) {
        int maxValueIndex = i;
        for (int j = i + 1; j < n; j++) {
            if (fabs(a[j][i]) > fabs(a[maxValueIndex][i])) {
                maxValueIndex = j;
            }
        }

        if (a[maxValueIndex][i] == 0) {
            det = 0;
            break;
        }

        if (maxValueIndex != i) {
            det = -det;
            for (int j = i; j < n; j++) {
                std::swap(a[i][j], a[maxValueIndex][j]);
            }
        }

        det *= a[i][i];
        for (int j = i + 1; j < n; j++) {
            a[j][i] /= a[i][i];
            for (int z = i + 1; z < n; z++) {
                a[j][z] -= a[i][z] * a[j][i];
            }
            a[j][i] = 0;
        }
    }

    return det;
}

int main(int argc, char *argv[]) {
    const int firstIndex = 1;
    const int argCnt = argc - firstIndex;

    if (argCnt < 2) {
        std::cout << "Error: Not enough arguments";
        return 0;
    }

    int threadsCnt;
    std::stringstream convert(argv[firstIndex]);
    if (!(convert >> threadsCnt)) {
        std::cout << "Error: First argument isn't integer";
        return 0;
    }

    std::ifstream fin(argv[firstIndex + 1]);
    if (!fin.is_open()) {
        std::cout << "Error: Input file not found";
        return 0;
    }

    int n;
    if (!(fin >> n)) {
        std::cout << "Error: N isn't integer";
        return 0;
    }

    std::vector<std::vector<float>> a(n, std::vector<float>(n));
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            if (!(fin >> a[i][j])) {
                std::cout << "Error: Element (row - " + std::to_string(i + 1) + ", column - " + std::to_string(j + 1) + ") isn't float";
                return 0;
            }
        }
    }

    fin.close();

    auto s1 = std::chrono::steady_clock::now();
    detWithoutOmp(n, a);
    auto e1 = std::chrono::steady_clock::now();

    float det = 1;
    auto s2 = std::chrono::steady_clock::now();
    if (threadsCnt) {
        omp_set_num_threads(threadsCnt);
    }

    for (int i = 0; i < n; i++) {
        int maxValueIndex = i;

#pragma omp parallel
        {
            int localMaxValueIndex = i;
#pragma omp for schedule(static)
            for (int j = i + 1; j < n; j++) {
                if (fabs(a[j][i]) > fabs(a[localMaxValueIndex][i])) {
                    localMaxValueIndex = j;
                }
            }
#pragma omp critical
            {
                if (fabs(a[localMaxValueIndex][i]) > fabs(a[maxValueIndex][i])) {
                    maxValueIndex = localMaxValueIndex;
                }
            }
        }

        if (a[maxValueIndex][i] == 0) {
            det = 0;
            break;
        }

        if (maxValueIndex != i) {
            det = -det;
#pragma omp parallel for schedule(static)
            for (int j = i; j < n; j++) {
                std::swap(a[i][j], a[maxValueIndex][j]);
            }
        }

        det *= a[i][i];
#pragma omp parallel for schedule(static)
        for (int j = i + 1; j < n; j++) {
            a[j][i] /= a[i][i];
            for (int z = i + 1; z < n; z++) {
                a[j][z] -= a[i][z] * a[j][i];
            }
            a[j][i] = 0;
        }
    }

    auto e2 = std::chrono::steady_clock::now();
    if (argCnt > 2) {
        std::ofstream fout(argv[firstIndex + 2]);
        if (fout.fail()) {
            std::cout << "Error: Fail with output file";
            return 0;
        }
        fout << det;
    } else {
        std::cout << det << '\n';
    }

    std::cout << "Time: " << std::chrono::duration_cast<std::chrono::milliseconds>(e1 - s1).count() << "ms\n";
    std::cout << "Time (" << threadsCnt << " thread" << (threadsCnt > 1 ? "s" : "") << "): " << std::chrono::duration_cast<std::chrono::milliseconds>(e2 - s2).count() << "ms\n";
    return 0;
}

