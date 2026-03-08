//
// Created by ssj5v on 24-10-2025.
//
#ifndef REFPTR_H
#define REFPTR_H

#include <cassert>
#include <functional>
#include <unordered_map>
#include <typeindex>

#include "IAllocator.h"
#include "Typedefs.h"

#include "../Utils/Guid.h"

typedef std::function<void(SIZE size, SIZE offset)> DEALLOC_FUNCTION_TYPE;
typedef std::function<void(SIZE offset)> DESTRUCTION_FUNCTION_TYPE;

struct RefHandle
{
public:
    RefHandle();
    RefHandle(IAllocator* InAllocator, char** BasePtr, const SIZE& InOffset, const SIZE& InSize, const DEALLOC_FUNCTION_TYPE& DeallocFunction, const DESTRUCTION_FUNCTION_TYPE& DestructorFunction,
           const std::type_info& InDataType);

    RefHandle(const RefHandle& InHandle);
    ~RefHandle();

public:
    RefHandle& operator=(const RefHandle& InHandle);
    bool operator==(const RefHandle& InHandle) const;
    bool operator!=(const RefHandle& InHandle) const;


public:
    [[nodiscard]] bool IsValid() const;

    /// Resolves handle to type, strictly checks for type safety
    template<typename T>
    T& GetSafe() const
    {
        // assert for type match.
        assert(typeid(T) == StoredDatatype);
        return Get<T>();
    }

    /// [UNSAFE]\n
    /// Resolve the handle to the required to type.\n
    /// NOTE: It is recommended to use SafeResolve wherever possible.
    template<typename T>
    T& Get() const
    {
        //safety check, return nullptr instead of an invalid ptr.
        assert(IsValid());
        return *reinterpret_cast<T*>(*BaseAddr + Offset);
    }

private:
    void RegisterAllocatorCallbacks();
    void UnregisterAllocatorCallbacks();

    void IncrementReference();
    void DecrementReference();
    void InValidateHandle();
    void Free();

private:
    Guid ReferenceGuid;
    char** BaseAddr = nullptr;
    SIZE Offset = 0;
    SIZE Size = 0;
    bool Freed = false;
    std::type_index StoredDatatype = typeid(void*);

    IAllocator* ParentAllocator = nullptr;

    DEALLOC_FUNCTION_TYPE DeallocFunctr = nullptr;
    DESTRUCTION_FUNCTION_TYPE DestructerFunctr = nullptr;

    // unique to all handles.
    Guid HandleId;
};

#endif //REFPTR_H
