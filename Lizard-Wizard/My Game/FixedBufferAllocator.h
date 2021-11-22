#ifndef FIXEDBUFFERALLOCATOR_H
#define FIXEDBUFFERALLOCATOR_H

#include "Defines.h"
#include <Abort.h>

struct FixedBufferAllocator {
    u8* ptr;
    usize cap;
    usize len;

    /// Initializes an existing allocator with the pointer.
    void Init(u8* ptr, usize cap) {
        this->ptr = ptr;
        this->cap = cap;
        this->len = 0;
    }

    /// Frees the contents of the associated pointer.
    void Deinit() {
        delete this->ptr;
        this->cap = 0;
        this->len = 0;
    }

    /// Reset the allocator to length 0.
    void Reset() {
        this->len = 0;
    }

    /// Reset the allocator to a specified length.
    /// The requested length must be less than cap and len.
    /// Technically any data allocated past this length could still be validly used,
    /// so make sure that you're properly invalidating things that should be invalid.
    void ResetTo(usize request) {
        if (request > this->cap) {
            ABORT("Requested reset was larger than cap!");
        } else if (request > this->len) {
            ABORT("Requested reset was larger than len!");
        }
    
        this->len = request;
    }

    /// Allocates the requested number of bytes into the allocator.
    u8* Alloc(usize request) {
        uintptr_t new_len = this->len + request;
    
        if (new_len > this->cap) { // bad allocation
            char buffer[256];
            sprintf(
                buffer,
                "Requested allocation of %llu bytes would offset buffer to %llu bytes when buffer can only hold %llu bytes!\n",
                request, new_len, this->cap
            );
    
            ABORT(buffer);
        }
    
        // actually allocate
        uint8_t* pointer = (uint8_t*)(this->ptr + this->len);
        this->len += request;
        return pointer;
    }

    template <typename T>
    T* Create(T t) {
        const u8* ptr = this->Alloc(sizeof(T));
        memcpy((void*)ptr, &t, sizeof(T));
        return (T*)ptr;
    }
};

#endif
