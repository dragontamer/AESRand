#ifndef AESRAND_H
#define AESRAND_H

// I expect ifdefs galore in this file 

#if __amd64__
#include <immintrin.h>
typedef __m128i simd128;
typedef __m128 simd128_float;
typedef __m128i simd128_uint32;
#endif

#if _ARCH_PPC64
#include <altivec.h>
typedef vector unsigned long long  simd128;
typedef vector float simd128_float;
typedef vector unsigned int simd128_uint32;
#endif

#if __aarch64__ 
#include <arm_neon.h>
typedef uint8x16_t  simd128;
typedef float32x4_t simd128_float;
typedef uint32x4_t simd128_uint32;
#endif

#include <array>
#include <cstdint>

simd128 AESRand_init();
void AESRand_increment(simd128& state);
std::array<simd128, 2> AESRand_rand(const simd128 state);

std::array<float, 8> AESRand_rand_float(const simd128 state); 
std::array<uint32_t, 8> AESRand_rand_uint32(const simd128 state); 

/*
std::array<uint32_t, 8> AESRand_randInt_range16(const simd128 state, uint16_t lower_bound, uint16_t upper_bound); 
std::array<uint32_t, 4> AESRand_randInt_range32(const simd128 state, uint32_t lower_bound, uint32_t upper_bound); 
uint64_t AESRand_randInt_range64(const simd128 state, uint64_t lower_bound, uint64_t upper_bound); 
*/

#endif
