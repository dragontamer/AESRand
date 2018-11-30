#include <stdio.h>

int main(){
	vector unsigned long long simd128 = {0, 1};
	printf("%llx %llx\n", simd128[0], simd128[1]); 
	simd128 = __builtin_crypto_vcipher(simd128, (vector unsigned long long){0,0}); 
	printf("%llx %llx\n", simd128[0], simd128[1]); 
}
