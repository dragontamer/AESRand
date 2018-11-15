#include <iostream>
#include <immintrin.h>
#include <array>
#include <stdio.h>

extern "C"{
	#include <testu01/unif01.h>
	#include <testu01/bbattery.h>
}


__m128i AESRand_init(){
	return _mm_setzero_si128(); 
}

__m128i increment = _mm_set_epi8(0x2f, 0x2b, 0x29, 0x25, 0x1f, 0x1d, 0x17, 0x13, 
		0x11, 0x0D, 0x0B, 0x07, 0x05, 0x03, 0x02, 0x01); 

void AESRand_increment(__m128i& state){
	state += increment; 
}

std::array<__m128i, 2> AESRand_rand(const __m128i state){
	__m128i penultimate = _mm_aesenc_si128(state, increment); 
	return {_mm_aesenc_si128(penultimate, increment), _mm_aesdec_si128(penultimate, increment)};
}

__m128i state = _mm_setzero_si128(); 
uint32_t buffer[8] __attribute__ ((aligned (16))); 
int buffer_state=8; 

// This 2nd test, will test all 8 numbers that comes through
unsigned int AESRand_gen(void){ 
	if(buffer_state>=8){
		AESRand_increment(state);
		auto rands = AESRand_rand(state); 
		_mm_storeu_si128((__m128i*)&buffer[0], rands[0]);
		_mm_storeu_si128((__m128i*)&buffer[4], rands[1]);
		buffer_state = 0;
	}

	return static_cast<unsigned int*>(&buffer[0])[buffer_state++]; 
}

int main(){
	// Thanks to http://www.pcg-random.org/posts/how-to-test-with-testu01.html
	unif01_Gen* gen = unif01_CreateExternGenBits("AESRand All 8xint32", AESRand_gen); 
	bbattery_BigCrush(gen); 
	unif01_DeleteExternGenBits(gen); 
	return 0;
}
