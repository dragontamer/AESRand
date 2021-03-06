// Note: There are three programs, depending on the #if statements that are
// controlled in the main() function. By default, the #if statement is set to run
// the timerTest() and timerTestOthers() code to calculate GBps. Toggle the #if statement
// to create a generator-program for PractRand or TestU01.

#include "pch.h"
#include <iostream>
#include <intrin.h>
#include <windows.h>
#include <array>

// This "primeIncrement" variable serves as the only constant used throughout
// the entire algorithm. It is composed of a simple 16-byte sequence: 1 followed by the next
// 15 prime numbers. Remember that _mm_set_epi8 is backwards, so I had to enter these prime numbers
// backwards.

// There are two major uses of this constant:
// 1. As an "adder" in AESRand_increment, the upper8-bytes and lower8-bytes serve as two parallel 64-bit
// numbers in a vectorized addition. This guarantees that all 16-bytes of the state changes on every 
// increment of the RNG-state. And secondly, it guarantees that this RNG as a whole has a 2^64 cycle 
// length.
//
// 2. As the "key" to the AESENC and AESDEC calls, the primeIncrement is implicitly XOR'd (as part of the 
// instructions) to the cipher. This further complicates the substitution / permutation network of AES 
// by ensuring every byte changes between every iteration of the cipher.

__m128i primeIncrement = _mm_set_epi8(0x2f, 0x2b, 0x29, 0x25, 0x1f, 0x1d, 0x17, 0x13, // High 64-bits
										0x11, 0x0D, 0x0B, 0x07, 0x05, 0x03, 0x02, 0x01); // Low 64-bits

// This increment is the bottleneck: 1-cycle of latency, with self-dependent state. This
// cannot execute any faster than once-per-cycle, even though AMD Zen / Intel Skylake can 
// execute 3 or 4 128-bit instructions per cycle.
void AESRand_increment(__m128i& state) {

	// Two 64-bit adds in parallel through the magic of AVX2 instructions. Since 
	// primeIncrement.high and primeIncrement.low are odd, this increment-cycle 
	// trivially has 2^64 cycle-length.
	state = _mm_add_epi64(state, primeIncrement);
}

// A singular AES Round is actually quite bad against 
// single bit changes. It requires 4+ rounds of AES before
// an adequate 1-bit change propagates to pass 16+ GB of PractRand's 
// statistical tests.

// Instead of "properly" securing against 1-bit changes, I wrote the AESRand_increment()
// and parallelStream() functions to ensure that a large-number of bytes change.
// This "AESRand_rand" function is SURPRISINGLY weak as a mixing-function, but if used
// correctly, it seems to work very well.

std::array<__m128i, 2> AESRand_rand(const __m128i state) {
	__m128i penultimate = _mm_aesenc_si128(state, primeIncrement);
	return { _mm_aesenc_si128(penultimate, primeIncrement),
		_mm_aesdec_si128(penultimate, primeIncrement)};

	// Surpringly, AESDEC is NOT the inverse of AESENC, and thus can be used
	// here to "stretch" the 16-byte seed into another set of 16-bytes
}


/* Some party tricks which are actually quite useful! I personally learned these tricks
from https://github.com/clMathLibraries/clRNG and xoroshiro128plus, so I had to make sure I could implement them
in my RNG!! */

// "Jump" the RNG forward by a huge jump. Helpful if you want to "reserve" numbers in a simulation.
// Ex: spawn a new thread, "reserve" 10,000 Random numbers to the sub-thread, and then carry on a 
// parallel thread with someone who owns the state. The subthread only need to be "passed" the __m128 state,
// and can generate its own RNGs in parallel. Passing a single 128-bit value is WAY faster than passing 
// 10,000+ random numbers across threads.
void AESRand_jumpState(__m128i& state, long long jumpLength) {
	// A large number of additions is simply a single multiplication.
	long long lowJump = primeIncrement.m128i_u64[0] * jumpLength;
	long long highJump = primeIncrement.m128i_u64[1] * jumpLength;

	state.m128i_u64[0] += lowJump;
	state.m128i_u64[1] += highJump;
}

