all: AESRand.s AESRand 

clean:
	rm AESRand.s AESRand AESRand.o UnitTest

AESRand.s: AESRand.cpp
	g++ -mcrypto -O0 -S AESRand.cpp -o AESRand.s

AESRand.o: AESRand.cpp
	g++ -g -c -mcrypto -O0 AESRand.cpp -o AESRand.o

UnitTest: UnitTest.cpp AESRand.o
	g++ -g -mcrypto -O0 UnitTest.cpp AESRand.o -o UnitTest
