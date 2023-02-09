#include <iostream>
#include <vector>

// `auto` return type. The compile figures out the correct type for you
auto square(int num)
{
    return num * num;
}

// Also works with function templates
template<typename T>
auto minimum(T a, T b)
{
    return a < b ? a : b; 
}

int main()
{
    std::cout << square(9) << std::endl;
    std::cout << minimum(0.5f, 0.6f) << std::endl;
    std::cout << minimum(999ul, 888ul) << std::endl;

    return 0;
}