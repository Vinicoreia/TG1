#include <iostream>
#include <string>
#include <boost/algorithm/searching/boyer_moore.hpp>
#include <typeinfo>

int main()
{   std::vector<uint8_t> c;
    c.push_back(0);
    std::vector<uint8_t> corpus;
    corpus.push_back(1);
    std::string pattern("");
    std::pair<__gnu_cxx::__normal_iterator<unsigned char *, std::vector<unsigned char>>, __gnu_cxx::__normal_iterator<unsigned char *, std::vector<unsigned char>>> b =
    boost::algorithm::boyer_moore_search(corpus.begin(), corpus.end(), c.begin(), c.end());
    std::cout << typeid(boost::algorithm::boyer_moore_search(corpus.begin(), corpus.end(),
                                                      pattern.begin(), pattern.end())).name();

 
    std::cout << "\n"<<&b.first[0];
    return 0;
}