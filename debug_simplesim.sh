#!/bin/bash

# SimpleSim 디버깅 스크립트
# 사용법: ./debug_simplesim.sh [벤치마크 프로그램] [입력 파일]

# 변수 설정
USERNAME="dooik1005"
SIMPLESIM_DIR="/home/$USERNAME/simplesim-3.0"
SIMPLESIM_BIN="$SIMPLESIM_DIR/sim-outorder"
TEST_DIR="$SIMPLESIM_DIR/tests-alpha/bin"
GDB_PORT="2345"

# make 실행
cd $SIMPLESIM_DIR
make
if [ $? -ne 0 ]; then
    echo "make 실행 중 오류가 발생했습니다."
    exit 1
fi  
cd -

# 인자 확인
if [ $# -lt 1 ]; then
    echo "사용법: $0 [벤치마크 프로그램] [입력 파일]"
    exit 1
fi

INPUT_NAME="$1"
INPUT_FILE="$2"
BENCHMARK="$TEST_DIR/$INPUT_NAME"

# gdbserver 실행
echo "gdbserver 시작..."
echo "gdbserver가 포트 $GDB_PORT에서 실행 중입니다."
echo "VSCode에서 디버깅 세션을 시작하세요."
if [ -z "$INPUT_FILE" ]; then
    gdbserver :$GDB_PORT $SIMPLESIM_BIN -cache:il1 il1:128:64:1:l -cache:dl1 dl1:128:64:1:l -cache:il2 dl2 -cache:dl2 ul2:1024:64:2:l $BENCHMARK
else
    gdbserver :$GDB_PORT $SIMPLESIM_BIN -cache:il1 il1:128:64:1:l -cache:dl1 dl1:128:64:1:l -cache:il2 dl2 -cache:dl2 ul2:1024:64:2:l $BENCHMARK < $INPUT_FILE
fi