#include "AESRand.h"

simd128 AESRand_init(){
	return _mm_setzero_si128(); 
}

simd128 increment = _mm_set_epi8(0x2f, 0x2b, 0x29, 0x25, 0x1f, 0x1d, 0x17, 0x13, 
		0x11, 0x0D, 0x0B, 0x07, 0x05, 0x03, 0x02, 0x01); 

void AESRand_increment(simd128& state){
	state += increment; 
}

std::array<simd128, 2> AESRand_rand(const simd128 state){
	simd128 penultimate = _mm_aesenc_si128(state, increment); 
	return {_mm_aesenc_si128(penultimate, increment), _mm_aesdec_si128(penultimate, increment)};
}

static __m128 toFloats(__m128i input){
	// Isolate the sign and exponent bits
	__m128i isolate = _mm_andnot_si128(_mm_set1_epi32(0xff800000), input);

	// 0x3f800000 is the magic number representing floating point 1.0
	__m128i addExponent = _mm_or_si128(_mm_set1_epi32(0x3f800000), isolate);

	// Numbers are now in between [1.0, 2.0)
	__m128 one = _mm_set1_ps(1.0);

	// Result is now in [0, 1), but we may have lost some bits.
	// We could return now, but... we can regain 9-lost bits without much effort
	__m128 fastResult = _mm_sub_ps(_mm_castsi128_ps(addExponent), one);

	return fastResult;

#if 0
	// This code takes the 9-unused bits and uses them in the bottom sometimes. This
	// may add extra bits of precision in some cases at the cost of possibly returning
	// a denormal.
	__m128i unused9bits = _mm_and_si128(_mm_set1_epi32(0xff800000), input);
	unused9bits = _mm_srli_epi32(unused9bits, 23);

	//Doing an _mm_xor_ps with those 9-bits results in a NAN error. Do xors 
	// in the integer domain, then convert back.

	return _mm_xor_ps(fastResult, _mm_castsi128_ps(unused9bits));
#endif
}

std::array<uint32_t, 8> AESRand_rand_uint32(const simd128 state){
	auto rands = AESRand_rand(state); 

	std::array<uint32_t, 8> toReturn;
	_mm_storeu_si128((__m128i*)&toReturn[0], rands[0]);
	_mm_storeu_si128((__m128i*)&toReturn[4], rands[1]);
	return toReturn; 
}

std::array<float, 8> AESRand_rand_float(const simd128 state){
	auto rands = AESRand_rand(state); 
	__m128 simd0 = toFloats(rands[0]);
	__m128 simd1 = toFloats(rands[1]);

	std::array<float, 8> toReturn;
	_mm_storeu_ps(&toReturn[0], simd0);
	_mm_storeu_ps(&toReturn[4], simd1);
	return toReturn; 
}
