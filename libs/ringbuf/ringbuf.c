#include "../ringbuf.h"
#include <stdint.h>
#include <stdbool.h>
#include <size_t.h>
#include <_null.h>
#include <liballoc.h>

// --- Encapsulated structure ---

struct __ringbuf {
    uintptr_t* buffer;
    size_t head;
    size_t tail;
    size_t max;
};

// === PRIVATE FUNCTIONS ========================

// === PUBLIC FUNCTIONS =========================

RingBufHandle rb_init(uintptr_t* buf, size_t size) {
    if (!(buf && size)) return nullptr;

    RingBufHandle ring = (RingBufHandle)lmalloc(sizeof(RingBuf));
    ring->buffer = buf;
    ring->max = size;
    rb_clear(ring);

    return ring;
}

void rb_free(RingBufHandle self) {
    if (self == nullptr) return;

    lfree(self);
}

void rb_clear(RingBufHandle self) {
    if (self == nullptr) return;

    self->head = 0;
    self->tail = 0;
}

RingBufResult rb_put(RingBufHandle self, uintptr_t data) {
    if (self == nullptr || !self->buffer) return RINGBUF_ERROR;

    if (!rb_is_full(self)) {
        self->buffer[self->head] = data;
        self->head = (self->head + 1) % self->max;
    } else {
        return RINGBUF_FULL;
    }

    return RINGBUF_SUCCESS;
}

RingBufResult rb_get(RingBufHandle self, uintptr_t* data) {
    if (self == nullptr || !self->buffer) return RINGBUF_ERROR;

    if (!rb_is_empty(self)) {
        *data = self->buffer[self->tail];
        self->tail = (self->tail + 1) % self->max;
    } else {
        return RINGBUF_EMPTY;
    }

    return RINGBUF_SUCCESS;
}

RingBufResult rb_peek(RingBufHandle self, uintptr_t* data) {
    if (self == nullptr || !self->buffer) return RINGBUF_ERROR;
    
    if (!rb_is_empty(self)) {
        *data = self->buffer[self->tail];
    } else {
        return RINGBUF_EMPTY;
    }

    return RINGBUF_SUCCESS;
}

bool rb_is_empty(RingBufHandle self) {
    if (self == nullptr) return false;
    return self->head == self->tail;
}

bool rb_is_full(RingBufHandle self) {
    if (self == nullptr) return false;
    return ((self->head + 1) % self->max) == self->tail;
}

size_t rb_get_capacity(RingBufHandle self) {
    if (self == nullptr) return 0;
    return self->max;
}

size_t rb_get_size(RingBufHandle self) {
    if (self == nullptr) return 0;
    
    if (rb_is_full(self)) return self->max;
    if (self->head >= self->tail) return self->head - self->tail;
    else return self->max + self->head - self->tail;

    return self->max;
}

