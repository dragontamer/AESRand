all: AESRand.s AESRand AESRand_BigCrush2

clean:
	rm AESRand.s AESRand AESRand_BigCrush

AESRand.s: AESRand.cpp
	g++ -march=westmere -O2 -S AESRand.cpp -o AESRand.s

AESRand: AESRand.cpp
	g++ -march=westmere -O2 AESRand.cpp -o AESRand

AESRand_BigCrush: AESRand_BigCrush.cpp
	g++ -march=westmere -O2 AESRand_BigCrush.cpp -o AESRand_BigCrush -ltestu01

AESRand_BigCrush2: AESRand_BigCrush2.cpp
	g++ -march=westmere -O2 AESRand_BigCrush2.cpp -o AESRand_BigCrush2 -ltestu01
