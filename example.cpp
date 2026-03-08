#include <iostream>
#include "Allocator/Allocator.h"

int main()
{
    Allocator allocator(Strategy::EXPAND_50);

    RefHandle handle;
    allocator.Create<int>(handle);

    auto& value = handle.GetSafe<int>();
    value = 42;

    std::cout << value << std::endl;
}

