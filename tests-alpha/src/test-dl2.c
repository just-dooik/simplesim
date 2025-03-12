#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define ARRAY_SIZE (1024 * 1024) // 1MB 배열 크기 (L1 캐시 크기를 초과)
#define STRIDE     64           // 캐시 라인 크기 (64바이트)

int main() {
    // 큰 배열 선언
    int *array = (int *)malloc(ARRAY_SIZE * sizeof(int));
    if (array == NULL) {
        fprintf(stderr, "메모리 할당 실패\n");
        return 1;
    }

    // 배열 초기화
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array[i] = i;
    }

    // DL2에 접근하기 위한 메모리 액세스
    volatile int sum = 0; // 최적화 방지
    for (int i = 0; i < ARRAY_SIZE; i += STRIDE) {
        sum += array[i]; // 큰 stride로 메모리 접근
    }

    printf("합계: %d\n", sum);

    // 메모리 해제
    free(array);
    return 0;
}