// Return a fully parallel stream, one guarenteed to not "intersect" with the originalStream
// A parallel stream will have an entire 2^64 cycle to itself.
__m128i AESRand_parallelStream(__m128i originalStream) {
	__m128i copy = originalStream;

	// 64-bit LCGRNG worked, but is mathematically a bit more difficult to prove various
	// properties. Passed 4TB of PractRand.
	// copy.m128i_u64[1] = 6364136223846793005ULL * copy.m128i_u64[1] + 1442695040888963407ULL;

	// This "increment by Pi" seems to provide a parallel stream, and passes 4TB+ of PractRand
	// as well, although with one unusual result (false positive?)

	// First 16 digits of pi  (in decimal 10, written in Hexadecimal). Last value 
	// happens to be odd, providing us a 2^64 cycle in the upper 2^64 bits, generating
	// all parallel streams.

	copy.m128i_u64[1] += 0x3141592653589793; 

	return copy;
}


// A simple timer routine. Run this to benchmark the GBps on your machine.
// There are two tests here, which helps demonstrate the massive parallelism on modern
// machines, even within a single core. First, a "single state" version of 
// the RNG will be run. Second, a "double-state" version will be run, and you'll notice
// that the double-state version is barely any slower, thanks to the magic of instruction-level parallelism
void timerTest() {
	LARGE_INTEGER start, end, frequency;
	constexpr long long ITERATIONS = 5000000000;

	// "total" variables are used to ensure that the random-numbers are used
	// to prevent the optimizer from ignoring our code. We will print the grand-total
	// to std::cout at the end of the timer benchmark.
	__m128i total = _mm_setzero_si128();
	__m128i total2 = _mm_setzero_si128();
	__m128i total3 = _mm_setzero_si128();
	__m128i total4 = _mm_setzero_si128();

	std::cout << "Beginning Single-state 'serial' test" << std::endl;

	__m128i singleState = _mm_setzero_si128();

	QueryPerformanceFrequency(&frequency);

	QueryPerformanceCounter(&start);
		for (long long i = 0; i < ITERATIONS; i++) {
			AESRand_increment(singleState);
			auto randVals = AESRand_rand(singleState);
			total = _mm_add_epi64(total, randVals[0]);
			total2 = _mm_add_epi64(total2, randVals[1]);
		}
	QueryPerformanceCounter(&end);
	total = _mm_add_epi64(total, total2);

	double time = (end.QuadPart - start.QuadPart)*1.0 / frequency.QuadPart;
	std::cout << "Total Seconds: " << time << std::endl;

	{
		constexpr double BYTES_PER_ITERATION = 32.0;
		constexpr long long GIGABYTE = 1 << 30;
		std::cout << "GBps: " << (ITERATIONS * BYTES_PER_ITERATION) / (GIGABYTE * time) << std::endl;
		std::cout << "Dummy Benchmark anti-optimizer print: " << total.m128i_u64[0] + total.m128i_u64[1] << std::endl;
	}

	// Run two states in parallel (on a single core) for maximum ILP and throughput!

	// The first seed is zero. The 2nd seed is generated with AESRand_parallelStream.
	__m128i parallel_state[2] = { _mm_setzero_si128(), AESRand_parallelStream(_mm_setzero_si128()) };

	// More "parallel totals".
	total = _mm_setzero_si128();
	total2 = _mm_setzero_si128();
	total3 = _mm_setzero_si128();
	total4 = _mm_setzero_si128();

	std::cout << "Beginning Parallel (2x) test: instruction-level parallelism" << std::endl;

	QueryPerformanceCounter(&start);
	
	for (long long i = 0; i < ITERATIONS; i++) {
		AESRand_increment(parallel_state[0]);
		AESRand_increment(parallel_state[1]);
		auto randVals1 = AESRand_rand(parallel_state[0]);
		auto randVals2 = AESRand_rand(parallel_state[1]);
		total = _mm_add_epi64(total, randVals1[0]);
		total2 = _mm_add_epi64(total2, randVals1[1]);
		total3 = _mm_add_epi64(total3, randVals2[0]);
		total4 = _mm_add_epi64(total4, randVals2[1]);
	}

	QueryPerformanceCounter(&end);

	total = _mm_add_epi64(total, total2);
	total = _mm_add_epi64(total, total3);
	total = _mm_add_epi64(total, total4);

	time = (end.QuadPart - start.QuadPart)*1.0 / frequency.QuadPart;

	{
		constexpr double BYTES_PER_ITERATION = 64.0; // The number of bytes-per-iteration has grown 2x in the parallel test.
		constexpr long long GIGABYTE = 1 << 30;

		std::cout << "Time: " << time << std::endl;
		std::cout << "GBps: " << (ITERATIONS * BYTES_PER_ITERATION) / (GIGABYTE * time) << std::endl;
		std::cout << "Dummy Benchmark anti-optimizer print: " << total.m128i_u64[0] + total.m128i_u64[1] << std::endl;
	}
}

