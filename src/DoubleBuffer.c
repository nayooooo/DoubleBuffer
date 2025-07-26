/**
 * @file DoubleBuffer.c
 */

#include "DoubleBuffer.h"

#define db_set_event(db, evt) db->event |=  evt
#define db_clr_event(db, evt) db->event &= ~evt

int db_init(struct DoubleBuffer * const db)
{
    if (db == nullptr) {
        return -1;
    }

    db->state = DB_STATE_UNINIT;

    db->buff[0] = nullptr;
    db->buff[1] = nullptr;
    db->size = 0;
    db->counter[0] = 0;
    db->counter[1] = 0;
    db->back_index = 0;

    db->state |= DB_STATE_NO_BUFFER;

    db->event = DB_EVENT_NONE;

    return 0;
}

int db_set_buffer(
    struct DoubleBuffer * const db,
    uint8_t * const buf0, uint8_t * const buf1,
    const uint32_t size
)
{
    if (db == nullptr) {
        return -1;
    }
    if (buf0 == nullptr || buf1 == nullptr) {
        return -2;
    }
    if (size <= 0) {
        return -3;
    }

    db->buff[0] = buf0;
    db->buff[1] = buf1;
    db->size = size;

    db->state &= ~DB_STATE_NO_BUFFER;
    db->state |= DB_STATE_IDLE;

    return 0;
}

int db_set_send_handle(
    struct DoubleBuffer * const db,
    uint32_t (*send)(uint8_t *buf, uint32_t size)
)
{
    if (db == nullptr) {
        return -1;
    }

    db->send = send;

    return 0;
}
