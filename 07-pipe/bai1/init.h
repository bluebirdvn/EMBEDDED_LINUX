#ifndef _INIT_H
#define _INIT_H
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/wait.h>
#include <signal.h>

#define handle_error(msg) do {perror(msg); exit(EXIT_FAILURE);} while (0)
#define BUFFERSIZE 256

#endif 