// pcg32_random declarations
typedef struct { uint64_t state;  uint64_t inc; } pcg32_random_t;
uint32_t pcg32_random_r(pcg32_random_t* rng);

// xoshiro256+ declarations for the test
extern uint64_t s[4];
uint64_t next(void);

#include <random>
void timerTestOthers() {
	LARGE_INTEGER start, end, frequency;
	constexpr long long ITERATIONS = 5000000000;
	double time;

	// The states of various RNGs. The
	// xoshiro256plus code is barebones with a global s[4] as its state

	std::mt19937 mt;
	pcg32_random_t pcg32 = {0, 1};
	s[0] = pcg32_random_r(&pcg32);
	s[1] = pcg32_random_r(&pcg32);
	s[2] = pcg32_random_r(&pcg32);
	s[3] = pcg32_random_r(&pcg32);

	// My code takes advantage of ILP and "parallel additions", it is only fair
	// I give the same advantage to other RNGs.
	uint32_t total32Bit[4] = { 0, 0, 0, 0 };
	uint64_t total64Bit[4] = { 0, 0, 0, 0 }; 

	QueryPerformanceFrequency(&frequency);

	std::cout << "\nTesting mt19937" << std::endl;

	QueryPerformanceCounter(&start);
	for (long long i = 0; i < ITERATIONS; i+=4) {
		total32Bit[0] += mt();
		total32Bit[1] += mt();
		total32Bit[2] += mt();
		total32Bit[3] += mt();
	}
	QueryPerformanceCounter(&end);
	total32Bit[0] += total32Bit[1] + total32Bit[2] + total32Bit[3];
	time = (end.QuadPart - start.QuadPart)*1.0 / frequency.QuadPart;

	{
		constexpr double BYTES_PER_ITERATION = 4.0; 
		constexpr long long GIGABYTE = 1 << 30;

		std::cout << "Time: " << time << std::endl;
		std::cout << "GBps: " << (ITERATIONS * BYTES_PER_ITERATION) / (GIGABYTE * time) << std::endl;
		std::cout << "Dummy Benchmark anti-optimizer print: " << total32Bit[0] << std::endl;
	}

	memset(total32Bit, 0, sizeof(total32Bit)); 

	std::cout << "\nTesting pcg32 Unrolled x4" << std::endl;

	QueryPerformanceCounter(&start);
	for (long long i = 0; i < ITERATIONS; i += 4) {
		total32Bit[0] += pcg32_random_r(&pcg32);
		total32Bit[1] += pcg32_random_r(&pcg32);
		total32Bit[2] += pcg32_random_r(&pcg32);
		total32Bit[3] += pcg32_random_r(&pcg32);
	}
	QueryPerformanceCounter(&end);
	total32Bit[0] += total32Bit[1] + total32Bit[2] + total32Bit[3];
	time = (end.QuadPart - start.QuadPart)*1.0 / frequency.QuadPart;

	{
		constexpr double BYTES_PER_ITERATION = 4.0;
		constexpr long long GIGABYTE = 1 << 30;

		std::cout << "Time: " << time << std::endl;
		std::cout << "GBps: " << (ITERATIONS * BYTES_PER_ITERATION) / (GIGABYTE * time) << std::endl;
		std::cout << "Dummy Benchmark anti-optimizer print: " << total32Bit[0] << std::endl;
	}

	memset(total32Bit, 0, sizeof(total32Bit));

	std::cout << "\nTesting pcg32" << std::endl;

	QueryPerformanceCounter(&start);
	for (long long i = 0; i < ITERATIONS; i += 1) {
		total32Bit[0] += pcg32_random_r(&pcg32);
	}
	QueryPerformanceCounter(&end);
	time = (end.QuadPart - start.QuadPart)*1.0 / frequency.QuadPart;

	{
		constexpr double BYTES_PER_ITERATION = 4.0;
		constexpr long long GIGABYTE = 1 << 30;

		std::cout << "Time: " << time << std::endl;
		std::cout << "GBps: " << (ITERATIONS * BYTES_PER_ITERATION) / (GIGABYTE * time) << std::endl;
		std::cout << "Dummy Benchmark anti-optimizer print: " << total32Bit[0] << std::endl;
	}

	memset(total32Bit, 0, sizeof(total32Bit));

	std::cout << "\nTesting xoshiro256plus Unrolled x4" << std::endl;

	QueryPerformanceCounter(&start);
	for (long long i = 0; i < ITERATIONS; i += 4) {
		total64Bit[0] += next();
		total64Bit[1] += next();
		total64Bit[2] += next();
		total64Bit[3] += next();
	}
	QueryPerformanceCounter(&end);
	total64Bit[0] += total64Bit[1] + total64Bit[2] + total64Bit[3];
	time = (end.QuadPart - start.QuadPart)*1.0 / frequency.QuadPart;

	{
		constexpr double BYTES_PER_ITERATION = 8.0;
		constexpr long long GIGABYTE = 1 << 30;

		std::cout << "Time: " << time << std::endl;
		std::cout << "GBps: " << (ITERATIONS * BYTES_PER_ITERATION) / (GIGABYTE * time) << std::endl;
		std::cout << "Dummy Benchmark anti-optimizer print: " << total64Bit[0] << std::endl;
	}

	memset(total64Bit, 0, sizeof(total64Bit));

	std::cout << "\nTesting xoshiro256plus" << std::endl;

	QueryPerformanceCounter(&start);
	for (long long i = 0; i < ITERATIONS; i += 1) {
		total64Bit[0] += next();
	}
	QueryPerformanceCounter(&end);
	total64Bit[0] += total64Bit[1] + total64Bit[2] + total64Bit[3];
	time = (end.QuadPart - start.QuadPart)*1.0 / frequency.QuadPart;

	{
		constexpr double BYTES_PER_ITERATION = 8.0;
		constexpr long long GIGABYTE = 1 << 30;

		std::cout << "Time: " << time << std::endl;
		std::cout << "GBps: " << (ITERATIONS * BYTES_PER_ITERATION) / (GIGABYTE * time) << std::endl;
		std::cout << "Dummy Benchmark anti-optimizer print: " << total64Bit[0] << std::endl;
	}

	memset(total64Bit, 0, sizeof(total64Bit));
}


