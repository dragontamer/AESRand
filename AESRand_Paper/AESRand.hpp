#ifdef __amd64__

simd128 AESRand_init(){
	return _mm_setzero_si128(); 
}

static simd128 increment = _mm_set_epi8(0x2f, 0x2b, 0x29, 0x25, 0x1f, 0x1d, 0x17, 0x13, 
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

#endif //amd64

#ifdef _ARCH_PPC64

// PPC Intrinsics defined in "64-bit ELF V2 ABI Specification", chapter 6 and Appendix A.
// GCC defines the crypto-extension

// PowerPC operates on big-endian FIPS 197 compatible AES-vectors
// Convert to big-endian and back to retain compatibility with AMD64
static simd128 endianConv(simd128 in){
	return vec_perm(in, in, (vector unsigned char){15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0});
}

simd128 AESRand_init(){
	return (simd128) {0, 0}; 
}

static simd128 increment = {0x110d0b0705030201, 0x2f2b29251f1d1713};

void AESRand_increment(simd128& state){
	state += increment; 
	//state = vec_add(state, increment); 
}

std::array<simd128, 2> AESRand_rand(const simd128 state){
	simd128 state_endian = endianConv(state);
	simd128 increment_endian = endianConv(increment);
	simd128 penultimate = __builtin_crypto_vcipher(state_endian, increment_endian); 
	simd128 first_ret = __builtin_crypto_vcipher(penultimate, increment_endian); 
	simd128 second_ret = __builtin_crypto_vncipher(penultimate, (vector unsigned long long) {0,0}); 

	// Note: this is suboptimal. A "MixColumns" can be applied at compile-time to the
	// increment_endian value to combine this XOR with the above vncipher command. Depends how much
	// we care about optimization...
	second_ret ^= increment_endian;
	return {endianConv(first_ret), endianConv(second_ret)}; 
}

std::array<uint32_t, 8> AESRand_rand_uint32(const simd128 state){
	auto rands = AESRand_rand(state); 

	std::array<uint32_t, 8> toReturn;
	toReturn[0] = rands[0][0];
	toReturn[1] = rands[0][0] >> 32;
	toReturn[2] = rands[0][1];
	toReturn[3] = rands[0][1] >> 32;
	toReturn[4] = rands[1][0];
	toReturn[5] = rands[1][0] >> 32;
	toReturn[6] = rands[1][1];
	toReturn[7] = rands[1][1] >> 32;
//	_mm_storeu_si128((__m128i*)&toReturn[0], rands[0]);
//	_mm_storeu_si128((__m128i*)&toReturn[4], rands[1]);
	return toReturn; 
}

#endif

#if __aarch64__

simd128 AESRand_init(){
	simd128 arb;
	return veorq_u8(arb, arb);
}

// Endian is reversed compared to Intel. Completely backwards...
uint8_t increment[16] = {0x01, 0x02, 0x03, 0x05, 0x07, 0x0B, 0x0D, 0x11,
	0x13, 0x17, 0x1d, 0x1f, 0x25, 0x29, 0x2b, 0x2f};

void AESRand_increment(simd128& state){
	simd128 inc = vld1q_u8(increment); 
	state = vaddq_u8(state, inc); 
}

std::array<simd128, 2> AESRand_rand(const simd128 state){
	simd128 inc = vld1q_u8(increment);
	simd128 penultimate_intel = vaesmcq_u8(vaeseq_u8(state, vdupq_n_u8(0)));
	simd128 penultimate_arm_enc = vaesmcq_u8(vaeseq_u8(penultimate_intel, (inc)));
	simd128 penultimate_arm_dec = vaesimcq_u8(vaesdq_u8(penultimate_intel, (inc)));
	return {veorq_u8(penultimate_arm_enc, (inc)), veorq_u8(penultimate_arm_dec, inc)};
}

std::array<uint32_t, 8> AESRand_rand_uint32(const simd128 state){
	auto rands = AESRand_rand(state); 

	std::array<uint32_t, 8> toReturn;
	vst1q_u8((uint8_t*) &toReturn[0], rands[0]);
	vst1q_u8((uint8_t*) &toReturn[4], rands[1]);
//	_mm_storeu_si128((__m128i*)&toReturn[0], rands[0]);
//	_mm_storeu_si128((__m128i*)&toReturn[4], rands[1]);
	return toReturn; 
}

/*
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

std::array<float, 8> AESRand_rand_float(const simd128 state){
	auto rands = AESRand_rand(state); 
	__m128 simd0 = toFloats(rands[0]);
	__m128 simd1 = toFloats(rands[1]);

	std::array<float, 8> toReturn;
	_mm_storeu_ps(&toReturn[0], simd0);
	_mm_storeu_ps(&toReturn[4], simd1);
	return toReturn; 
}
*/
#endif
