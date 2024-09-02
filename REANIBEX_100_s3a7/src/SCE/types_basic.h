/******************************************************************************
 * Name      : R100_S3A7                                                      *
 * Copyright : Osatu S. Coop                                                  *
 * Compiler  : GNU ARM Embedded                                               *
 * Target    : Reanibex Serie 100                                             *
 ******************************************************************************/

/*!
* @file        Types_Basic.h
* @brief       Basic Types' definition.
* @version     0.0
* @date        2007-10-11
* @author      EG (egarcia@osatu.com)
* @warning     EG           2007-10-11      V0.0      .- First edition
*
*
*/

#ifndef TYPES_BASIC_H
#define TYPES_BASIC_H
/******************************************************************************
**Includes
*/

/******************************************************************************
** Defines
*/

#define FALSE       0uL               ///< Boolean False
#define TRUE        1uL               ///< Boolean True

#define OFF         0uL               ///< State Off
#define ON          1uL               ///< State On
#define BLINK       2uL               ///< State Blinking

#define ENABLED     1uL               ///< Enable  feature
#define DISABLED    0uL               ///< Disable feature

#define INC         1uL               ///< Increment variable
#define DEC         0uL               ///< Decrement variable

#ifndef NULL
#define NULL        0uL               ///< Null definition
#endif //  NULL_DEFINED

#define UNUSED(x)                   (void) (x)



/// macro used to toggle a boolean (LSB)
#define Toggle_bool(x)                (x ^= TRUE)

/******************************************************************************
** Typedefs
*/

#ifndef VOID_T_DEFINED
#define VOID_T_DEFINED
/// Void type (use the standard name)
// typedef void                void_t;
#endif //  VOID_T_DEFINED

#ifndef BOOL_T_DEFINED
#define BOOL_T_DEFINED
/// Bool type
typedef unsigned char       bool_t;
#endif // BOOL_T_DEFINED

#ifndef BOOL32_T_DEFINED
#define BOOL32_T_DEFINED
/// Bool 32
typedef unsigned long       bool32_t;
#endif // BOOL32_T_DEFINED

#ifndef CHAR_T_DEFINED
#define CHAR_T_DEFINED
/// Char type
typedef char                char_t;
#endif // CHAR_T_DEFINED

#ifndef UCHAR_T_DEFINED
#define UCHAR_T_DEFINED
/// Unsigned char type
typedef unsigned char       uchar_t;
#endif // UCHAR_T_DEFINED

#ifndef UINT8_T_DEFINED
#define UINT8_T_DEFINED
/// Unsigned int 8
typedef unsigned char       uint8_t;
#endif // UINT8_T_DEFINED

#ifndef UINT16_T_DEFINED
#define UINT16_T_DEFINED
/// Unsigned int 16
typedef unsigned short      uint16_t;
#endif // UINT16_T_DEFINED

#ifndef UINT32_T_DEFINED
#define UINT32_T_DEFINED
/// Unsigned int 32
typedef unsigned long       uint32_t;
#endif // UINT32_T_DEFINED

#ifndef UINT64_T_DEFINED
#define UINT64_T_DEFINED
/// Unsigned int 64
typedef unsigned long long  uint64_t;
#endif // UINT64_T_DEFINED

#ifndef INT8_T_DEFINED
#define INT8_T_DEFINED
/// Signed int 8
typedef signed char         int8_t;
#endif // INT8_T_DEFINED

#ifndef INT16_T_DEFINED
#define INT16_T_DEFINED
/// Signed int 16
typedef signed short        int16_t;
#endif // INT16_T_DEFINED

#ifndef INT32_T_DEFINED
#define INT32_T_DEFINED
/// Signed int 32
typedef signed long         int32_t;
#endif // INT32_T_DEFINED

#ifndef INT64_T_DEFINED
#define INT64_T_DEFINED
/// Signed int 64
typedef signed long long    int64_t;
#endif // INT64_T_DEFINED

#ifndef BITSET8_T_DEFINED
#define BITSET8_T_DEFINED
/// Bitset 8
typedef unsigned char       bitset8_t;
#endif // BITSET8_T_DEFINED

