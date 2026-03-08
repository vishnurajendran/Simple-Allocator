//
// Created by ssj5v on 24-10-2025.
//
#include "Allocator.h"
#include <iostream>
#include <cstdlib>

#include "FreeSpace.h"

Allocator::Allocator() : Allocator(sizeof(char) * 1024, Strategy::NO_EXPAND)
{

}

Allocator::Allocator(const SIZE blobSize, const Strategy& InStrategy)
{
    MemoryBasePointer = new char*;
    MemoryStrat = InStrategy;
    MemoryBlob = static_cast<char*>(malloc(blobSize));
    *MemoryBasePointer = MemoryBlob;
    TotalSizeBlobSize = blobSize;
    std::cout<<"Allocator::Allocated "<< blobSize <<" Bytes"<<std::endl;
}

Allocator::~Allocator()
{
    for (const auto& kv : OnDestructEventListeners)
    {
        // call function
        kv.second();
    }
    OnDestructEventListeners.clear();

    // Clean free space list
    while (FreeListHead != nullptr)
    {
        auto ptr = FreeListHead;
        FreeListHead = FreeListHead->Next;
        delete ptr;
    }

    // Free Base Address Pointer
    delete MemoryBasePointer;

    // Free the blob
    free(MemoryBlob);
    std::cout<<"Allocator::Cleanup complete"<<std::endl;
}

bool Allocator::IsValid() const
{
    return MemoryBlob != nullptr && TotalSizeBlobSize > 0;
}

std::unordered_map<Guid, int>& Allocator::GetRefCountMap()
{
    return RefCounts;
}

void Allocator::CopyMemoryBlob(char* blob) const
{
    //blob loaded from disk -> MemoryBlob
    memcpy(blob, MemoryBlob, TotalSizeBlobSize);
}

void Allocator::Free(const SIZE& InSize, const SIZE& InOffset)
{
    if (FreeListHead == nullptr)
    {
        FreeListHead = FreeListTail = new FreeMemSpace{
            .Size = InSize,
            .Offset = InOffset,
            .Next = nullptr
        };
    }
    else
    {
        FreeListTail->Next = new FreeMemSpace{
            .Size = InSize,
            .Offset = InOffset,
            .Next = nullptr
        };
        FreeListTail = FreeListTail->Next;
    }
    std::cout<<"Allocator:: Freed"<<std::endl;
}

int Allocator::MemHoles() const
{
    auto curr =  FreeListHead;
    int count = 0;
    while (curr != nullptr)
    {
        curr = curr->Next;
        count++;
    }
    return count;
}

void Allocator::RegisterOnDeathCallback(const Guid& callBackId, const std::function<void()>& InDeathCallback)
{
    OnDestructEventListeners[callBackId] = InDeathCallback;
}

void Allocator::UnregisterOnDeathCallback(const Guid& callbackId)
{
    if (OnDestructEventListeners.contains(callbackId))
        OnDestructEventListeners.erase(callbackId);
}

bool Allocator::FindFreeSpace(const SIZE& RequiredSize, SIZE& NewOffset)
{
    // Basic strategy is to bump
    auto currPositionInMemory = index;
    FreeMemSpace* prev = nullptr;
    auto iter = FreeListHead;
    while (iter != nullptr)
    {
        if (iter->Size >= RequiredSize)
        {
            currPositionInMemory = iter->Offset;
            iter->Offset += RequiredSize;
            iter->Size -= RequiredSize;
            if (iter->Size == 0)
            {
                if (iter == FreeListHead)
                {
                    FreeListHead = iter->Next;
                }
                else if (prev != nullptr) //prev is only null when curr is Head.
                {
                    prev->Next = iter->Next;
                }
                delete iter; //remove this entry
            }
            return true;
        }
        prev = iter;
        iter = iter->Next;
    }

    // we exceeded memory limits.
    if (MemoryBlob + currPositionInMemory + RequiredSize > MemoryBlob + TotalSizeBlobSize)
    {
        if (MemoryStrat == Strategy::NO_EXPAND)
            return false;
        else
        {
            ExpandMemory();
            return FindFreeSpace(RequiredSize, NewOffset);
        }
    }

    NewOffset = currPositionInMemory;
    index += RequiredSize;
    return true;
}

void Allocator::ExpandMemory()
{
    SIZE newSize = TotalSizeBlobSize;

    if (MemoryStrat == Strategy::EXPAND_50)
    {
        newSize += newSize/2;
    }
    else
    {
        newSize += newSize;
    }

    if (newSize == TotalSizeBlobSize)
    {
        throw std::runtime_error("New Memory Size is Equal to Old Size, Invalid Expansion parameter");
    }

    MemoryBlob = static_cast<char*>(realloc(MemoryBlob, newSize));
    *MemoryBasePointer = MemoryBlob;
    TotalSizeBlobSize = newSize;
}
