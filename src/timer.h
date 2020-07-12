#ifndef MINIOS_TIMER_H
#define MINIOS_TIMER_H

#include "common.h"
#include "isr.h"

typedef int time_t;
typedef int timer_t;

struct timespec
{
    time_t tv_sec;  /* Seconds */
    long   tv_nsec; /* Nanoseconds */
};

struct itimerspec
{
    struct timespec it_interval; /* Timer interval */
    struct timespec it_value;    /* Initial expiration */
};

typedef s32int clockid_t;

typedef void(*clock_callback_t)(registers_t regs);

#define CLOCK_MONOTONIC             0

#define TIMER_ABSTIME               1

#define __CLK_DEVICES_COUNT         1
#define __CLK_HANDLERS_COUNT        16
#define __CLK_MONOTONIC_DEFAULT_FRQ 82

#define __CLK_TICKING_HANDLER_ID    0
#define __CLK_TASKING_HANDLER_ID    1

extern const struct timespec __clk_infinite;

#define __CLK_INFINITE (&__clk_infinite)

/// Clock initialization; not a part of POSIX
int __clk_init(
    const clockid_t clockid,
    const u32int    frequency);

int __clk_setup_tick_handler(
    const clockid_t  clockid,
    clock_callback_t handler,
    u32int           id);

int clock_getres(
    clockid_t         clockid,
    struct timespec * resolution);

int clock_gettime(
    clockid_t         clockid,
    struct timespec * time);

int clock_settime(
    clockid_t               clockid,
    const struct timespec * time);

int clock_nanosleep(
    clockid_t               clockid,
    int                     flags,
    const struct timespec * request,
    struct timespec *       remain);

#endif