// timer.c -- Initialises the PIT, and handles clock updates.
//            Written for JamesM's kernel development tutorials.

#include "timer.h"
#include "isr.h"
#include "monitor.h"
#include "u64.h"
#include "task.h"

#define CLOCK_DEVICES_COUNT __CLK_DEVICES_COUNT
#define CLOCK_HANDLERS_COUNT __CLK_HANDLERS_COUNT

#define MONOTONIC_DEVICE_FREQUENCY 1193182
#define NANOSEC_IN_SEC 1000000000

#define CHECK_CLOCKID(id)            \
  ({                                 \
    if ((id) >= CLOCK_DEVICES_COUNT) \
      return EXIT_FAILURE;           \
  })

typedef struct
{
  u64int resolution;
  u64int ticks;
} clock_device_t;

static volatile u32int sleep_count = 0;
const struct timespec __clk_infinite = {.tv_sec = 2147483647, .tv_nsec = 0};

static void __clk_monotonic_increase_tick_count(registers_t regs);

static clock_device_t __clk_devices[CLOCK_DEVICES_COUNT];
static clock_callback_t __clk_handlers[CLOCK_DEVICES_COUNT][CLOCK_HANDLERS_COUNT] =
{
        [CLOCK_MONOTONIC] =
            {
                __clk_monotonic_increase_tick_count,
            },
};

static inline int a_then_b(const struct timespec *a, const struct timespec *b)
{
  if (a->tv_sec == b->tv_sec)
  {
    return a->tv_nsec < b->tv_nsec;
  }
  return a->tv_sec < b->tv_sec;
}

static inline void a_add_b(const struct timespec *a, const struct timespec *b, struct timespec *res)
{
  res->tv_sec = a->tv_sec + b->tv_sec;
  res->tv_nsec = a->tv_nsec + b->tv_nsec;

  u64int overflow = __udivdi3(res->tv_nsec, NANOSEC_IN_SEC);
  if (overflow != 0)
  {
    res->tv_sec += overflow;
    res->tv_nsec -= overflow * NANOSEC_IN_SEC;
  }
}

static inline unsigned long long rdtsc(void)
{
  unsigned long long int x;
  asm volatile(".byte 0x0f, 0x31"
               : "=A"(x));
  return x;
}

static void __clk_monotonic_increase_tick_count(registers_t regs)
{
  __clk_devices[CLOCK_MONOTONIC].ticks += 1;
}

static void __clk_monotonic_routine(registers_t regs)
{
  for (u32int id = 0; id < CLOCK_HANDLERS_COUNT; ++id)
  {
    clock_callback_t callback = __clk_handlers[CLOCK_MONOTONIC][id];
    if (callback != NULL)
    {
      callback(regs);
    }
  }
}

static int __clk_monotonic_init(const u32int frequency)
{
  // We are modifying kernel structures, and so cannot be interrupted.
  asm volatile("cli");

  // We can initialize clock once via API
  ASSERT(__clk_devices[CLOCK_MONOTONIC].resolution == 0);

  // Checkout frequency
  if (MONOTONIC_DEVICE_FREQUENCY % frequency != 0)
    return 1;

  // The value we send to the PIT is the value to divide it's input clock
  // (1193182 Hz) by, to get our required frequency. Important to note is
  // that the divisor must be small enough to fit into 16-bits.
  u32int divisor = MONOTONIC_DEVICE_FREQUENCY / frequency;

  // Resolution in nanoseconds
  __clk_devices[CLOCK_MONOTONIC].resolution = NANOSEC_IN_SEC / frequency;

  // Register our timer callback.
  register_interrupt_handler(IRQ0, &__clk_monotonic_routine);

  // Send the command byte.
  outb(0x43, 0x36);

  // Divisor has to be sent byte-wise, so split here into upper/lower bytes.
  u8int l = (u8int)(divisor & 0xFF);
  u8int h = (u8int)((divisor >> 8) & 0xFF);

  // Send the frequency divisor.
  outb(0x40, l);
  outb(0x40, h);

  // Clock initialized, ebable unterrupts back
  asm volatile("sti");

  return EXIT_SUCCESS;
}

int __clk_init(const clockid_t clockid, const u32int frequency)
{
  switch (clockid)
  {
  case CLOCK_MONOTONIC:
    return __clk_monotonic_init(frequency);
  }
  return 1;
}

int __clk_setup_tick_handler(
    const clockid_t clockid,
    clock_callback_t handler,
    u32int id)
{
  // Checkout input parameters
  CHECK_CLOCKID(clockid);
  if (id < CLOCK_DEVICES_COUNT || id >= CLOCK_HANDLERS_COUNT)
    return EXIT_FAILURE;

  // Update handler
  __clk_handlers[CLOCK_MONOTONIC][id] = handler;

  return EXIT_SUCCESS;
}

int clock_getres(
    clockid_t clockid,
    struct timespec *resolution)
{
  CHECK_CLOCKID(clockid);

  u64int device_resolution = __clk_devices[CLOCK_MONOTONIC].resolution;
  resolution->tv_sec = 0;
  resolution->tv_nsec = device_resolution;

  return EXIT_SUCCESS;
}

int clock_gettime(
    clockid_t clockid,
    struct timespec *time)
{
  CHECK_CLOCKID(clockid);

  u64int ticks = __clk_devices[CLOCK_MONOTONIC].ticks;
  u64int device_resolution = __clk_devices[CLOCK_MONOTONIC].resolution;
  u64int current = ticks * device_resolution;
  time->tv_sec = __udivdi3(current, NANOSEC_IN_SEC);
  time->tv_nsec = current - time->tv_sec * NANOSEC_IN_SEC;

  return EXIT_SUCCESS;
}

int clock_settime(
    clockid_t clockid,
    const struct timespec *time)
{
  return EXIT_FAILURE;
}

int clock_nanosleep(
    clockid_t clockid,
    int flags,
    const struct timespec *request,
    struct timespec *remain)
{
  CHECK_CLOCKID(clockid);

  int exit_status = EXIT_SUCCESS;
  struct timespec current;
  struct timespec wakeline = *request;

  if (flags != TIMER_ABSTIME)
  {
    clock_gettime(CLOCK_MONOTONIC, &current);
    a_add_b(&current, &wakeline, &wakeline);
  }

  ++sleep_count;
  // Recieve current time. If it does not overrun request time then halt until next interrupt.
  while (exit_status = clock_gettime(CLOCK_MONOTONIC, &current),
         !FAILED(exit_status) && a_then_b(&current, &wakeline))
  {
    if (proc_count() - sleep_count <= 1)
    {
      asm volatile("hlt");
    }
    else
    {
      switch_task();
    }
  }
  --sleep_count;

  if (FAILED(exit_status))
    return EXIT_FAILURE;
  return EXIT_SUCCESS;
}