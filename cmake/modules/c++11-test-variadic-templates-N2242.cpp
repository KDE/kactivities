#include <iostream>

template <typename T>
int addall(T value)
{
    return value;
}

template <typename T, typename... Args>
int addall(T value, Args ... args)
{
    return value + addall(args...);
}

int main()
{
    int v1 = addall(1, 2, 3, 4, 5); // 15
    int v2 = addall(1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4); // 40
    return ((v1 == 15) && (v2 == 40)) ? 0 : 1;
}
