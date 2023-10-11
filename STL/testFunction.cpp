#include "Function.hpp"
#include <cstdio>
#include <iostream>

void sayHello(int i) { printf("#%d Hello\n", i); }

struct printNumberT
{
    void operator()(int i) const { printf("#%d Numbers are: %d, %d\n", i, x, y); }

    int x;
    int y;
};

void repeatTwice(Function<void(int)> const &func)
{
    func(1);
    func(2);
}

int main()
{
    int x = 4;
    int y = 2;
    repeatTwice([=](int i) { printf("#%d Numbers are: %d, %d\n", i, x, y); });
    printNumberT p {x, y};
    repeatTwice(p);
    repeatTwice(sayHello);
    return 0;
}
