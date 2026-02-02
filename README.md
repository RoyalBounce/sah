# SAH — Stack as Heap

SAH (Stack as Heap) is a small, low-level C library that provides a
**stack-backed allocator** implemented on top of `mmap` (Linux) and
virtual memory primitives.

The goal of SAH is to offer a **fast, predictable, stack-like temporary
allocation model**, with guard pages and real stack semantics, while
being controlled explicitly by the user.

This is not a general-purpose heap replacement.
It is designed for **temporary allocations, arenas, parsing, runtimes,
VMs, and performance-critical code**.

## Key Ideas

- Memory is allocated using `mmap` (or `VirtualAlloc` on Windows).
- A guard page is placed below the stack to catch overflows.
- The stack grows **downwards**, like a real CPU stack.
- A base pointer (BP) and stack pointer (SP) are explicitly managed.
- Allocation and deallocation are O(1) and branch-free.
- Overflow is detected by the OS via a page fault (SIGSEGV).

SAH is essentially a **stack allocator / LIFO arena** with real virtual
memory protection.

## Memory Layout

On Linux, SAH maps two regions, a 4 or 8 KB guard page and a 4 KB stack by default

- The stack grows downward.
- SP starts at BP.
- Crossing into the guard page triggers a segmentation fault.

## Design Goals

- Minimal overhead (no bounds checks in hot paths)
- Real stack semantics (BP/SP, downward growth)
- OS-level overflow detection
- Simple and predictable behavior
- Suitable for temporary allocations and arenas

## Public API

### Create / Destroy

```c
struct sah_stack* stack_create(void);
void stack_destroy(struct sah_stack* s);
```
Creates a new stack backed by mmap and installs a guard page.

## Raw Push / Pop (manual size)

These are the lowest-level operations.
```c
void* push(struct sah_stack* s, size_t n);
void  pop(struct sah_stack* s, size_t n);
```
Semantics:

push(n) moves SP down by n bytes and returns a pointer to write to.

pop(n) moves SP up by n bytes.

This is equivalent to:
```asm
sub rsp, n   ; push
add rsp, n   ; pop
```

The caller is responsible for matching push/pop sizes.

## Structured Push / Pop (automatic size tracking)

SAH also provides a metadata-based interface that automatically tracks block sizes.
```c
void* spush(struct sah_stack* s, size_t n);
void  spop(struct sah_stack* s);
```
Semantics:

spush(n) allocates n bytes and stores internal metadata.

spop() automatically pops the last spush() block.

This behaves like a simple stack allocator with headers.

# Example
```c
#define SAH_IMPLEMENTATION
#include "sah.h"

int main(void)
{
    struct sah_stack* s = stack_create();

    int* x = push(s, sizeof(int));
    *x = 123;

    char* buff = spush(s, 32);
    // use buff...

    spop(s);              // frees buf
    pop(s, sizeof(int)); // frees x

    stack_destroy(s);
    return 0;
}
```

# Error Handling and Overflow
SAH intentionally does not perform bounds checks in hot paths.
If the stack grows into the guard page:

The OS will raise a segmentation fault (SIGSEGV on Linux).
This is by design and mimics real stack overflow behavior.

This makes bugs fail fast and keeps the allocator branch-free.

# Platform Support
Linux: Supported

Windows: Work In Progress

# License
BSD 3-Clause
