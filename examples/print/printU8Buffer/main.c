/**
 * @file main.c
 * @author nayooooo
 */

#include <windows.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include "../../../src/DoubleBuffer.h"

#define BUFFER_SIZE (4096)
#define RUN_ROUND   (100000)
#define TEST_USE_PRINT       0
#define TEST_USE_MONITOR     1

struct DoubleBuffer db;
uint8_t buf0[BUFFER_SIZE] = { 0 };
uint8_t buf1[BUFFER_SIZE] = { 0 };

struct {
    uint8_t send_flag : 1;
    uint8_t back_flag : 1;
    uint8_t turn;
} peterson_lock = {
    .send_flag = 0,
    .back_flag = 0,
    .turn = 0
};

typedef struct {
    uint8_t *buf;
    uint32_t size;
} ThreadParam;
ThreadParam send_thread_param = { nullptr, 0 };

HANDLE threads[2] = { NULL, NULL };
#if TEST_USE_MONITOR
HANDLE monitor_thread = NULL;
#endif
HANDLE send_semaphore = NULL;

uint32_t send_counter = 0;
uint32_t back_counter = 0;

void db_send_handler(uint8_t *buf, uint32_t size)
{
    send_thread_param.buf = buf;
    send_thread_param.size = size;
    ReleaseSemaphore(send_semaphore, 1, NULL);
}

#if TEST_USE_MONITOR
DWORD WINAPI monitor_thread_entry(LPVOID param)
{
    float need_time = 1E30f;
    uint32_t send_counter_backup = 0;
    uint32_t back_counter_backup = 0;
    DWORD sleep_time = 500;

    do {
        char str[200] = "";
        time_t current_time = time(NULL);
        struct tm *local_time = localtime(&current_time);
        strftime(&str[0], sizeof(str), "The calculation starts from %Y-%m-%d %H:%M:%S\n", local_time);
        printf("%s", str);
    } while (0);

    // wait
    Sleep(10);

    while (1) {
        char str[200] = "";
        size_t len = 0;

        // clear
        printf("\r");
        fflush(stdout);

        // time
        time_t current_time = time(NULL);
        struct tm *local_time = localtime(&current_time);
        len += strftime(&str[len], sizeof(str) - len, "[%Y-%m-%d %H:%M:%S]", local_time);

        // info
        len += sprintf(&str[len], "[send %u/%u][back %u/%u]", send_counter, RUN_ROUND, back_counter, RUN_ROUND);

        uint32_t send_complete_round_last = send_counter - send_counter_backup;
        uint32_t back_complete_round_last = back_counter - back_counter_backup;

        // speed
        float speed = 1.0 * send_complete_round_last * BUFFER_SIZE / sleep_time;  // B/ms
        speed *= 1000;  // B/s
        speed /= 1024;  // kB/s
        speed /= 1024;  // MB/s
        len += sprintf(&str[len], "[speed %.03f MB/s]", speed);

        // need time
        float send_need_time = 1.0 * (RUN_ROUND - send_counter) / send_complete_round_last;
        float back_need_time = 1.0 * (RUN_ROUND - back_counter) / back_complete_round_last;
        need_time = MAX(send_need_time, back_need_time);
        need_time *= sleep_time;
        len += sprintf(&str[len], " need");
        int day = (int)need_time / (1000 * 60 * 60 * 24); need_time -= day * (1000 * 60 * 60 * 24);
        int hour = (int)need_time / (1000 * 60 * 60); need_time -= hour * (1000 * 60 * 60);
        int minute = (int)need_time / (1000 * 60); need_time -= minute * (1000 * 60);
        int seconds = (int)(need_time / 1000);
        if (day > 0) {
            len += sprintf(&str[len], " %d day", day);
        }
        len += sprintf(&str[len], " %02d:%02d:%02d", hour, minute, seconds);

        printf("%s", str);
        fflush(stdout);

        send_counter_backup = send_counter;
        back_counter_backup = back_counter;

        if (send_counter >= RUN_ROUND && back_counter >= RUN_ROUND) {
            break;
        }

        Sleep(sleep_time);
    }

    return 0;
}
#endif

DWORD WINAPI send_thread(LPVOID param)
{
    ThreadParam *para = (ThreadParam *)param;
    uint8_t buf[BUFFER_SIZE] = { 0 };

    while (send_counter < RUN_ROUND) {
        DWORD ret = WaitForSingleObject(send_semaphore, INFINITE);
        if (ret == WAIT_OBJECT_0) {
#if TEST_USE_PRINT
            printf("send thread has received data\n");
#endif
        } else {
#if TEST_USE_PRINT
            printf("send thread wait data error: %d\n", GetLastError());
#endif
            goto end;
        }
        uint8_t *pBuf = para->buf;
        uint32_t size = para->size;

#if TEST_USE_PRINT
        printf("send[%u]:", send_counter);
#endif
        for (uint32_t i = 0; i < size; i++) {
#if TEST_USE_PRINT
            if (i % 8 == 0) {
                printf("\n");
            } else {
                printf("\t");
            }
            printf("%u", pBuf[i]);
#else
            buf[i] = pBuf[i];
#endif
        }
#if TEST_USE_PRINT
        printf("%ssend[%u] end\n\n", ((size - 1) % 8 == 0) ? "" : "\n", send_counter);
#endif

    end:
        peterson_lock.send_flag = 1;
        peterson_lock.turn = 1;
        while (peterson_lock.back_flag == 1 && peterson_lock.turn == 1) Sleep(0);

        db_send_complete(&db);

        peterson_lock.send_flag = 0;

        send_counter++;
    }

    return 0;
}

