/*
 *  ByteOrder.h
 *  mindstormssimulation
 *
 *  Created by Torsten Kammer on 30.06.10.
 *  Copyright 2010 Ferroequinologist.de. All rights reserved.
 *
 */

static inline uint32_t SwapU32LittleToHost(uint32_t arg)
{
#if __BIG_ENDIAN__
    return ((arg & 0xFF) << 24) | ((arg & 0xFF00) << 8) | ((arg & 0xFF0000) >> 8) | ((arg & 0xFF000000) >> 24);
#else
    return arg;
#endif
}

static inline uint16_t SwapU16LittleToHost(uint16_t arg)
{
#if __BIG_ENDIAN__
    return ((arg & 0xFF) << 8) | ((arg & 0xFF00) >> 8);
#else
    return arg;
#endif
}

static inline void SwapU16LittleToHost(uint16_t *args, unsigned length)
{
#if __BIG_ENDIAN__
    for (unsigned i = 0; i < length; i++)
        args[i] = SwapU16LittleToHost(args[i]);
#endif
}

// Swapping to and from is of course identical, but this might help with retaining overveiw
#define SwapU16HostToLittle(a) SwapU16LittleToHost(a)
#define SwapU32HostToLittle(a) SwapU16LittleToHost(a)

static inline void SwapU32LittleToHost(uint32_t *args, unsigned length)
{
#if __BIG_ENDIAN__
    for (unsigned i = 0; i < length; i++)
        args[i] = SwapU32LittleToHost(args[i]);
#endif
}

template <class T> static inline T SwapLittleToHost(T arg)
{
    union
    {
        T t;
        uint32_t u32;
        uint16_t u16;
    } convert = {arg};
    if (sizeof(T) == sizeof(uint16_t)) convert.u16 = SwapU16LittleToHost(convert.u16);
    else if (sizeof(T) == sizeof(uint32_t)) convert.u32 = SwapU32LittleToHost(convert.u32);
    return convert.t;
}

template <class T> static inline T SwapHostToLittle(T arg)
{
    return SwapLittleToHost<T>(arg);
}
