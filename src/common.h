// common.h -- Defines typedefs and some global functions.
//             From JamesM's kernel development tutorials.

#ifndef COMMON_H
#define COMMON_H

// Some nice typedefs, to standardise sizes across platforms.
// These typedefs are written for 32-bit X86.
typedef unsigned int   u32int;
typedef          int   s32int;
typedef unsigned short u16int;
typedef          short s16int;
typedef unsigned char  u8int;
typedef          char  s8int;

typedef unsigned long long u64int;
typedef          long long s64int;

void outb(u16int port, u8int value);
u8int inb(u16int port);
u16int inw(u16int port);

#define PANIC(msg) panic(msg, __FILE__, __LINE__);
#define ASSERT(p) ((p) ? (void)0 : panic_assert(__FILE__, __LINE__, #p))
#define ASSERT_SUCCESS(p) (ASSERT(!(p)))

#define array_size(array) (sizeof(array) / sizeof(array[0]))

#define NULL ((void*)0)
#define EXIT_SUCCESS (0)
#define EXIT_FAILURE (-1)
#define FAILED(p)    ((p) != EXIT_SUCCESS)

extern void panic(const char *message, const char *file, u32int line);
extern void panic_assert(const char *file, u32int line, const char *desc);

extern void memcpy(void *dest, const void *src, u32int len);
extern void memset(void *dest, const int val, u32int len);

#endif // COMMON_H
