// main.c -- Defines the C-code kernel entry point, calls initialisation routines.
//           Made for JamesM's tutorials <www.jamesmolloy.co.uk>

#include "monitor.h"
#include "descriptor_tables.h"
#include "timer.h"
#include "paging.h"
#include "multiboot.h"
//#include "fs.h"
//#include "initrd.h"
#include "task.h"

extern u32int end;
extern u32int placement_address;
u32int initial_esp;

static const struct timespec one_s   = {.tv_sec = 1, .tv_nsec = 0};
static const struct timespec two_s   = {.tv_sec = 2, .tv_nsec = 0};
static const struct timespec three_s = {.tv_sec = 3, .tv_nsec = 0};
static const struct timespec four_s  = {.tv_sec = 4, .tv_nsec = 0};
static const struct timespec five_s  = {.tv_sec = 5, .tv_nsec = 0};

void task_1(void)
{
  monitor_write("[*] task 1 started\n");
  clock_nanosleep(CLOCK_MONOTONIC, 0, &five_s, NULL);
  monitor_write("[!] task 1 done job\n");
}

void task_2(void)
{
  monitor_write("[*] task 2 preparing to start\n");
  clock_nanosleep(CLOCK_MONOTONIC, 0, &three_s, NULL);
  monitor_write("[*] task 2 started\n");
  clock_nanosleep(CLOCK_MONOTONIC, 0, &four_s, NULL);
  monitor_write("[!] task 2 done job\n");
}

int main(struct multiboot *mboot_ptr, u32int initial_stack)
{
  placement_address = (u32int)(&end);
  initial_esp = initial_stack;
  // Initialise all the ISRs and segmentation
  init_descriptor_tables();
  // Initialise the screen (by clearing it)
  monitor_clear();
  // Initialise the PIT to 82Hz
  __clk_init(CLOCK_MONOTONIC, __CLK_MONOTONIC_DEFAULT_FRQ);

  // Start paging.
  initialise_paging();

  // Start multitasking.
  initialise_tasking();

  if (fork())
    task_1();
  // else if (fork())
  else
    task_2();

  // Sleep forever
  clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, __CLK_INFINITE, NULL);
}
