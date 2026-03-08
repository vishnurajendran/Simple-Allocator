//
// Created by ssj5v on 24-10-2025.
//

#ifndef FREESPACE_H
#define FREESPACE_H
#include "Typedefs.h"

struct FreeMemSpace
{
public:
    SIZE Size;
    SIZE Offset;
    FreeMemSpace* Next;
};

#endif //FREESPACE_H
