//
// Created by ssj5v on 24-10-2025.
//

#ifndef ALLOCATOR_H
#define ALLOCATOR_H
#include "IAllocator.h"
#include "RefHandle.h"
#include "Strategy.h"

#define DEFAULT_ALLOCATOR_SIZE sizeof(char) * 1024

struct FreeMemSpace;

class Allocator : public IAllocator {
public:

    // Constructors
    Allocator();
    explicit Allocator(Strategy strategy) : Allocator(DEFAULT_ALLOCATOR_SIZE ,strategy) {};
    explicit Allocator(SIZE blobSize, const Strategy& strategy);
    ~Allocator() override;

    [[nodiscard]] bool IsValid() const override;
    [[nodiscard]] std::unordered_map<Guid, int>& GetRefCountMap() override;

    template<typename T>
    bool Create(RefHandle& InHandle) {
        SIZE offset = 0;
        if (!FindFreeSpace(sizeof(T), offset))
            return false;

        // create instance
        auto addr = MemoryBlob + offset;
        auto instance = new (addr) T();

        auto deAllocFunc = [this](const SIZE& inSize,const SIZE& inOffset)
        {
            Free(inSize, inOffset);
        };

        auto destroyFunc = [this](const SIZE& inOffset)
        {
            auto ptr = reinterpret_cast<T*>(MemoryBlob + inOffset);
            ptr->~T();
        };

        // create RefPtr
        InHandle = RefHandle(this, MemoryBasePointer, offset, sizeof(T), deAllocFunc, destroyFunc, typeid(T));
        return true;
    }

    void CopyMemoryBlob(char* blob) const;
    void Free(const SIZE& InSize, const SIZE& InOffset);
    int MemHoles() const;

    void RegisterOnDeathCallback(const Guid& callBackId, const std::function<void()>& InDeathCallback) override;
    void UnregisterOnDeathCallback(const Guid& callbackId) override;

private:
    bool FindFreeSpace(const SIZE& RequiredSize, SIZE& NewOffset);
    void ExpandMemory();

private:
    FreeMemSpace* FreeListHead = nullptr;
    FreeMemSpace* FreeListTail = nullptr;

    char* MemoryBlob=nullptr;

    //passed to RefPointers to get MemoryBlob Pointers
    char** MemoryBasePointer = nullptr;

    SIZE index = 0;
    SIZE TotalSizeBlobSize;
    Strategy MemoryStrat;

    // used by allocators for ref handling
    std::unordered_map<Guid, int> RefCounts;

    // notified when allocator is destructed
    std::unordered_map<Guid, std::function<void()>> OnDestructEventListeners;
};

#endif //ALLOCATOR_H
