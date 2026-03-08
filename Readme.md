
# Simple Allocator (Handle-Based Memory Experiment)

This is a small C++ project where I experimented with writing a **custom memory allocator** that returns **handles instead of raw pointers**.

The main idea was to explore how game engines sometimes manage memory internally without exposing raw pointers everywhere. Instead of returning a `T*`, the allocator returns a `RefHandle` that can safely resolve to the stored object.

The project is mostly a **learning exercise** to better understand:

- custom allocators
- memory relocation
- reference counting
- handle-based memory access
- placement new
- basic memory management strategies

---

# Why I Built This

I’ve always been curious about how engines avoid pointer chaos when objects move in memory or when memory pools expand.

Typical usage in C++ looks like this:

```
T* obj = new T();
```

But that makes it hard to:

- relocate memory
- track object lifetime
- avoid dangling pointers

So this allocator tries a different idea:

```
Allocator -> returns RefHandle
RefHandle -> resolves object safely
```

Handles store offsets into a memory blob instead of raw pointers.

---

# How It Works

The allocator manages a **large memory blob** internally.

Objects are constructed directly inside the blob using **placement new**.

```
auto instance = new (addr) T();
```

Each allocation returns a `RefHandle` which stores:

- offset into the blob
- pointer to the blob base pointer
- type info
- a reference ID
- allocator pointer

To access the object:

```
handle.GetSafe<T>()
```

The handle resolves the object like this:

```
BaseAddress + Offset
```

---

# Memory Relocation Safety

One of the more interesting parts of the project is that the allocator supports **reallocating the memory blob** when it grows.

Handles store a `char**` instead of a `char*`.

So when memory expands:

```
MemoryBlob = realloc(...)
*MemoryBasePointer = MemoryBlob
```

All existing handles automatically point to the new location.

---

# Reference Counting

Each allocation gets a **reference GUID**.

Handles automatically:

- increment reference count when copied
- decrement reference count when destroyed

When the last reference goes away:

```
Destructor is called
Memory is returned to the allocator
```

---

# Type Safety

Handles remember the type of the stored object.

```
handle.GetSafe<T>()
```

Internally it checks:

```
assert(typeid(T) == StoredDatatype);
```

This prevents resolving the memory as the wrong type.

---

# Allocator Death Protection

Handles register a callback with the allocator.

When the allocator is destroyed, it notifies all handles so they invalidate themselves.

---

# Expansion Strategy

The allocator supports simple growth strategies:

```
Strategy::NO_EXPAND
Strategy::EXPAND_50
Strategy::EXPAND_100
```

Example:

```
Allocator allocator(Strategy::EXPAND_50);
```

---

# Example

```
#include "Allocator/Allocator.h"

int main()
{
    Allocator allocator(Strategy::EXPAND_50);

    RefHandle handle;

    allocator.Create<int>(handle);

    int& value = handle.GetSafe<int>();
    value = 42;

    std::cout << value << std::endl;
}
```
---

# Limitations (For Now)

This was mostly an experiment, so a few things are intentionally missing:

- alignment handling
- free block merging
- thread safety
- optimized refcount storage

Those could definitely be added later.
