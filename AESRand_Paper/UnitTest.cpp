#include <iostream>
#include "AESRand.h"
#include <string.h>

int main(){
	simd128 state = AESRand_init(); 
	AESRand_increment(state);
	std::array<uint32_t, 8> ints = AESRand_rand_uint32(state);
	std::array<uint32_t, 8> matches = 
		{
			0x12e826e6,
			0x6c302fd5,
			0x83155f50,
			0xc33a3964,
			0x337eacb1,
			0xe74bf1c4,
			0xbf8be05e,
			0x5068aca6,
		};

	if(memcmp((void*)&ints[0], (void*)&matches[0], sizeof(ints)) == 0){
		std::cout << "Unit Test passed" << std::endl;
	} else {
		std::cout << "Unit Test failed" << std::endl;
		for(int i=0; i<8; i++){
			std::cout << std::hex << ints[i] << "  " << matches[i] << std::endl;
		}
	}


}
