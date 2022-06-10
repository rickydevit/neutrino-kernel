#pragma once
#include <stdint.h>
#include <size_t.h>
#include <stdbool.h>

typedef struct __ringbuf RingBuf;
typedef RingBuf* RingBufHandle;

typedef enum __ringbuf_result {
    RINGBUF_SUCCESS,
    RINGBUF_ERROR,
    RINGBUF_FULL,
    RINGBUF_EMPTY
} RingBufResult;

RingBufHandle rb_init(uintptr_t* buf, size_t size);
void rb_free(RingBufHandle self);

void rb_clear(RingBufHandle self);
RingBufResult rb_put(RingBufHandle self, uintptr_t data);
RingBufResult rb_get(RingBufHandle self, uintptr_t* data);
RingBufResult rb_peek(RingBufHandle self, uintptr_t* data);
bool rb_is_empty(RingBufHandle self);
bool rb_is_full(RingBufHandle self);
size_t rb_get_capacity(RingBufHandle self);
size_t rb_get_size(RingBufHandle self);
