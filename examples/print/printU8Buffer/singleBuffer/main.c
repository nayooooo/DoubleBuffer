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
#define RUN_ROUND   (100)
#define TEST_USE_PRINT       0

uint8_t buf[BUFFER_SIZE] = { 0 };

int main()
{
    LARGE_INTEGER frequency, start, end;
    double elapsed_time;
    uint32_t counter = 0;
    uint8_t _buf[BUFFER_SIZE] = { 0 };
    QueryPerformanceFrequency(&frequency);

    printf("start!\n");

    srand(time(NULL));

    QueryPerformanceCounter(&start);
    while (1) {
        counter++;

        for (uint32_t i = 0; i < BUFFER_SIZE; i++) {
            buf[i] = (uint8_t)(rand() & 0xFF);
        }

#if TEST_USE_PRINT
        printf("send[%u]:", counter);
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
        printf("%ssend[%u] end\n\n", ((BUFFER_SIZE - 1) % 8 == 0) ? "" : "\n", counter);

        if (counter == RUN_ROUND) {
            break;
        }
    }
    QueryPerformanceCounter(&end);
    elapsed_time = (end.QuadPart - start.QuadPart) * 1000.0 / frequency.QuadPart;

    printf("test ok!\n");

    printf("buf[%u] %u test, use %.3f ms\n", BUFFER_SIZE, RUN_ROUND, elapsed_time);
    printf("single buf use %.3f ms\n", elapsed_time / RUN_ROUND);

    system("pause");

    return 0;
}
