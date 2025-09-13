/**
 * @file main.c
 * @author nayooooo
 */

#include <windows.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define BUFFER_SIZE (128)
#define RUN_ROUND   (100000)
#define TEST_USE_PRINT       0

uint8_t buf[BUFFER_SIZE] = { 0 };

int main()
{
    LARGE_INTEGER frequency, start, end;
    double elapsed_time;
    uint32_t counter = 0;
    uint8_t _buf[BUFFER_SIZE] = { 0 };
    QueryPerformanceFrequency(&frequency);

    printf("buf[%u] %u test start!\n", BUFFER_SIZE, RUN_ROUND);

    srand(time(NULL));

    LARGE_INTEGER last_time;
    QueryPerformanceCounter(&last_time);
    uint32_t counter_backup = counter;
    QueryPerformanceCounter(&start);
    while (counter < RUN_ROUND) {

        for (uint32_t i = 0; i < BUFFER_SIZE; i++) {
            buf[i] = (uint8_t)(rand() & 0xFF);
        }

#if TEST_USE_PRINT
        printf("send[%u]:", RUN_ROUND - counter);
#endif
        for (uint32_t i = 0; i < BUFFER_SIZE; i++) {
#if TEST_USE_PRINT
            if (i % 8 == 0) {
                printf("\n");
            } else {
                printf("\t");
            }
            printf("%u", buf[i]);
#else
            _buf[i] = buf[i];
#endif
        }
#if TEST_USE_PRINT
        printf("%ssend[%u] end\n\n", ((BUFFER_SIZE - 1) % 8 == 0) ? "" : "\n", RUN_ROUND - counter);
#endif

        uint32_t sleep_time = 500;
        {
            LARGE_INTEGER this_time;
            QueryPerformanceCounter(&this_time);

            if ((this_time.QuadPart - last_time.QuadPart) * 1000 / frequency.QuadPart > sleep_time) {
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
                len += sprintf(&str[len], "[%u/%u]", counter, RUN_ROUND);

                uint32_t complete_round_last = counter - counter_backup;

                // speed
                float speed = 1.0 * complete_round_last * BUFFER_SIZE / sleep_time;  // B/ms
                speed *= 1000;  // B/s
                speed /= 1024;  // kB/s
                speed /= 1024;  // MB/s
                len += sprintf(&str[len], "[speed %.03f MB/s]", speed);

                // need time
                float need_time = 1.0 * (RUN_ROUND - counter) / complete_round_last;
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

                last_time = this_time;
                counter_backup = counter;
            }
        }

        counter++;
    }
    QueryPerformanceCounter(&end);
    elapsed_time = (end.QuadPart - start.QuadPart) * 1000.0 / frequency.QuadPart;

    printf("\ntest ok!\n");

    printf("buf[%u] %u test, use %.3f ms\n", BUFFER_SIZE, RUN_ROUND, elapsed_time);
    printf("single buf use %.3f ms\n", elapsed_time / RUN_ROUND);

    system("pause");

    return 0;
}
