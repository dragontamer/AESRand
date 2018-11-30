# AESRand
A Prototype implementation of Pseudo-RNG based on hardware-accelerated AES instructions and 128-bit SIMD

TL;DR
--------
* State: 128 bits (One XMM register)
* 256-bits / 32-bytes generated per iteration
* Incredible speed: roughly 3.7 CPU cycles per iteration
* Cycle Length: 2^64
* Independent Streams: 2^64
* Core RNG passes 8TB+ tests. Parallel-stream generator fails at 8TB of PractRand (passes 4TB)
* Tested: ~29.2 GBps (Gigabytes per second) single-thread / single-core. 
* Dual-stream version achieves 37.1 GBps
* A throughput of ~8.5 Bytes per cycle. Or roughly 3.73 cycles per 256-bit iteration.
* Faster than xoshiro256plus, pcg32, and std::mt19937

The shortest sample code is in the [Simplified Linux Version](AESRand_Linux/AESRand.cpp).
Commentary is provided in this README, as well as through many comments in the Windows version.
The Windows version contains a self-included benchmark to compare against the speed of 
xoshiro256, pcg32, and std::mt19937.

Design Principles
-------

1. AESRound-based. Every x86 CPUs since Intel Westmere (2010) and AMD Bulldozer (2011) can execute
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
a "mixer" (which was a simple shift add xor hash-function). AESRand_increment serves as
the "counter", while AESRand_rand serves as the "mixer".

4. Minimum latency on the "Counter" -- The latency of the counter-portion of this RNG
(AESRand_increment) is the absolute limit to the speed of any RNG. If it takes 5-cycles to
update the state, your RNG will take 5-cycles (or more) per iteration. I've minimized
the latency of AESRand_increment to 1-cycle, the absolute minimum latency.

5. Instruction level parallelism (ILP) -- All instructions of the "mixer" portion of the RNG
(AESRand_rand) have a throughput of 1-per-cycle or more. AMD Zen can execute two AES
instructions per clock (and thus has a throughput of 2-per-cycle!!). Notice the 
signature of AESRand_rand(const \__m128i state). The state MUST be a constant to take
advantage of ILP. Aside from the counter-latency, each iteration i can execute in parallel
with future iterations i+1, i+2, i+3, etc. etc. Modern CPUs are incredibly good at capturing 
this parallelism and internally pipelining the AES-instructions of the mixer. ILP allows you
to beat the latency-characteristics of your instructions. For example, every iteration
has a latency of 4 cycles per AESENC, or 8-cycles of latency total. However, I've tested
3.7 cycles per iteration. The magic of ILP makes this possible. 

6. Full invertibility -- http://www.burtleburtle.net/bob/hash/doobs.html The JOAAT hash has a concept
of a "bit funnel", which is a BAD thing for hashes. If you provably have full-invertibility, it means you
never lose information. Its kind of a hard concept to describe, but it is fundamental to the design
of RNGs, Cryptography, and so forth. The entirety of GF(2) fields are all based around
the concept of invertible operations. The XOR, Add, and AES-encode instructions all have inverts
(XOR, Subtract, and AES-decode respectively), and therefore have the greatest chance of passing
statistical tests... as long as the bits are "shuffled" enough.

Benchmark Results
--------

[Click here](BenchmarkResults.md) for the latest benchmark results.

This is a very simple timer-based benchmark, where I simply run the various RNGs to be tested
(AESRand, mt19937, pcg32, and xoshiro256plus) in a tight loop of 5-billion iterations. To ensure that
the optimizer does NOT remove the RNG code, I have a "total" value that adds up every output
of the RNG, and eventually prints it out to the screen.

Before and after the 5-billion long loop, I run Window's 'QueryPerformanceCounter" to log the time.

I checked the generated assembly (After building in VS2017, check the "AESRand.cod" file).
The "mt19937" code was NOT inlined. Which may be a disadvantage, and why its so much slower than
the other RNGs.

PCG32 and xoshiro256plus were both inlined well. I wasn't sure how well they'd adapt to ILP, so I
created a 4x manually unrolled version for the both of them. The unrolled versions don't seem to be
faster or slower. I admit that I haven't used those RNGs before, so I'm not entirely sure if I've
set up their ideal conditions.


Weaknesses and Future Work
----------------
AESRand is surprisngly BAD at 1-bit changes. If I changed the increment to a single-bit change
like [0x1, 0, 0, 0, ...], it would take 4, maybe 5 aesenc instructions before the code could get
above 8GB of tests in PractRand.

