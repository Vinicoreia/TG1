#include <iostream>
#include <string>
#include <boost/algorithm/searching/boyer_moore.hpp>
#include <typeinfo>

int main()
{
    std::string corpus("ai você");
    std::string pattern("você");
    std::pair<__gnu_cxx::__normal_iterator<char *, std::__cxx11::basic_string<char>>, __gnu_cxx::__normal_iterator<char *, std::__cxx11::basic_string<char>>> b = boost::algorithm::boyer_moore_search(corpus.begin(), corpus.end(),
                                                                                                                                                                                                       pattern.begin(), pattern.end());
    // std::cout << typeid(boost::algorithm::boyer_moore_search(corpus.begin(), corpus.end(),
    //                                                   pattern.begin(), pattern.end())).name();

 
    // std::cout << "\n"<<&b.second[0];
    char a = 'ẽ';
    std::cout<<a;
    return 0;
}