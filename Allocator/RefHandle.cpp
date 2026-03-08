#include "RefHandle.h"
#include "../Utils/Guid.h"

RefHandle::RefHandle()
{
    // assign handle id
    HandleId = Guid::New();
}

RefHandle::RefHandle(IAllocator* InAllocator, char** BasePtr, const SIZE& InOffset, const SIZE& InSize,
                     const DEALLOC_FUNCTION_TYPE& DeallocFunction, const DESTRUCTION_FUNCTION_TYPE& DestructorFunction,
                     const std::type_info& InDataType) : RefHandle()
{

    ReferenceGuid = Guid::New();
    BaseAddr = BasePtr;
    Offset = InOffset;
    Size = InSize;
    DeallocFunctr = DeallocFunction;
    DestructerFunctr = DestructorFunction;
    StoredDatatype = InDataType;
    ParentAllocator = InAllocator;

    //register to parent allocator events
    RegisterAllocatorCallbacks();

    IncrementReference();
}

RefHandle::RefHandle(const RefHandle& InHandle) : RefHandle()
{
    ReferenceGuid = InHandle.ReferenceGuid;
    BaseAddr = InHandle.BaseAddr;
    Offset = InHandle.Offset;
    Size = InHandle.Size;
    DeallocFunctr = InHandle.DeallocFunctr;
    DestructerFunctr = InHandle.DestructerFunctr;
    StoredDatatype = InHandle.StoredDatatype;
    ParentAllocator = InHandle.ParentAllocator;
    Freed = InHandle.Freed;

    //register to parent allocator events
    RegisterAllocatorCallbacks();

   IncrementReference();
}

RefHandle::~RefHandle()
{
    DecrementReference();
    InValidateHandle();
}

RefHandle& RefHandle::operator=(const RefHandle& InHandle)
{
    if (this == &InHandle)
        return *this;

    if (IsValid())
    {
        UnregisterAllocatorCallbacks();
        DecrementReference();
    }

    ReferenceGuid = InHandle.ReferenceGuid;
    BaseAddr = InHandle.BaseAddr;
    Offset = InHandle.Offset;
    Size = InHandle.Size;
    DeallocFunctr = InHandle.DeallocFunctr;
    DestructerFunctr = InHandle.DestructerFunctr;
    StoredDatatype = InHandle.StoredDatatype;
    ParentAllocator = InHandle.ParentAllocator;
    Freed = InHandle.Freed;

    RegisterAllocatorCallbacks();

    IncrementReference();
    return *this;
}

bool RefHandle::operator==(const RefHandle& InHandle) const
{
    return ReferenceGuid == InHandle.ReferenceGuid;
}

bool RefHandle::operator!=(const RefHandle& InHandle) const
{
    return ReferenceGuid != InHandle.ReferenceGuid;
}

bool RefHandle::IsValid() const
{
    return ParentAllocator != nullptr && ParentAllocator->IsValid() &&
                        BaseAddr != nullptr && *BaseAddr != nullptr && !Freed;
}

void RefHandle::InValidateHandle()
{
    if (!IsValid())
        return;

    UnregisterAllocatorCallbacks();
    ParentAllocator = nullptr;
    BaseAddr = nullptr;
    Offset = 0;
}

void RefHandle::Free()
{
    if (Freed)
        return;

    DestructerFunctr(Offset); // call destructor
    DeallocFunctr(Size, Offset);

    InValidateHandle();
    Freed = true;
}

void RefHandle::RegisterAllocatorCallbacks()
{
    //register to parent allocator events
    if (ParentAllocator != nullptr)
    {
        ParentAllocator->RegisterOnDeathCallback(HandleId, [this](){InValidateHandle();});
    }
}

void RefHandle::UnregisterAllocatorCallbacks()
{
    if (ParentAllocator != nullptr)
    {
        ParentAllocator->UnregisterOnDeathCallback(HandleId);
    }
}

void RefHandle::IncrementReference()
{
    if (ParentAllocator == nullptr)
        return;

    auto& refCounts = ParentAllocator->GetRefCountMap();
    refCounts[ReferenceGuid]++;
}

void RefHandle::DecrementReference()
{
    if (ParentAllocator == nullptr)
        return;

    auto& refCounts = ParentAllocator->GetRefCountMap();

    if (!refCounts.contains(ReferenceGuid))
        return;

    refCounts[ReferenceGuid]--;
    if (refCounts[ReferenceGuid] <= 0)
    {
        refCounts.erase(ReferenceGuid);
        Free();
    }
}
