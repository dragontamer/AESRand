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
