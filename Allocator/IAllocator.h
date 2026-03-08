//
// Created by ssj5v on 08-03-2026.
//

#ifndef IALLOCATOR_H
#define IALLOCATOR_H
#include <functional>
#include <unordered_map>

struct Guid;

class IAllocator
{
public:
    IAllocator() = default;
    virtual ~IAllocator() = default;
public:
    [[nodiscard]] virtual bool IsValid() const = 0;
    [[nodiscard]] virtual std::unordered_map<Guid, int>& GetRefCountMap()=0;

    virtual void RegisterOnDeathCallback(const Guid& callBackId, const std::function<void()>& InDeathCallback) = 0;
    virtual void UnregisterOnDeathCallback(const Guid& callbackId) = 0;
};

#endif //IALLOCATOR_H