I experimented with various other reversible functions documented on Lemire's blog
https://lemire.me/blog/2016/08/09/how-many-reversible-integer-operations-do-you-know/. XOR, Adds,
bitshifts, multiplies-with-odd numbers, and more are all interesting, but the AES-instructions
seemed to mix bits better than any of the primitive instructions.

The one instruction that holds a lot of promise is PCLMULQDQ (Carry-less Multiply). This is a
64-bit x 64-bit polynomial multiply on 128-bit XMM registers. Roughly 3 or 4 PCLMULQDQ, along
with some bitshifts and XORs, could implement the 128-bit carryless multiply used in GCM 
(galois counter mode). And this seems to be a very good way to "disperse bits" and create
an avalanche-effect.

Furthermore, 64-bit carryless multiply is implemented on x86 (PCLMULQDQ), ARMv8 (PMULL and PMULL2
on ARM64, VMULL on ARM32), and Power9 (vpmsumh: Vector Polynomial Multiply-Sum). These instructions serve
as the basis for GCM-mode, Eliptical Curve Cryptography, and other important developments in the modern
cipher world. I expect all future CPUs to have carryless-multiply implemented due to their importance
to the cryptography community.

However, my Threadripper 1950x appears to run the PCLMULQDQ instruction as microcode, and thus it only has
a throughput of one-PCLMULQDQ every TWO instructions (4x less throughput than AESenc). In effect, running
aesenc 4x in a row has more throughput, on my machine at least. Intel machines are documented to run 
PCLMULQDQ per cycle, and thus PCLMULQDQ may be a faster base to use on Intel machines. Further investigation 
into the relative speeds of these cryptography instructions, across the different modern CPUs could be important.

aesenc has 4 steps: SubBytes, ShiftRows, MixColumns, and XOR Round Key. SubBytes is absolutely excellent for
RNG work. ShiftRows is useful, but only with multiple AES-instructions in a row. MixColumns is unfortunately 
only a 32-bit operation, albeit parallel across 4-different 32-bit values. Still, a single aesenc or aesdec 
disperses bits across 32-bits of the state. After two rounds of AES, any particular input bit only 
affects half of the bits: 64-bits per 128-bit XMM register (or a total of ~128-bits of the 256-bit output)

So 2-rounds of AES is NOT sufficient to have a proper avalanche (defined as a 50% chance to flip any bit of 
the output). I get around the severe 1-bit weakness by ensuring that all 128-bits of state changes on every
iteration.

The "Parallel Stream" generator only changes the top 64-bits of the input. This is the "weak direction" of the
random number generator, which fails after 8TB of testing in PractRand. Nonetheless, the ability to support
parallel streams is important in today's world of highly-parallelized simulations. Passing 4TB of PractRand
means that 34-Billion parallel streams were created, and PractRand was unable to detect
any statistical correlation between their start points. So at least 2^35 high-quality parallel streams are 
available to use.

Thanks and Notes
------------
The core algorithm is based on pcg32, documented here: http://www.pcg-random.org/. The idea to 
split "counter" with "mixer" is an incredibly effective design on modern machines with large amounts of
instruction-level parallelism.

The theory of hashing by Bob Jenkins is what most made me "get" cipher design. Bob Jenkin's
page is absolutely excellent, and his "theory of funnels" put me on the right track. 
http://www.burtleburtle.net/bob/hash/doobs.html

Daniel Lemire's blog is filled to the brim with SIMD tips and tricks. His article here also documents
MANY reversible functions. While none of these reversible operations ended up in this implementation,
the page served as a valuable reference in my experiments. 
https://lemire.me/blog/2016/08/09/how-many-reversible-integer-operations-do-you-know/

Donald Knuth's "The Art of Computer Programming", volume 2, serves as a great introduction to the
overall theory of RNGs.

PractRand: http://pracrand.sourceforge.net/ for making an incredibly awesome RNG-testing utility that 
actually works on Windows (and works easily!).

Agner Fog's instruction tables: I was constantly referencing Agner Fog's latency and throughput tables 
throughout the coding of this RNG: https://www.agner.org/optimize/

PractRand Results
------------

[Click here](PractRand.md) for PractRand results.


BigCrush Results
------------

AESRand_Linux contains two BigCrush tests, which require TestU01 in order to be run. The "primary" AESRand generator passes BigCrush through multiple means: reversed bits, forward bits and so forth. TestU01 is limited to 32-bit tests, so it is a bit odd to try to adapt a 256-bit generator like AESRand to TestU01's interface.
