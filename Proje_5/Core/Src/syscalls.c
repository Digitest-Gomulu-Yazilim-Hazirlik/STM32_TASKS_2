/*
 * syscalls.c
 *
 *  Created on: Oct 22, 2025
 *      Author: Administrator
 */

#include "main.h"
#include <sys/stat.h>
#include <stdio.h>
#include <errno.h>

extern UART_HandleTypeDef huart2;


/* _write fonksiyonu printf çıktısını SWV veya UART üzerinden gönderir */
int _write(int file, char *ptr, int len)
{
    HAL_UART_Transmit(&huart2, (uint8_t*)ptr, len, HAL_MAX_DELAY);
    return len;
}

/* Diğer sistem çağrıları boş (stub) olarak bırakılıyor */
int _close(int file) { return -1; }
int _fstat(int file, struct stat *st) { st->st_mode = S_IFCHR; return 0; }
int _isatty(int file) { return 1; }
int _lseek(int file, int ptr, int dir) { return 0; }
int _read(int file, char *ptr, int len) { return 0; }
int _kill(int pid, int sig) { return -1; }
int _getpid(void) { return 1; }

