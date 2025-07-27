/**
 * @file DoubleBuffer.c
 */

#include "DoubleBuffer.h"

#include <string.h>
#define DB_Memcpy memcpy

#define db_set_state(db, sta) (db->state |=  sta)
#define db_clr_state(db, sta) (db->state &= ~sta)
#define db_chk_state(db, sta) (db->state &   sta)

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

    db_set_state(db, DB_STATE_NO_BUFFER);

    db->send = nullptr;

    db->fill_start    = nullptr;
    db->fill_complete = nullptr;
    db->swap_start    = nullptr;
    db->swap_complete = nullptr;
    db->send_start    = nullptr;
    db->send_complete = nullptr;
    db->recv_start    = nullptr;
    db->recv_complete = nullptr;

    db->user_data = nullptr;

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

    db_set_state(db, DB_STATE_IDLE);
    db_clr_state(db, DB_STATE_NO_BUFFER);

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

int db_set_fill_start(struct DoubleBuffer * const db, DB_Buff_Event_Callback fill_start)
{
    if (db == nullptr) {
        return -1;
    }

    db->fill_start = fill_start;

    return 0;
}

int db_set_fill_complete(struct DoubleBuffer * const db, DB_Buff_Event_Callback fill_complete)
{
    if (db == nullptr) {
        return -1;
    }

    db->fill_complete = fill_complete;

    return 0;
}

int db_set_swap_start(struct DoubleBuffer * const db, DB_Swap_Event_Callback swap_start)
{
    if (db == nullptr) {
        return -1;
    }

    db->swap_start = swap_start;

    return 0;
}

int db_set_swap_complete(struct DoubleBuffer * const db, DB_Swap_Event_Callback swap_complete)
{
    if (db == nullptr) {
        return -1;
    }

    db->swap_complete = swap_complete;

    return 0;
}

int db_set_send_start(struct DoubleBuffer * const db, DB_Buff_Event_Callback send_start)
{
    if (db == nullptr) {
        return -1;
    }

    db->send_start = send_start;

    return 0;
}

int db_set_send_complete(struct DoubleBuffer * const db, DB_Buff_Event_Callback send_complete)
{
    if (db == nullptr) {
        return -1;
    }

    db->send_complete = send_complete;

    return 0;
}

int db_set_recv_start(struct DoubleBuffer * const db, DB_Buff_Event_Callback recv_start)
{
    if (db == nullptr) {
        return -1;
    }

    db->recv_start = recv_start;

    return 0;
}

int db_set_recv_complete(struct DoubleBuffer * const db, DB_Buff_Event_Callback recv_complete)
{
    if (db == nullptr) {
        return -1;
    }

    db->recv_complete = recv_complete;

    return 0;
}

int db_set_user_data(struct DoubleBuffer * const db, void *user_data)
{
    if (db == nullptr) {
        return -1;
    }

    db->user_data = user_data;

    return 0;
}

int db_send(
    struct DoubleBuffer * const db,
    const uint8_t * const buff,
    uint32_t offset, uint32_t size
)
{
    uint32_t size_to_send = 0;

    if (db == nullptr) {
        return -1;
    }
    if (buff == nullptr) {
        return -2;
    }
    if (size <= 0) {
        return -3;
    }

    if (db->back_index >= 2) {
        db->state = DB_STATE_UNINIT;
        return -4;
    }

    if (db_chk_state(db, DB_STATE_NO_BUFFER)) {
        return -5;
    }
    if (!db_chk_state(db, DB_STATE_IDLE)) {
        return -6;
    }

    size_to_send = MIN(db->size, size);
    db->counter[db->back_index] = 0;
    db_clr_state(db, DB_STATE_IDLE);
    db_set_state(db, DB_STATE_FILLING);
    if (db->fill_start != nullptr) {
        db->fill_start(size, db->user_data);
    }

    DB_Memcpy((void *)&db->buff[db->back_index][0],
              (void *)(buff + offset),
              size_to_send);
    db->counter[db->back_index] = size_to_send;

    db_set_state(db, DB_STATE_FILLED);
    db_clr_state(db, DB_STATE_FILLING);
    if (db->fill_complete != nullptr) {
        db->fill_complete(db->counter[db->back_index], db->user_data);
    }

    if (!db_chk_state(db, DB_STATE_FILLING)
        && db_chk_state(db, DB_STATE_SENDING)) {
        db_set_state(db, DB_STATE_READY_TO_SWAP);
        db_clr_state(db, DB_STATE_FILLED);
        db_clr_state(db, DB_STATE_SENT);
    }

    return (int)db->counter[db->back_index];
}

int db_recv(
    struct DoubleBuffer * const db,
    const uint8_t * const buff,
    uint32_t offset, uint32_t size
)
{
    UNUSED(db);
    UNUSED(buff);
    UNUSED(offset);
    UNUSED(size);

    return 0;
}
