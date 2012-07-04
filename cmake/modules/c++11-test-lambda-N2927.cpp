#include <iostream>

int main()
{
    int ref = 10;
    int pass = 2;

    ([&](int mul) {
        ref *= mul;
    })(pass);

    bool checkRefNoref = (ref == 10 * pass);

    int result = ([=](int mul) {
        return ref * mul;
    })(pass);

    bool checkReturn = (result == 10 * pass * pass);

    std::cout
        << "Capture by reference:  " << checkRefNoref << std::endl
        << "Return a value:        " << checkReturn   << std::endl;

    return (checkRefNoref && checkReturn) ? 0 : 1;
}
