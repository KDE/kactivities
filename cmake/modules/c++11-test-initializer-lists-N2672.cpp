#include <iostream>
#include <vector>
#include <string>

struct test {
    int i;
    int j;
    double k;
    std::string s;
};

int main()
{
    test t { 1, 2, 4, "asdf" };
    std::vector<int> v { 1, 2, 3, 4 };

    return (
            t.i == v[0]
            && t.j == v[1]
            && t.k > v[2]
            && t.s.size() == v[3]
            )
        ? 0 : 1;
}
