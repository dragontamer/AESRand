#include <iostream>
#include <immintrin.h>
#include <array>


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

int main(){
	std::cout << "Running 5-billion iterations (160 Billion-bytes of Random Data)" << std::endl; 
	__m128i state = AESRand_init(); 
	__m128i total = _mm_setzero_si128(); 
	__m128i total2 = _mm_setzero_si128(); 

	for(long long i=0; i<5000000000; i++){
		AESRand_increment(state); 
		auto rands = AESRand_rand(state); 
		total += rands[0];
		total2 += rands[1]; 
	}

	total += total2;
	std::cout << "Dummy print to negate optimizer: " << total[0] << std::endl;
}
