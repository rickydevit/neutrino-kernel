#pragma once
#include <libs/ipc/ipc.h>
#include <libs/ringbuf.h>
#include <neutrino/lock.h>

#define CHANNEL_BUFFER_SIZE sizeof(uintptr_t)*32

typedef enum __channel_flags {
    CHANNEL_CAN_RECEIVE = 0b001,
    CHANNEL_CAN_SEND = 0b010,
    CHANNEL_CAN_BROADCAST =0b100,
} ChannelFlag;

typedef enum __channel_transmit_result {
    CHANNEL_TRANSMIT_SUCCESS,
    CHANNEL_TRANSMIT_BUFFER_FULL,
    CHANNEL_TRANSMIT_BAD_SENDER,
    CHANNEL_TRANSMIT_UNAUTHORIZED_SENDER,
    CHANNEL_TRANSMIT_BAD_RECIPIENT,
    CHANNEL_TRANSMIT_UNAUTHORIZED_RECIPIENT,
    CHANNEL_TRANSMIT_BAD_PACKAGE,
} ChannelTransmitResult;

typedef enum __channel_receive_result {
    CHANNEL_RECEIVE_SUCCESS,
    CHANNEL_RECEIVE_BUFFER_EMPTY,
    CHANNEL_RECEIVE_BAD_RECEIVER,
    CHANNEL_RECEIVE_UNAUTHORIZED_RECEIVER
} ChannelReceiveResult;

typedef enum __channel_peek_result {
    CHANNEL_PEEK_SUCCESS,
    CHANNEL_PEEK_BUFFER_EMPTY,
    CHANNEL_PEEK_BAD_RECEIVER,
    CHANNEL_PEEK_UNAUTHORIZED_RECEIVER
} ChannelPeekResult;

typedef struct __channel {
    ChannelFlag flags;
    
    Agent agent;
    Lock ring_lock;
    RingBufHandle ring;
    uintptr_t* _buffer;
} Channel;

typedef struct __channel_agent_data {
    Channel* channel;
    Agent* agent;
} ChannelAgentData;

Channel* NewChannel(ChannelFlag flags, const char* agent_name);
void DestroyChannel(Channel* channel);
Channel* channel_find_by_agent_name(const char* name);
Channel* channel_find_by_agent_id(AgentID id);
ChannelTransmitResult channel_transmit(Channel* self, Channel* dest, Package* msg);
ChannelReceiveResult channel_receive(Channel* self, Package** msg);
ChannelPeekResult channel_peek(Channel* self, Package** msg);
