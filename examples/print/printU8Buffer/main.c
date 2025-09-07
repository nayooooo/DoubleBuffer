/**
 * @file main.c
 * @author nayooooo
 */

#include <windows.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include "../../../src/DoubleBuffer.h"

#define BUFFER_SIZE (128)
#define RUN_ROUND   (100)
#define TEST_USE_PRINT       0

struct DoubleBuffer db;
uint8_t buf0[BUFFER_SIZE] = { 0 };
uint8_t buf1[BUFFER_SIZE] = { 0 };

typedef struct {
    uint8_t *buf;
    uint32_t size;
} ThreadParam;
ThreadParam send_thread_param = { nullptr, 0 };

HANDLE threads[2] = { NULL, NULL };
HANDLE send_semaphore = NULL;

void db_send_handler(uint8_t *buf, uint32_t size)
{
    send_thread_param.buf = buf;
    send_thread_param.size = size;
    ReleaseSemaphore(send_semaphore, 1, NULL);
}

DWORD WINAPI send_thread(LPVOID param)
{
    ThreadParam *para = (ThreadParam *)param;
    uint32_t counter = 0;
    uint8_t buf[BUFFER_SIZE] = { 0 };

    while (1) {
        counter++;

        DWORD ret = WaitForSingleObject(send_semaphore, INFINITE);
        if (ret == WAIT_OBJECT_0) {
#if TEST_USE_PRINT
            printf("send thread has received data\n");
#endif
        } else {
#if TEST_USE_PRINT
            printf("send thread wait data error: %d\n", GetLastError());
#endif
            continue;
        }

#if TEST_USE_PRINT
        printf("send[%u]:", counter);
#endif
        for (uint32_t i = 0; i < para->size; i++) {
#if TEST_USE_PRINT
            if (i % 8 == 0) {
                printf("\n");
            } else {
                printf("\t");
            }
            printf("%u", para->buf[i]);
#else
            buf[i] = para->buf[i];
#endif
        }
        printf("%ssend[%u] end\n\n", ((para->size - 1) % 8 == 0) ? "" : "\n", counter);

        db_send_complete(&db);

        if (counter == RUN_ROUND) {
            break;
        }
    }

    return 0;
}

DWORD WINAPI back_thread(LPVOID param)
{
    uint32_t counter = 0;
    uint8_t buf[BUFFER_SIZE] = { 0 };

    while (1) {
        counter++;

        for (uint32_t i = 0; i < BUFFER_SIZE; i++) {
            buf[i] = (uint8_t)(rand() & 0xFF);
        }

        db_send(&db, &buf[0], 0, BUFFER_SIZE, 0xFFFFFFFF);

        if (counter == RUN_ROUND) {
            break;
        }
    }

    return 0;
}

int main()
{
    LARGE_INTEGER frequency, start, end;
    double elapsed_time;
    QueryPerformanceFrequency(&frequency);

    printf("start!\n");

    srand(time(NULL));

    db_init(&db);
    db_set_buffer(&db, buf0, buf1, BUFFER_SIZE);
    db_set_handle(&db, db_send_handler, nullptr);

    send_semaphore = CreateSemaphore(NULL, 0, 1, NULL);
    if (send_semaphore == NULL) {
        printf("send semaphore create failed!\n");
        return -3;
    }
    printf("send semaphore is created!\n");

    threads[0] = CreateThread(NULL, 0, send_thread, &send_thread_param, 0, NULL);
    if (threads[0] == NULL) {
        printf("thread 0 create failed!\n");
        return -1;
    }
    printf("thread 0 is created!\n");

    threads[1] = CreateThread(NULL, 0, back_thread, NULL, 0, NULL);
    if (threads[1] == NULL) {
        printf("thread 1 create failed!\n");
        return -2;
    }
    printf("thread 1 is created!\n");

    QueryPerformanceCounter(&start);
    printf("waitting for thread work complete...\n");
    WaitForMultipleObjects(2, threads, TRUE, INFINITE);
    QueryPerformanceCounter(&end);
    elapsed_time = (end.QuadPart - start.QuadPart) * 1000.0 / frequency.QuadPart;

    CloseHandle(threads[0]);
    CloseHandle(threads[1]);
    CloseHandle(send_semaphore);

    printf("test ok!\n");

    printf("buf[%u] %u test, use %.3f ms\n", BUFFER_SIZE, RUN_ROUND, elapsed_time);
    printf("single buf use %.3f ms\n", elapsed_time / RUN_ROUND);

    system("pause");

    return 0;
}
