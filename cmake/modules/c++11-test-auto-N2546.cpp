#include <iostream>

int main()
{
    auto i = 5;
    auto f = 3.14159f;
    auto d = 3.14159;
    auto l = 3l;

    bool checkFloats = (sizeof(f) < sizeof(d));
    bool checkInts   = (sizeof(i) == sizeof(int));
    bool checkLongs  = (sizeof(l) == sizeof(long));

    std::cout
        << "Float sizes correct:  " << checkFloats << std::endl
        << "Integer size correct: " << checkFloats << std::endl
        << "Long sizes correct:   " << checkFloats << std::endl;

    return (checkFloats && checkInts && checkLongs) ? 0 : 1;
}