// The "PlusOne" generator exists as a control for the benchmark.
// It only adds +1 to the 128-bit XMM register, and returns the state
// as the two return values. The purpose of the PlusOne
// test is to measure the amount of overhead that the benchmark code has

void PlusOne_increment(__m128i& state) {
	state = _mm_add_epi64(state, _mm_set_epi64x(0, 1)); 
}


std::array<__m128i, 2> PlusOne_rand(const __m128i state) {
	return { state, state }; 
}

// A "Control" test. This control test doesn't perfectly
// measure the overhead, because Instruction-level Parallelism allows the
// overhead costs to be mostly hidden in practice. Still, I figure it is worth
// testing the overhead somehow, and I can't think of a better way to do so.
void timerTestPlusOne() {
	LARGE_INTEGER start, end, frequency;
	constexpr long long ITERATIONS = 5000000000;

	// "total" variables are used to ensure that the random-numbers are used
	// to prevent the optimizer from ignoring our code. We will print the grand-total
	// to std::cout at the end of the timer benchmark.
	__m128i total = _mm_setzero_si128();
	__m128i total2 = _mm_setzero_si128();

	std::cout << "\nBeginning PlusOne XMM-registers Test" << std::endl;

	__m128i singleState = _mm_setzero_si128();

	QueryPerformanceFrequency(&frequency);

	QueryPerformanceCounter(&start);
	for (long long i = 0; i < ITERATIONS; i++) {
		PlusOne_increment(singleState);
		auto randVals = PlusOne_rand(singleState); // Not really "random" values. 
		total = _mm_add_epi64(total, randVals[0]);
		total2 = _mm_add_epi64(total2, randVals[1]);
	}
	QueryPerformanceCounter(&end);
	total = _mm_add_epi64(total, total2);

	double time = (end.QuadPart - start.QuadPart)*1.0 / frequency.QuadPart;
	std::cout << "Total Seconds: " << time << std::endl;

	{
		constexpr double BYTES_PER_ITERATION = 32.0;
		constexpr long long GIGABYTE = 1 << 30;
		std::cout << "GBps: " << (ITERATIONS * BYTES_PER_ITERATION) / (GIGABYTE * time) << std::endl;
		std::cout << "Dummy Benchmark anti-optimizer print: " << total.m128i_u64[0] + total.m128i_u64[1] << std::endl;
	}

#if 0
	/*
	WARNING!! THIS TEST IS AUTOVECTORIZED
	
	VisualStudio 2017 turns this "PlusOne 64-bit Registers" code into:

	001d0	48 ff c3	 inc	 rbx
	001d3	c4 e1 f9 6e c3	 vmovq	 xmm0, rbx
	001d8	c4 e2 79 59 c0	 vpbroadcastq xmm0, xmm0
	001dd	c5 f9 d4 f6	 vpaddq	 xmm6, xmm0, xmm6
	001e1	48 83 ef 01	 sub	 rdi, 1
	001e5	75 e9		 jne	 SHORT $LL7@timerTestP

	This is not good, so I've commented it out to leave it as a reminder as a bad test.
	*/

	std::cout << "Beginning PlusOne 64-bit Registers Test" << std::endl;

	long long state64Bit = 0;
	long long total64[2] = { 0, 0 };

	QueryPerformanceFrequency(&frequency);

	QueryPerformanceCounter(&start);
	for (long long i = 0; i < ITERATIONS; i++) {
		state64Bit++;
		total64[0] += state64Bit;
		total64[1] += state64Bit;
	}
	QueryPerformanceCounter(&end);
	total64[0] += total64[1];

	time = (end.QuadPart - start.QuadPart)*1.0 / frequency.QuadPart;
	std::cout << "Total Seconds: " << time << std::endl;

	{
		constexpr double BYTES_PER_ITERATION = 16.0;
		constexpr long long GIGABYTE = 1 << 30;
		std::cout << "GBps: " << (ITERATIONS * BYTES_PER_ITERATION) / (GIGABYTE * time) << std::endl;
		std::cout << "Dummy Benchmark anti-optimizer print: " << total64[0] << std::endl;
	}
#endif
}