#ifndef BITSET16_T_DEFINED
#define BITSET16_T_DEFINED
/// Bitset 16
typedef unsigned short      bitset16_t;
#endif // BITSET16_T_DEFINED

#ifndef BITSET32_T_DEFINED
#define BITSET32_T_DEFINED
/// Bitset 32
typedef unsigned long       bitset32_t;
#endif // BITSET32_T_DEFINED

#ifndef FLOAT_T_DEFINED
#define FLOAT_T_DEFINED
/// Float type
typedef float               float_t;
#endif // FLOAT_T_DEFINED

#ifndef DOUBLE_T_DEFINED
#define DOUBLE_T_DEFINED
/// Double type
typedef long double         double_t;
#endif // DOUBLE_T_DEFINED

/// Definition of time structure
typedef struct {
    uint8_t        sec;         ///< seconds [00..59]
    uint8_t        min;         ///< minutes [00..59]
    uint8_t        hour;        ///< hours   [00..23]
} TIME_t;

/// Definition of date structure
typedef struct {
    uint8_t        date;        ///< date  [01..31]
    uint8_t        month;       ///< month [01..12]
    uint8_t        year;        ///< year from 2000 [00..99]
} DATE_t;

// delay implemented by soft in a loop (in fast mode, running at nominal speed)
#define SOFT_FAST_DELAY_1MSEC   15000                           ///< Fast delay 1 msec
#define SOFT_FAST_DELAY_2MSEC   (SOFT_FAST_DELAY_1MSEC * 2)     ///< Fast delay 2 msecs
#define SOFT_FAST_DELAY_10MSEC  (SOFT_FAST_DELAY_1MSEC * 10)    ///< Fast delay 10 msecs
#define SOFT_FAST_DELAY_20MSEC  (SOFT_FAST_DELAY_1MSEC * 20)    ///< Fast delay 20 msecs
#define SOFT_FAST_DELAY_50MSEC  (SOFT_FAST_DELAY_1MSEC * 50)    ///< Fast delay 50 msecs
#define SOFT_FAST_DELAY_100MSEC (SOFT_FAST_DELAY_1MSEC * 100)   ///< Fast delay 100 msecs
#define SOFT_FAST_DELAY_200MSEC (SOFT_FAST_DELAY_1MSEC * 200)   ///< Fast delay 200 msecs
#define SOFT_FAST_DELAY_500MSEC (SOFT_FAST_DELAY_1MSEC * 500)   ///< Fast delay 500 msecs

// delay implemented by soft in a loop (in slow mode, running at low speed)
#define SOFT_SLOW_DELAY_1MSEC   (SOFT_FAST_DELAY_1MSEC / 8)     ///< Slow delay 1 msec
#define SOFT_SLOW_DELAY_2MSEC   (SOFT_SLOW_DELAY_1MSEC * 2)     ///< Slow delay 2 msecs
#define SOFT_SLOW_DELAY_10MSEC  (SOFT_SLOW_DELAY_1MSEC * 10)    ///< Slow delay 10 msecs
#define SOFT_SLOW_DELAY_20MSEC  (SOFT_SLOW_DELAY_1MSEC * 20)    ///< Slow delay 20 msecs
#define SOFT_SLOW_DELAY_50MSEC  (SOFT_SLOW_DELAY_1MSEC * 50)    ///< Slow delay 50 msecs
#define SOFT_SLOW_DELAY_100MSEC (SOFT_SLOW_DELAY_1MSEC * 100)   ///< Slow delay 100 msecs
#define SOFT_SLOW_DELAY_150MSEC (SOFT_SLOW_DELAY_1MSEC * 150)   ///< Slow delay 150 msecs
#define SOFT_SLOW_DELAY_200MSEC (SOFT_SLOW_DELAY_1MSEC * 200)   ///< Slow delay 200 msecs
#define SOFT_SLOW_DELAY_500MSEC (SOFT_SLOW_DELAY_1MSEC * 500)   ///< Slow delay 500 msecs

