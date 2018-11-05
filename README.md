# AESRand
Random Number Generator based on hardware-accelerated AES instructions

Using the principles behind http://www.pcg-random.org, I decided to design my own 
pseudo-RNG. Here were my ideas:

1. AESRound-based. Every x86 CPU that can execute the "aesenc" instruction has a 
throughput of doing so once every cycle, or faster. AMD Ryzen for example can execute
TWO "aesenc" instructions per cycle, with a latency of 4-cycles. This makes
aesenc even faster than 64-bit multiply. The best alternative I can think of
is the SIMD instruction PMADDWD (multiply-and-add, SIMD across 32-bits, producing
64-bit results), which executes in 5-cycle latency and two-per-cycle throughput.
on Intel Skylake, but I bet that AESRound produces higher-quality random numbers
than a simple multiply/add (LCGRNG)

2. SIMD-acceleration -- Modern computers are 128-bit, 256-bit, or even 512-bit machines.
Because AES is only defined for 128-bits, I stick with 128-bit. The hypothetical SHA
instructions have multiple disadvantages compared to AES and are only 128-bit anyway.

3. PCG-random.org "Simple counter" + "Mixer" design -- PCG-random.org has a two-step
RNG process. The "counter" (which was a multiply-based LCGRNG in the pcg32_random_r code), and
a "mixer" (which was a simple shift add xor hash-function). My design is instead a 
relatively simple 128-bit counter through SIMD-instructions, and an AESRound to mix the bits
around.

4. Instruction level parallelism (ILP) -- Modern processors can execute instructions in parallel
if they are NOT dependent on each other. The "Simple Counter + Mixer" design from pcg-random.org
lends itself very well to ILP: have the mixer entirely independent of the counter.

5. Optimize Simple Counter's latency -- ILP can only run as fast as your shortest dependency chain.
In this case, the "simple counter" portion has a latency of 4-cycles (cmpeq, shift-left, blendvb, and add).
_mm_setzero_si128() is 0-cycles on modern machine (decoder-bottlenecked). Is there a faster
way to implement a 128-bit counter??

6. Note: If a 64-bit state is all that is necessary, using the "add" instruction without
cmpeq, shift-left, and blendvb will grossly improve speed.

4. Absolute minimum "latency" on the counter h

4. Instruction level parallelism -- Prefer designs with "short" dependency chains. Notice
that AESRand_randA's two aesenc instructions can be executed in parallel (ie: pipelined in Skylake,
and literally in parallel on AMD Zen). They aren't dependent on each other's results. "chaining"
AES rounds together would lead to a higher-quality RNG for sure, but instruction-level parallelism
results in a far, far faster design.

AESRand_randA and AESRand_randB both execute in parallel, to return two 16-byte values (32-bytes worked
on parallel). This is because the "increment" code takes far longer to execute, due to the dependency
chain innate to the counter design. To allow a large number of independent random numbers from the same
seeds, I run aesdec (decode) in randB, while I run aesenc (encode) in randA.

5. Full invertibility -- http://www.burtleburtle.net/bob/hash/doobs.html The JOAAT hash has a concept
of a "bit funnel", which is a BAD thing for hashes. If you provably have full-invertibility, it means you
never lose information. Its kind of a hard concept to describe, so see Bob Jenkins's webpage. Long story
short: if you can prove that your design is invertible, you've maximized the "entropy" of your hash mixer.

Invertibility is easily proven with AES-rounds: the decryption function perfectly inverts any operation I do.
The other functions I use (XOR, Add, and Subtract) are all trivially invertible as well, proving that I have
absolute invertibility on every aspect of my RNG.

6. Counter-based design is friendly to parallelism -- Different threads can easily have "slices" of the RNG
to themselves, by setting the top 64-bits of the RNG to their own value. Each thread therefore gets a different
64-bit value for themselves.

-------

Results:

On my AMD Threadripper 1950x computer, I achieve:

Time: 17.2887
GBps: 8.61904

5-billion iterations, 16-bytes (from AESRand_randA), + 16 bytes (from AESRand_randB), over 17-seconds
is roughly 8.6GB/s. This seems to be on the same order as LCGRNGs.

Cycle-length is easily proven to be a full 128-bits. The "counter" repeats after 2^128 iterations (every 2^64
iterations, the top-64 bits get +1 added to them. The bottom 64-bits have a full cycle length of 2^64 due
to the "increment" being relatively prime to 2^64).

I'll have to test to see if this passes RNG-testers later. But I'm feeling excited about this.

-----------

Notes:

1. The "Increment" 0x110D0B0705030201 -- While +1 would have been the simplest design, AES is surprisngly
badd at diffusion if you only give it a single round! (A cryptographically-secure AES is 8-rounds after all).
These are the first 7 prime numbers, followed by +1 to maximize the cycle-length. The bottom 64-bits will cycle 
through all possibilities before repeating, because the +1 at the end is relatively-prime to 2^64. As such,
this increment provides me a 128-bit cycle.

2. Any BYTE that changes (even by 1 bit) will dramatically look different due to AES's "SubBytes" step. However,
greater "diffusion" of bits is primarily through the "MixColumns" step. AES's "ShiftRows" is great when
combined with MixColumns, but since this code only ever does a single "layer" of AES, the ShiftRows step
never really gets to shine. 

3. MixColumns isn't very good at diffusion with a single round. So to get a proper avalanche effect (avalanche:
a single-bit change in the input has a 50% chance of changing ALL bits of the output), I need the 2nd
AES in there. Chaining AES together is out of the question (gotta keep the dependency chain at a minimum), so 
I hope that a parallel AES round would be sufficient.

3. The "obvious" way to diffuse better would be to AES-chain together. But this increases the dependency chain
and slows down the code severely. Keeping with a "parallel" AES and hoping that the two (when combined) do
a proper avalanche effect is my hope.

4. The magic value 0x5a5a5a5a and 0xa5a5a5a5 are the values 1010101010 in binary, and 0101010101 in binary.
They're the magical "forced diffusion" variables I use whenever I need to mix up the bits somehow. I call
this value the "key", as it is most commonly used as the AES-key argument (which is actually a stupid XOR).

5. "Static" variables seem to cause a "call" to be generated to some initialization code. Don't use static variables
in highly optimized code.

https://lemire.me/blog/2017/08/22/testing-non-cryptographic-random-number-generators-my-results/