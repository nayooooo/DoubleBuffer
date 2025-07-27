/**
 * @file DoubleBuffer.h
 */

#ifndef __DOUBLEBUFFER_H__
#define __DOUBLEBUFFER_H__

#include <stdint.h>

/*==================================================
    Tools Begin
==================================================*/

/* ----- Unused ----- */

#ifndef UNUSED
#   define UNUSED(X) ((void)X)
#endif  // !UNUSED

/* ----- Compare ----- */

#ifndef MAX
#   define MAX(a, b) (((a) > (b)) ? (a) : (b))
#endif  // !MAX
#ifndef MIN
#   define MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif  // !MIN

/* ----- Bit ----- */

#ifndef BIT
#   define BIT(n) (1UL << (n))
#endif  // !BIT

/* ----- Pointer ----- */

#ifndef nullptr
#   define nullptr ((void *)0)
#endif  // !nullptr

/* ----- Structer ----- */

#ifndef container_of
#   define container_of(ptr, type, member) \
        ((type *)((char *)(ptr) - (unsigned long)(&((type *)0)->member)))
#endif  // !container_of

/*==================================================
    Tools End
==================================================*/

/*==================================================
    Fsm Begin
==================================================*/

typedef uint32_t DB_State;
#define DB_STATE_UNINIT                 0
#define DB_STATE_NO_BUFFER              BIT(0)
#define DB_STATE_IDLE                   BIT(1)
#define DB_STATE_FILLING                BIT(2)
#define DB_STATE_FILLED                 BIT(3)
#define DB_STATE_SENDING                BIT(4)
#define DB_STATE_SENT                   BIT(5)
#define DB_STATE_READY_TO_SWAP          BIT(6)
#define DB_STATE_SWAPPING               BIT(7)
#define DB_STATE_MASK                   (BIT(8) - 1)

/*==================================================
    Fsm End
==================================================*/

/*==================================================
    Structer Begin
==================================================*/

/* ----- DoubleBuffer ----- */

typedef void (*DB_Buff_Event_Callback)(uint32_t size, void *user_data);
typedef void (*DB_Swap_Event_Callback)(uint8_t nb_ind, void *user_data);

struct DoubleBuffer
{
    /*---------- buf ----------*/

    uint8_t *buff[2];
    uint32_t size;
    volatile uint32_t counter[2];

    volatile uint8_t back_index;

    /*---------- fsm ----------*/

    volatile DB_State state;

    /*-------- handle ---------*/

    uint32_t (*send)(uint8_t *buf, uint32_t size);

    /*---- event callback -----*/

    DB_Buff_Event_Callback fill_start;
    DB_Buff_Event_Callback fill_complete;
    DB_Swap_Event_Callback swap_start;
    DB_Swap_Event_Callback swap_complete;
    DB_Buff_Event_Callback send_start;
    DB_Buff_Event_Callback send_complete;
    DB_Buff_Event_Callback recv_start;
    DB_Buff_Event_Callback recv_complete;

    /*------- user data -------*/

    void *user_data;
};

/*==================================================
    Structer End
==================================================*/

/*==================================================
    API Begin
==================================================*/

int db_init(struct DoubleBuffer * const db);
int db_set_buffer(
    struct DoubleBuffer * const db,
    uint8_t * const buf0, uint8_t * const buf1,
    const uint32_t size
);
int db_set_send_handle(
    struct DoubleBuffer * const db,
    uint32_t (*send)(uint8_t *buf, uint32_t size)
);
int db_set_fill_start(struct DoubleBuffer * const db, DB_Buff_Event_Callback fill_start);
int db_set_fill_complete(struct DoubleBuffer * const db, DB_Buff_Event_Callback fill_complete);
int db_set_swap_start(struct DoubleBuffer * const db, DB_Swap_Event_Callback swap_start);
int db_set_swap_complete(struct DoubleBuffer * const db, DB_Swap_Event_Callback swap_complete);
int db_set_send_start(struct DoubleBuffer * const db, DB_Buff_Event_Callback send_start);
int db_set_send_complete(struct DoubleBuffer * const db, DB_Buff_Event_Callback send_complete);
int db_set_recv_start(struct DoubleBuffer * const db, DB_Buff_Event_Callback recv_start);
int db_set_recv_complete(struct DoubleBuffer * const db, DB_Buff_Event_Callback recv_complete);

int db_set_user_data(struct DoubleBuffer * const db, void *user_data);

int db_send(
    struct DoubleBuffer * const db,
    const uint8_t * const buff,
    uint32_t offset, uint32_t size
);
int db_recv(
    struct DoubleBuffer * const db,
    const uint8_t * const buff,
    uint32_t offset, uint32_t size
);

/*==================================================
    API End
==================================================*/

#endif  /* __DOUBLEBUFFER_H__ */
