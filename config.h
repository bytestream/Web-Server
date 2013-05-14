#ifndef CONFIG_H_
#define CONFIG_H_

#define _GNU_SOURCE

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>

#include "misc.h"

/*
 * Set printf to you my_printf with custom debugging
 */
#define printf my_printf

// User config
#define HTTP_PORT 3337
#define MAX_CONN 512

// Do not change
#define MAX_LEN 1024

#endif