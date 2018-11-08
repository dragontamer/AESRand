I redid the test while locking my CPU to 3.4 GHz (AMD Zen options: P0 state 3.4 GHz, P1+ was disabled, Zen Core Performance Boost off). I manually verified in Ryzen Master that the CPU remained at 3.4GHz during the test.

Pattern for calculating cycles per iteration: (number of seconds) * 3.4GHz clock speed / (number of iterations). Number of iterations is either 10-billion, or 5-billion, depending on how the specific benchmark worked.

* Single-State AESRand: 3.69 cycles per iteration (32-bytes)
* Double-State AESRand: 2.95 cycles per iteration (32-bytes)
* mt19937: 14.76 cycles per iteration (4-bytes)
* pcg32 (Unrolled): 5.46 cycles per iteration (4-bytes)
* xoshiro256plus (Unrolled): 3.32 cycles per iteration (8-bytes)
* PlusOne XMM-registers: 1.59 cycles per iteration (Dummy Control, like BogoMIPS)

The "overhead" of the AESRand benchmark are:
1. The "For" loop: one-add per iteration (i++), and the cmp/jnz instruction (i<=ITERATIONS). Unrolling reduced this overhead, but modern CPUs are good at executing the loop-logic in parallel, which mitigates this overhead.
2. Two SIMD-adds in Single-State AESRand for the "Dummy Print".
3. Four SIMD-adds for Double-state AESRand
4. One 32-bit add for mt19937
5. One 32-bit add for pcg32
6. One 64-bit add for xoshiro256plus

Raw Results (AMD Threadripper 1950x locked to 3.4GHz)
===========

Beginning Single-state 'serial' test
Total Seconds: 5.4278
GBps: 27.4534
Dummy Benchmark anti-optimizer print: 1706011378085583560
Beginning Parallel (2x) test: instruction-level parallelism
Time: 8.67641
GBps: 34.3487
Dummy Benchmark anti-optimizer print: 1283732354369314394

Testing mt19937
Time: 21.7154
GBps: 0.857752
Dummy Benchmark anti-optimizer print: 1680273558

Testing pcg32 Unrolled x4
Time: 8.0232
GBps: 2.32157
Dummy Benchmark anti-optimizer print: 2362602604

Testing pcg32
Time: 8.22974
GBps: 2.26331
Dummy Benchmark anti-optimizer print: 757965796

Testing xoshiro256plus Unrolled x4
Time: 4.88474
GBps: 7.62639
Dummy Benchmark anti-optimizer print: 2202972135473059297

Testing xoshiro256plus
Time: 5.09052
GBps: 7.31809
Dummy Benchmark anti-optimizer print: 5290432412060736627

Beginning PlusOne XMM-registers Test
Total Seconds: 2.3376
GBps: 63.7456
Dummy Benchmark anti-optimizer print: 6553255931290448384