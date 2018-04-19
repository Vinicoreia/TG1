#include <iostream>
#include <chrono>
#include <unordered_map>
#include <vector>
using namespace std::chrono;

int main(){
    std::unordered_map <char, std::vector<int>> hash;

    high_resolution_clock::time_point t1 = high_resolution_clock::now();
    int hpe = 32765*5;
    int filesize = 32766;
    if (hpe > filesize)
    {
        hpe = filesize;
    }
    int i = 0;
    while (i < hpe)
    {
        hash[(char)(i%255)].push_back(i);
        i++;
    }
    high_resolution_clock::time_point t2 = high_resolution_clock::now();
    auto duration = duration_cast<microseconds>(t2 - t1).count();
    std::cout << "duration: " << duration << std::endl;
}