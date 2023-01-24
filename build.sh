#!/bin/bash

SRC_DIR=sources
INC_DIR=headers
BIN_DIR=bin

CC=x86_64-w64-mingw32-gcc

[ -d ${BIN_DIR} ] || mkdir ${BIN_DIR}

for file in ${SRC_DIR}/*.c; do
    ${CC} -o ${BIN_DIR}/$(basename ${file%.c}).exe -I ${INC_DIR} ${file} -lgdi32
done
