#include <iostream>
#include <limits>
#include <string>

using namespace std;

unsigned binary_to_decimal(unsigned num)
{
    unsigned res = 0;

    for (int i = 0; num > 0; ++i)
    {
        if ((num % 10) == 1)
            res += (1 << i);

        num /= 10;
    }

    return res;
}

std::string toBinary(unsigned int n)
{
    std::string r;
    while (n != 0)
    {
        r = (n % 2 == 0 ? "0" : "1") + r;
        n /= 2;
    }
    return r;
}

std::string decimalToBitString(unsigned int n, unsigned int len)
{
    std::string r;
    while (n != 0)
    {
        r = (n % 2 == 0 ? "0" : "1") + r;
        n /= 2;
    }
    while (r.size() < len)
    {
        r.insert(0, "0");
    }
    return r;
}

int main()
{
   unsigned int a =  binary_to_decimal(01010);

   std::string b = decimalToBitString('�', 8);
   std::cout<<b; 
}