#define MSECS_PER_TICK          10           ///< number of miliseconds in a single tick

#define TICKS_PER_SECOND        (1000 / MSECS_PER_TICK)         ///< Ticks per second
#define TICKS_PER_MINUTE        ((  60 * 1000) / MSECS_PER_TICK)///< Ticks per minute
#define TICKS_PER_HOUR          ((3600 * 1000) / MSECS_PER_TICK)///< Ticks per hour

#define OSTIME_NO_DELAY         (0)                         ///< No delay
#define OSTIME_10MSEC           (TICKS_PER_SECOND / 100)    ///< 10 msecs
#define OSTIME_20MSEC           (OSTIME_10MSEC * 2)         ///< 20 msecs
#define OSTIME_30MSEC           (OSTIME_10MSEC * 3)         ///< 30 msecs
#define OSTIME_40MSEC           (OSTIME_10MSEC * 4)         ///< 40 msecs
#define OSTIME_50MSEC           (OSTIME_10MSEC * 5)         ///< 50 msecs
#define OSTIME_60MSEC           (OSTIME_10MSEC * 5)         ///< 60 msecs
#define OSTIME_70MSEC           (OSTIME_10MSEC * 5)         ///< 70 secs
#define OSTIME_80MSEC           (OSTIME_10MSEC * 5)         ///< 80 msecs
#define OSTIME_90MSEC           (OSTIME_10MSEC * 5)         ///< 90 msecs
#define OSTIME_100MSEC          (OSTIME_10MSEC * 10)        ///< 100 msecs
#define OSTIME_150MSEC          (OSTIME_10MSEC * 15)        ///< 150 secs
#define OSTIME_200MSEC          (OSTIME_10MSEC * 20)        ///< 200 msecs
#define OSTIME_300MSEC          (OSTIME_10MSEC * 30)        ///< 300 msecs
#define OSTIME_400MSEC          (OSTIME_10MSEC * 40)        ///< 400 msecs
#define OSTIME_500MSEC          (OSTIME_10MSEC * 50)        ///< 500 msecs
#define OSTIME_600MSEC          (OSTIME_10MSEC * 60)        ///< 600 msecs
#define OSTIME_1SEC             (OSTIME_10MSEC * 100)       ///< 1 sec
#define OSTIME_2SEC             (OSTIME_10MSEC * 200)       ///< 2 secs
#define OSTIME_3SEC             (OSTIME_10MSEC * 300)       ///< 3 secs
#define OSTIME_4SEC             (OSTIME_10MSEC * 400)       ///< 4 secs
#define OSTIME_5SEC             (OSTIME_10MSEC * 500)       ///< 5 secs
#define OSTIME_6SEC             (OSTIME_10MSEC * 600)       ///< 6 secs
#define OSTIME_7SEC             (OSTIME_10MSEC * 700)       ///< 7 secs
#define OSTIME_8SEC             (OSTIME_10MSEC * 800)       ///< 8 secs
#define OSTIME_9SEC             (OSTIME_10MSEC * 900)       ///< 9 secs
#define OSTIME_10SEC            (OSTIME_10MSEC * 1000)      ///< 10 secs
#define OSTIME_12SEC            (OSTIME_10MSEC * 1200)      ///< 12 secs
#define OSTIME_14SEC            (OSTIME_10MSEC * 1400)      ///< 14 secs
#define OSTIME_15SEC            (OSTIME_10MSEC * 1500)      ///< 15 secs
#define OSTIME_20SEC            (OSTIME_10MSEC * 2000)      ///< 20 secs
#define OSTIME_30SEC            (OSTIME_10MSEC * 3000)      ///< 30  secs
#define OSTIME_60SEC            (OSTIME_10MSEC * 6000)      ///< 60 secs


#define COMPILE_ASSERT(x) extern int __dummy[(int)x]

/******************************************************************************
** Prototypes
*/

#endif  /*TYPES_BASIC_H*/

/*
** $Log$
**
** end of Types_Basic.h
******************************************************************************/