#include <io.h>  
#include <fcntl.h>  
#include <stdio.h>

int main()
{
	// This #if 1 defaults to running the GBps test code.
	// Set #if to 0 to run the generator code, which can serve as input to PractRand
#if 1
	timerTest();
	timerTestOthers();
	timerTestPlusOne();
	return 0;
#else
	// Use this code and pipe it to a RNG-statistical test, such as PractRand or TestU01
	__m128i output[16384];
	__m128i rngState = _mm_setzero_si128();

	_setmode(_fileno(stdout), _O_BINARY);

	while (!ferror(stdout)) {
		for (int i = 0; i < 16384; i += 2) {
			// Two versions: One to test AESRand_increment. A second to test AESRand_parallelStream
#define TEST_INCREMENT
#ifdef TEST_INCREMENT
			AESRand_increment(rngState);
			auto randVals = AESRand_rand(rngState);
			output[i + 0] = randVals[0];
			output[i + 1] = randVals[1];
#else
			// This is far slower than AESRand_increment, but it should be statistcally valid
			// and therefore deserves a test
			rngState = AESRand_parallelStream(rngState);
			auto randVals = AESRand_rand(rngState);
			output[i + 0] = randVals[0];
			output[i + 1] = randVals[1];
#endif
		}

		fwrite(output, 1, sizeof(output), stdout);
	}
#endif
}