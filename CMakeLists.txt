cmake_minimum_required(VERSION 3.30)
project(CustomAllocator)

set(CMAKE_CXX_STANDARD 20)

add_executable(CustomAllocator example.cpp
        Allocator/Allocator.cpp
        Allocator/Allocator.h
        Allocator/FreeSpace.h
        Allocator/RefHandle.h
        Allocator/Typedefs.h
        Allocator/Strategy.h
        Allocator/IAllocator.h
        Utils/Guid.h
        Allocator/RefHandle.cpp
)
