all: AESRand.s AESRand

AESRand.s: AESRand.cpp
	g++ -march=haswell -O2 -S AESRand.cpp -o AESRand.s

AESRand: AESRand.cpp
	g++ -march=haswell -O2 AESRand.cpp -o AESRand
