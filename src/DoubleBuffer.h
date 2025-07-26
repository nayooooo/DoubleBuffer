/**
 * @file DoubleBuffer.h
 */

#ifndef __DOUBLEBUFFER_H__
#define __DOUBLEBUFFER_H__

#include <stdint.h>

/*==================================================
    Tools Begin
==================================================*/

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
    Event Begin
==================================================*/

typedef uint32_t DB_Event;
#define DB_EVENT_NONE                   0
#define DB_EVENT_START_FILL             BIT(0)
#define DB_EVENT_STOP_FILL              BIT(1)
#define DB_EVENT_START_SEND             BIT(2)
#define DB_EVENT_STOP_SEND              BIT(3)
#define DB_EVENT_SWAP                   BIT(4)
#define DB_EVENT_MASK                   (BIT(5) - 1)

/*==================================================
    Event End
==================================================*/

/*==================================================
    Structer Begin
==================================================*/

/* ----- DoubleBuffer ----- */

struct DoubleBuffer
{
    /*---------- buf ----------*/

    uint8_t *buff[2];
    uint32_t size;
    volatile uint32_t counter[2];

    volatile uint8_t back_index;

    /*---------- fsm ----------*/

    volatile DB_State state;

    /*--------- event ---------*/

    volatile DB_Event event;

    /*-------- handle ---------*/

    uint32_t (*send)(uint8_t *buf, uint32_t size);
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

/*==================================================
    API End
==================================================*/

#endif  /* __DOUBLEBUFFER_H__ */
