all: AESRand.s AESRand 

clean:
	rm AESRand.s AESRand AESRand.o UnitTest

AESRand.s: AESRand.cpp
	g++ -march=westmere -O2 -S AESRand.cpp -o AESRand.s

AESRand.o: AESRand.cpp
	g++ -c -march=westmere -O2 AESRand.cpp -o AESRand.o

UnitTest: UnitTest.cpp AESRand.o
	g++ -march=westmere -O2 UnitTest.cpp AESRand.o -o UnitTest
