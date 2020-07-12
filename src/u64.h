#include "common.h"

static inline u64int __udivdi3(u64int num, u64int den)
{
    u64int quot, qbit;

    quot = 0;
    qbit = 1;

    if (den == 0)
    {
        return 0;
    }

    while ((long long)den >= 0)
    {
        den <<= 1;
        qbit <<= 1;
    }

    while (qbit)
    {
        if (den <= num)
        {
            num -= den;
            quot += qbit;
        }
        den >>= 1;
        qbit >>= 1;
    }

    return quot;
}