#include <iostream> // cout, endl
#include <cstdlib> // EXIT_SUCCESS, EXIT_FAILURE
#include <fscc.hpp> // Fscc::Port

int main(void)
{
    Fscc::Port p(0);

    // Send "Hello world!" text
    p.Write("Hello world!");

    // Read the data back in (with our loopback connector)
    std::cout << p.Read() << std::endl;

    return EXIT_SUCCESS;
}
