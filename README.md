# AESRand
A Prototype implementation of Pseudo-RNG based on hardware-accelerated AES instructions and 128-bit SIMD

TL;DR
--------
* State: 128 bits (One XMM register of state)
* 256-bits per iteration
* Cycle Length: 2^64
* Independent Streams: 2^64
* Passes PractRand 1TB tests and beyond.
* Tested: 29.2 GBps (Gigabytes per second) single-thread / single-core. 
* A throughput of ~8.5 Bytes per cycle. Or roughly 3.73 cycles per 256-bit iteration.
* Faster than xoshiro256plus, pcg32, and std::mt19937

Introduction
-------
Using the principles behind http://www.pcg-random.org, I decided to design my own 
pseudo-RNG. Here were my ideas:

1. AESRound-based. Every x86 CPUs for the past 5, maybe 10, years can execute
not only a singular "aesenc" instruction (AES Encode Round)... but they can also
execute them incredibly quickly: at least one per cycle. AMD Ryzen / EPYC CPUs 
can even execute them TWICE per cycle if they are independent. With a latency
of roughly 4-cycles on modern Intel Skylake CPUs, the 128-bit AES-encode 
instruction is faster than a 64-bit multiply.

	Note: AESRound is implemented on all major CPUs of the modern (Nov 2018) era.
	Power9 has the vcipher instruction, which seems to be identical to the x86 aesenc 
	instruction. ARM unfortunately plays a bit differently, but a sequence of AESE, 
	AESMC, and XOR would replecate the x86 "aesenc" instruction.

	AESD, AESIMC, and XOR would together be equivalent to an x86 "aesdec" instruction.
	It is time to take advantage of the universal AES-hardware instructions
	embedded in all of our CPUs, even our Cell Phones can do this in 2018.

2. SIMD-acceleration -- Modern computers are 128-bit, 256-bit, or even 512-bit machines.
Because AES is only defined for 128-bits, I stick with 128-bit. Power9 and ARM machines
also support 128-bit SIMD easily. Future CPUs will probably be more SIMD-heavy. If anyone
can think of how to extend this concept out to 256-bit (YMM) registers and beyond, they
probably can beat the results I have here!

3. PCG-random.org "Simple counter" + "Mixer" design -- PCG-random.org has a two-step
RNG process. The "counter" (which was a multiply-based LCGRNG in the pcg32_random_r code), and
a "mixer" (which was a simple shift add xor hash-function). My design is around a 64-bit
counter and a 128-bit mixer, which outputs 256-bits of output each iteration through 
three AES instructions.

4. Instruction level parallelism (ILP) -- Modern processors can execute instructions in parallel
if they are NOT dependent on each other. The "Simple Counter + Mixer" design from pcg-random.org
lends itself very well to ILP. I designed the AESRand "counter" to have the absolute minimum
1-cycle of latency per iteration. While the "mixer" executes in parallel: all XMM / 128-bit SIMD
instructions I use have a throughput of 1-per-cycle or faster. In effect, this AES-RNG code is designed
to theoretically pump out a 256-bit result every cycle (A hypothetical future CPU could
feasibly run this code steady-state at 256-bits per-cycle)

5. Full invertibility -- http://www.burtleburtle.net/bob/hash/doobs.html The JOAAT hash has a concept
of a "bit funnel", which is a BAD thing for hashes. If you provably have full-invertibility, it means you
never lose information. Its kind of a hard concept to describe, but it is fundamental to the design
of RNGs, Cryptography, and so forth. The entirety of GF(2) fields and whatnot are all based around
the concept of invertible operations. The XOR, Add, and AES-encode instructions all have inverts
(XOR, Subtract, and AES-decode respectively), and therefore have the greatest chance of passing
statistical tests.

Benchmark Results
--------

[Click here](BenchmarkResults.md) for the latest benchmark results.

This is a very simple timer-based benchmark, where I simply run the various RNGs to be tested
(AESRand, mt19937, pcg32, and xoshiro256plus) in a tight loop of 5-billion iterations. To ensure that
the optimizer does NOT remove the RNG code, I have a "total" value that adds up every output
of the RNG, and eventually prints it out to the screen.

Before and after the 5-billion long loop, I run Window's 'QueryPerformanceCounter" to check
the time before and afterwards. The clock typically ticks at 3MHz (every 333 nanoseconds or so),
which should be accurate enough for tests of length 5-seconds or longer. 

I checked the generated assembly (After building in VS2017, check the "AESRand.cod" file).
The "mt19937" code was NOT inlined. Which may be a disadvantage, and why its so much slower than
the other RNGs.

PCG32 and xoshiro256plus were both inlined well. I wasn't sure how well they'd adapt to ILP, so I
created a 4x manually unrolled version for the both of them. The unrolled versions don't seem to be
faster or slower.


Weaknesses
----------------
This RNG is surprisngly BAD at 1-bit changes. It would take 4, maybe 5 aesenc instructions
in a row before I could get above 8GB of tests in PracRand, and the code was just multiples slower at that point.
I've been investigating ways to make the generator better with 1-bit changes: the PCLMULQDQ (Carry-less Multiply)
instruction which is the basis of the 128-bit multiply in GCM seems very promising. Carry-less multiply is 
implemented in x86, ARM, and Power9 as well.

However, my Threadripper 1950x appears to run the PCLMULQDQ instruction as microcode, and thus it only has
a throughput of one-PCLMULQDQ every TWO instructions (4x less throughput than AESenc). In effect, running
aesenc 5x in a row is faster, on my machine at least. (Intel machines are documented to run PCLMULQDQ per cycle,
and thus PCLMULQDQ may be a faster base to use on Intel machines)

aesenc has 4 steps: SubBytes, ShiftRows, MixColumns, and XOR Round Key. SubBytes is absolutely excellent for
RNG work. ShiftRows is useful, but only with multiple AES-instructions in a row. MixColumns is unfortunately only a
32-bit operation, and thus only disperses bits across 32-bits inside of the state. Multiple rounds are needed
to disperse bits further.

Thanks and Notes
------------

I stand on the shoulders of giants.

The core algorithm is based on pcg32, documented here: http://www.pcg-random.org/. The idea to 
split "counter" with "mixer" is an incredibly effective design on modern machines with large amounts of
instruction-level parallelism.

The theory of hashing by Bob Jenkins is what most made me "get" cipher design. Bob Jenkin's
page is absolutely excellent, and his "theory of funnels" put me on the right track. http://www.burtleburtle.net/bob/hash/doobs.html

Daniel Lemire's blog is filled to the brim with SIMD tips and tricks. His article here also documents
MANY reversible functions. While none of these reversible operations ended up in this implementation,
the page served as a valuable reference in my experiments. https://lemire.me/blog/2016/08/09/how-many-reversible-integer-operations-do-you-know/

Donald Knuth's "The Art of Computer Programming", volume 2, serves as a great introduction to the
overall theory of RNGs.

PractRand: http://pracrand.sourceforge.net/ for making an incredibly awesome RNG-testing utility that actually works
on Windows (and works easily!).

Agner Fog's instruction tables: I was constantly referencing Agner Fog's latency and throughput tables 
throughout the coding of this RNG: https://www.agner.org/optimize/

PractRand Results
------------

[Click here](PractRand.md) for PractRand results.