#include "channel.h"
#include <libs/ringbuf.h>
#include <libs/ipc/ipc.h>
#include <string.h>
#include <liballoc.h>
#include <stdbool.h>
#include <_null.h>
#include <linkedlist.h>

static Lock agent_channel_lock = NewLock;
static List* agent_channel_map = nullptr;

// === PRIVATE FUNCTIONS ========================

bool channel_agent_remove(Channel* channel) {
    lock(&agent_channel_lock);
    List* p = agent_channel_map;
    size_t i = 0;

    while (p != nullptr) {
        ChannelAgentData* data = list_get_value(p);
        
        if (data->channel == channel) {
            agent_channel_map = list_delete_at(agent_channel_map, i);
            unlock(&agent_channel_lock);
            return true;
        }

        p = p->next;
        i++;
    }

    unlock(&agent_channel_lock);
    return false;
}

bool channel_exists(Channel* channel) {
    List* p = agent_channel_map;
    size_t i = 0;

    while (p != nullptr) {
        ChannelAgentData* data = list_get_value(p);
        
        if (data->channel == channel) 
            return true;

        p = p->next;
        i++;
    }

    return false;
}
 
// === PUBLIC FUNCTIONS =========================

Channel* NewChannel(ChannelFlag flags, const char* agent_name) {
    Channel* channel = (Channel*)kmalloc(sizeof(Channel));
    channel->_buffer = (uintptr_t*)kmalloc(CHANNEL_BUFFER_SIZE);
    channel->flags = flags;
    channel->ring_lock = NewLock;
    channel->ring = rb_init(channel->_buffer, CHANNEL_BUFFER_SIZE);
    agent_init(&channel->agent, agent_name);

    ChannelAgentData* data = (ChannelAgentData*)kmalloc(sizeof(ChannelAgentData));
    data->agent = &channel->agent;
    data->channel = channel;
    LockOperation(agent_channel_lock, 
        agent_channel_map = list_append(agent_channel_map, data));

    return channel;
}

void DestroyChannel(Channel* channel) {
    rb_free(channel->ring);
    channel_agent_remove(channel);
    kfree(channel->_buffer);
    kfree(channel);
}

Channel* channel_find_by_agent_id(AgentID id) {
    List* p = agent_channel_map;
    size_t i = 0;

    while (p != nullptr) {
        ChannelAgentData* data = list_get_value(p);
        
        if (data->agent->id == id) 
            return data->channel;

        p = p->next;
        i++;
    }

    return nullptr;
}

Channel* channel_find_by_agent_name(const char* name) {
    List* p = agent_channel_map;
    size_t i = 0;

    while (p != nullptr) {
        ChannelAgentData* data = list_get_value(p);
        
        if (strcmp(data->agent->name, name)) 
            return data->channel;

        p = p->next;
        i++;
    }

    return nullptr;
}

ChannelTransmitResult channel_transmit(Channel* self, Channel* dest, Package* msg) {
    if (!(self && channel_exists(self))) return CHANNEL_TRANSMIT_BAD_SENDER;
    if (!(self->flags & CHANNEL_CAN_SEND)) return CHANNEL_TRANSMIT_UNAUTHORIZED_SENDER;
    if (!(dest && channel_exists(dest))) return CHANNEL_TRANSMIT_BAD_RECIPIENT;
    if (!(dest->flags & CHANNEL_CAN_RECEIVE)) return CHANNEL_TRANSMIT_UNAUTHORIZED_RECIPIENT;
    if (!msg) return CHANNEL_TRANSMIT_BAD_PACKAGE;

    lock(&dest->ring_lock);
    if (rb_is_full(dest->ring)) return CHANNEL_TRANSMIT_BUFFER_FULL;
    rb_put(dest->ring, (uintptr_t)msg);
    unlock(&dest->ring_lock);

    return CHANNEL_TRANSMIT_SUCCESS;
}

ChannelReceiveResult channel_receive(Channel* self, Package** msg) {
    if (!(self && channel_exists(self))) return CHANNEL_RECEIVE_BAD_RECEIVER;
    if (!(self->flags & CHANNEL_CAN_RECEIVE)) return CHANNEL_RECEIVE_UNAUTHORIZED_RECEIVER;

    lock(&self->ring_lock);
    if (rb_is_empty(self->ring)) return CHANNEL_RECEIVE_BUFFER_EMPTY;
    rb_get(self->ring, (uintptr_t*)msg);
    unlock(&self->ring_lock);

    return CHANNEL_RECEIVE_SUCCESS;
}

ChannelPeekResult channel_peek(Channel* self, Package** msg) {
    if (!(self && channel_exists(self))) return CHANNEL_PEEK_BAD_RECEIVER;
    if (!(self->flags & CHANNEL_CAN_RECEIVE)) return CHANNEL_PEEK_UNAUTHORIZED_RECEIVER;

    lock(&self->ring_lock);
    if (rb_is_empty(self->ring)) return CHANNEL_PEEK_BUFFER_EMPTY;
    rb_peek(self->ring, (uintptr_t*)msg);
    unlock(&self->ring_lock);

    return CHANNEL_PEEK_SUCCESS;
}
