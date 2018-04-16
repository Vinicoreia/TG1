#include<bits/stdc++.h>
#include "util.h"
    using namespace std;
int main(){
    std::string teste = "test es!´~âã";
    std::string bitstring;
    for (int i =0; i<teste.size();i++){
    std::bitset<8> t =(teste[i]);
    bitstring.append(t.to_string());
    }

    for(int i=0; i<bitstring.size(); i+=8){
        std::cout << static_cast<char>(stoi(bitstring.substr(i,8), 0, 2));
    }


    return 0;
}
