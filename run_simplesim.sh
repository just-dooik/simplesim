#!/bin/bash

# SimpleSim 시뮬레이터 실행 스크립트
# 사용법: ./run_simplesim.sh [벤치마크 프로그램] [입력 파일]

# 오류 처리 함수
error_exit() {
    echo "오류: $1" >&2
    exit 1
}
# 사용자 이름 설정
USERNAME="dooik1005" 

# SimpleSim 경로 설정
SIMPLESIM_DIR="/home/$USERNAME/simplesim-3.0"
SIMPLESIM_BIN="$SIMPLESIM_DIR/sim-outorder"
TEST_DIR="$SIMPLESIM_DIR/tests-alpha/bin"  # 미리 컴파일된 Alpha 바이너리가 있는 경로

# 실행 파일 존재 여부 확인
if [ ! -f "$SIMPLESIM_BIN" ]; then
    error_exit "SimpleSim 실행 파일($SIMPLESIM_BIN)을 찾을 수 없습니다."
fi

# 인자 확인
if [ $# -lt 1 ]; then
    echo "사용법: $0 [벤치마크 프로그램] [입력 파일]"
    echo "예시: $0 test-math"
    exit 1
fi

# 입력 파일 이름 처리
INPUT_NAME="$1"
INPUT_FILE="$2"

# 바이너리 경로 검색
BENCHMARK="$TEST_DIR/$INPUT_NAME"

# 바이너리가 없으면 다른 경로도 시도
if [ ! -f "$BENCHMARK" ]; then
    # 다른 가능한 테스트 디렉토리들
    POSSIBLE_DIRS=(
        "$SIMPLESIM_DIR/tests-alpha/bin.little"
        "$SIMPLESIM_DIR/tests/bin"
        "$SIMPLESIM_DIR/tests/bin.little"
        "$SIMPLESIM_DIR/bench/bin"
        "$SIMPLESIM_DIR/bench/bin.little"
        "$SIMPLESIM_DIR/tests-pisa/bin"
        "$SIMPLESIM_DIR/tests-pisa/bin.little"
    )
    
    # 각 디렉토리에서 바이너리 찾기
    for DIR in "${POSSIBLE_DIRS[@]}"; do
        if [ -f "$DIR/$INPUT_NAME" ]; then
            BENCHMARK="$DIR/$INPUT_NAME"
            echo "바이너리를 찾았습니다: $BENCHMARK"
            break
        fi
    done
fi

# 바이너리 존재 여부 확인
if [ ! -f "$BENCHMARK" ]; then
    error_exit "벤치마크 프로그램($BENCHMARK)을 찾을 수 없습니다. SimpleSim용으로 미리 컴파일된 바이너리가 필요합니다."
    echo "참고: SimpleSim용 크로스 컴파일러가 설치되어 있지 않습니다. 소스 코드를 컴파일하려면 sslittle-na-sstrix-gcc 또는 ssalpha-na-sstrix-gcc 크로스 컴파일러가 필요합니다."
fi

# SimpleSim 실행
echo "SimpleSim 시뮬레이션 시작..."

if [ -z "$INPUT_FILE" ]; then
    # 입력 파일이 없는 경우
    "$SIMPLESIM_BIN" -cache:il1 il1:128:64:1:l -cache:dl1 dl1:128:64:1:l -cache:il2 dl2 -cache:dl2 ul2:1024:64:2:l "$BENCHMARK"
else
    # 입력 파일이 있는 경우
    "$SIMPLESIM_BIN" -cache:il1 il1:128:64:1:l -cache:dl1 dl1:128:64:1:l -cache:il2 dl2 -cache:dl2 ul2:1024:64:2:l "$BENCHMARK" < "$INPUT_FILE"
fi

# 실행 결과 확인
if [ $? -eq 0 ]; then
    echo "시뮬레이션이 성공적으로 완료되었습니다."
else
    error_exit "시뮬레이션 실행 중 오류가 발생했습니다."
fi