DWORD WINAPI back_thread(LPVOID param)
{
    uint8_t buf[BUFFER_SIZE] = { 0 };
    uint32_t delay_times = 0;

    while (back_counter < RUN_ROUND) {
        for (uint32_t i = 0; i < BUFFER_SIZE; i++) {
            buf[i] = (uint8_t)(rand() & 0xFF);
        }

        int ret;
        do {
            peterson_lock.back_flag = 1;
            peterson_lock.turn = 0;
            while (peterson_lock.send_flag == 1 && peterson_lock.turn == 0) Sleep(0);
            
            ret = db_send(&db, &buf[0], 0, BUFFER_SIZE, 0xF);

            peterson_lock.back_flag = 0;

            if (ret != BUFFER_SIZE) {
                delay_times++;
#if TEST_USE_PRINT
                printf("round[%u] error code: %d\n", back_counter, ret);
#endif
            }
        } while (ret != BUFFER_SIZE);

        back_counter++;
    }

    printf("\ntotal delay times: %u\n", delay_times);

    return 0;
}

int main()
{
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    printf("system cpu core num: %d\n", sysInfo.dwNumberOfProcessors);

    LARGE_INTEGER frequency, start, end;
    double elapsed_time;
    QueryPerformanceFrequency(&frequency);

    printf("buf[%u] %u test start!\n", BUFFER_SIZE, RUN_ROUND);

    srand(time(NULL));

    db_init(&db);
    db_set_buffer(&db, buf0, buf1, BUFFER_SIZE);
    db_set_handle(&db, db_send_handler, nullptr);

    send_semaphore = CreateSemaphore(NULL, 0, 1, NULL);
    if (send_semaphore == NULL) {
        printf("send semaphore create failed!\n");
        return -1;
    }
    printf("send semaphore is created!\n");

#if TEST_USE_MONITOR
    monitor_thread = CreateThread(NULL, 0, monitor_thread_entry, NULL, 0, NULL);
    if (monitor_thread == NULL) {
        printf("monitor thread create failed!\n");
        return -2;
    }
    printf("monitor thread created!\n");
    if (SetThreadAffinityMask(monitor_thread, BIT(6)) != 0) {
        printf("monitor thread put into core 6 success!\n");
    } else {
        printf("monitor thread put into core 6 failed!\n");
        return -3;
    }
#endif

    threads[0] = CreateThread(NULL, 0, send_thread, &send_thread_param, 0, NULL);
    if (threads[0] == NULL) {
        printf("thread 0 create failed!\n");
        return -4;
    }
    printf("thread 0 is created!\n");
    if (SetThreadAffinityMask(threads[0], BIT(0)) != 0) {
        printf("thread 0 put into core 0 success!\n");
    } else {
        printf("thread 0 put into core 0 failed!\n");
        return -5;
    }

    threads[1] = CreateThread(NULL, 0, back_thread, NULL, 0, NULL);

    QueryPerformanceCounter(&start);

    if (threads[1] == NULL) {
        printf("thread 1 create failed!\n");
        return -6;
    }
    printf("thread 1 is created!\n");
    if (SetThreadAffinityMask(threads[1], BIT(0)) != 0) {
        printf("thread 1 put into core 0 success!\n");
    } else {
        printf("thread 1 put into core 0 failed!\n");
        return -7;
    }

    printf("waitting for thread work complete...\n");
    WaitForMultipleObjects(2, &threads[0], TRUE, INFINITE);
    
    QueryPerformanceCounter(&end);
    elapsed_time = (end.QuadPart - start.QuadPart) * 1000.0 / frequency.QuadPart;

    printf("thread o and 1 complete!\n");

#if TEST_USE_MONITOR
    WaitForMultipleObjects(1, &monitor_thread, TRUE, INFINITE);
    printf("\nmonitor thread complete!\n");
#endif

    CloseHandle(threads[0]);
    CloseHandle(threads[1]);
#if TEST_USE_MONITOR
    CloseHandle(monitor_thread);
#endif
    CloseHandle(send_semaphore);

    printf("test ok!\n");

    printf("buf[%u] %u test, use %.3f ms\n", BUFFER_SIZE, RUN_ROUND, elapsed_time);
    printf("single buf use %.3f ms\n", elapsed_time / RUN_ROUND);

    system("pause");

    return 0;
}
