#pragma once

#include <inttypes.h>
#include <stdio.h>

void virtser_init(void);
void virtser_recv_task(void);
int virtser_putchar(char c, FILE *unused);
void virtser_recv(uint8_t c);

extern FILE *_sf;
