#ifndef SG_TYPES
#define SG_TYPES 1

#include <stdint.h>

typedef unsigned long long ull;
typedef unsigned long ul;
typedef unsigned u;
typedef long long ll;
typedef long l;
typedef float f;

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t   i8;
typedef int16_t  i16;
typedef int32_t  i32;
typedef int64_t  i64;

typedef uint_fast8_t  u8f;
typedef uint_fast16_t u16f;
typedef uint_fast32_t u32f;
typedef uint_fast64_t u64f;

typedef int_fast8_t  i8f;
typedef int_fast16_t i16f;
typedef int_fast32_t i32f;
typedef int_fast64_t i64f;

typedef float       f32;
typedef double      f64;
typedef long double f128;

typedef ull SgSize;
typedef int SgInt;
typedef u   SgUInt;
typedef f32 SgFloat;

typedef enum SgResult {
	SG_SUCCESS = 0,
} SgResult;
enum SgBool {
	SG_FALSE = 0,
	SG_TRUE = 1,
};
typedef uint_fast8_t SgBool;
typedef uint32_t SgFlags;


#endif
