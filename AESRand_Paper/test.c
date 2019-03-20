#if 0
#include <stdio.h>
#include <altivec.h>

void printArray(char array[16]){
	for(int i=0; i<16; i++){
		printf("%02x ", array[i]); 
	}
	printf("\n"); 
}

int main(){
	char array[16];
	vector unsigned long long simd128 = {0, 1};
	memcpy(array, &simd128, 16); 
	printArray(array); 
	simd128 = vec_perm(simd128, simd128, (vector unsigned char){15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0});
	simd128 = __builtin_crypto_vcipher(simd128, (vector unsigned long long){0,0}); 
	simd128 = vec_perm(simd128, simd128, (vector unsigned char){15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0});
	memcpy(array, &simd128, 16); 
	printArray(array); 
}
#endif

#include <string.h>
#include <stdio.h>
#include <arm_neon.h>

void printArray(uint8_t array[16]){
	for(int i=0; i<16; i++){
		printf("%02x ", array[i]); 
	}
	printf("\n"); 
}

int main(){
	//uint8_t increment[16] = {0x2f, 0x2b, 0x29, 0x25, 0x1f, 0x1d, 0x17, 0x13,
	//		0x11, 0x0D, 0x0B, 0x07, 0x05, 0x03, 0x02, 0x01};
	uint8_t increment[16] = {0x01, 0x02, 0x03, 0x05, 0x07, 0x0B, 0x0D, 0x11, 
				0x13, 0x17, 0x1d, 0x1f, 0x25, 0x29, 0x2b, 0x2f};
	uint8_t array[16] = {
				0, 0, 0, 0,
				0, 0, 0, 0,
				1, 0, 0, 0,
				0, 0, 0, 0,
			};
	uint8x16_t simd128 = vld1q_u8(array);
	printArray(array); 

	simd128 = vaesmcq_u8(vaeseq_u8(simd128, vdupq_n_u8(0)));
	memcpy(array, &simd128, 16); 
	printArray(array); 

	simd128 ^= vld1q_u8(increment);
	memcpy(array, &simd128, 16); 
	printArray(array); 

/*
	simd128 = vec_perm(simd128, simd128, (vector unsigned char){15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0});
	simd128 = __builtin_crypto_vcipher(simd128, (vector unsigned long long){0,0}); 
	simd128 = vec_perm(simd128, simd128, (vector unsigned char){15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0});
	memcpy(array, &simd128, 16); 
	printArray(array); 
*/
}
