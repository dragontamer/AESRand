Initial Results from PractRand

Preliminary PractRand Results on AESRand_increment
------------

RNG_test using PractRand version 0.94
RNG = RNG_stdin, seed = unknown
test set = core, folding = standard(unknown format)

rng=RNG_stdin, seed=unknown
length= 256 megabytes (2^28 bytes), time= 2.1 seconds
  no anomalies in 213 test result(s)

rng=RNG_stdin, seed=unknown
length= 512 megabytes (2^29 bytes), time= 4.1 seconds
  no anomalies in 229 test result(s)

rng=RNG_stdin, seed=unknown
length= 1 gigabyte (2^30 bytes), time= 7.5 seconds
  no anomalies in 248 test result(s)

rng=RNG_stdin, seed=unknown
length= 2 gigabytes (2^31 bytes), time= 14.2 seconds
  no anomalies in 266 test result(s)

rng=RNG_stdin, seed=unknown
length= 4 gigabytes (2^32 bytes), time= 28.0 seconds
  no anomalies in 282 test result(s)

rng=RNG_stdin, seed=unknown
length= 8 gigabytes (2^33 bytes), time= 53.5 seconds
  no anomalies in 299 test result(s)

rng=RNG_stdin, seed=unknown
length= 16 gigabytes (2^34 bytes), time= 108 seconds
  no anomalies in 315 test result(s)

rng=RNG_stdin, seed=unknown
length= 32 gigabytes (2^35 bytes), time= 208 seconds
  no anomalies in 328 test result(s)

rng=RNG_stdin, seed=unknown
length= 64 gigabytes (2^36 bytes), time= 437 seconds
  no anomalies in 344 test result(s)

rng=RNG_stdin, seed=unknown
length= 128 gigabytes (2^37 bytes), time= 844 seconds
  no anomalies in 359 test result(s)

rng=RNG_stdin, seed=unknown
length= 256 gigabytes (2^38 bytes), time= 1620 seconds
  no anomalies in 372 test result(s)

rng=RNG_stdin, seed=unknown
length= 512 gigabytes (2^39 bytes), time= 3484 seconds
  no anomalies in 387 test result(s)

rng=RNG_stdin, seed=unknown
length= 1 terabyte (2^40 bytes), time= 7024 seconds
  no anomalies in 401 test result(s)

rng=RNG_stdin, seed=unknown
length= 2 terabytes (2^41 bytes), time= 13255 seconds
  no anomalies in 413 test result(s)

rng=RNG_stdin, seed=unknown
length= 4 terabytes (2^42 bytes), time= 27845 seconds
  no anomalies in 426 test result(s)

rng=RNG_stdin, seed=unknown
length= 8 terabytes (2^43 bytes), time= 56894 seconds
  no anomalies in 438 test result(s)

Preliminary PractRand Results on AESRand_parallelStream "Plus Pi"
-------------------------------------------------------

This version uses:

    __m128i AESRand_parallelStream(__m128i originalStream) {
        __m128i copy = originalStream;
        copy.m128i_u64[1] += 0x3141592653589793; 
        return copy;
    }

One "unusual" result at 4GB of test, but not unusual enough
to fail PractRand's default settings. 

RNG_test using PractRand version 0.94
RNG = RNG_stdin, seed = unknown
test set = core, folding = standard(unknown format)

rng=RNG_stdin, seed=unknown
length= 256 megabytes (2^28 bytes), time= 2.3 seconds
  no anomalies in 213 test result(s)

rng=RNG_stdin, seed=unknown
length= 512 megabytes (2^29 bytes), time= 4.4 seconds
  no anomalies in 229 test result(s)

rng=RNG_stdin, seed=unknown
length= 1 gigabyte (2^30 bytes), time= 8.0 seconds
  no anomalies in 248 test result(s)

rng=RNG_stdin, seed=unknown
length= 2 gigabytes (2^31 bytes), time= 15.0 seconds
  no anomalies in 266 test result(s)

rng=RNG_stdin, seed=unknown
length= 4 gigabytes (2^32 bytes), time= 28.3 seconds
  Test Name                         Raw       Processed     Evaluation
  BCFN(2+2,13-0,T)                  R=  +8.2  p =  6.6e-4   unusual
  ...and 281 test result(s) without anomalies

rng=RNG_stdin, seed=unknown
length= 8 gigabytes (2^33 bytes), time= 57.6 seconds
  no anomalies in 299 test result(s)

rng=RNG_stdin, seed=unknown
length= 16 gigabytes (2^34 bytes), time= 117 seconds
  no anomalies in 315 test result(s)

rng=RNG_stdin, seed=unknown
length= 32 gigabytes (2^35 bytes), time= 220 seconds
  no anomalies in 328 test result(s)

rng=RNG_stdin, seed=unknown
length= 64 gigabytes (2^36 bytes), time= 461 seconds
  no anomalies in 344 test result(s)

rng=RNG_stdin, seed=unknown
length= 128 gigabytes (2^37 bytes), time= 900 seconds
  no anomalies in 359 test result(s)

rng=RNG_stdin, seed=unknown
length= 256 gigabytes (2^38 bytes), time= 1733 seconds
  no anomalies in 372 test result(s)

rng=RNG_stdin, seed=unknown
length= 512 gigabytes (2^39 bytes), time= 3646 seconds
  no anomalies in 387 test result(s)

rng=RNG_stdin, seed=unknown
length= 1 terabyte (2^40 bytes), time= 7479 seconds
  no anomalies in 401 test result(s)

rng=RNG_stdin, seed=unknown
length= 2 terabytes (2^41 bytes), time= 14248 seconds
  no anomalies in 413 test result(s)

rng=RNG_stdin, seed=unknown
length= 4 terabytes (2^42 bytes), time= 29950 seconds
  no anomalies in 426 test result(s)

rng=RNG_stdin, seed=unknown
length= 8 terabytes (2^43 bytes), time= 60801 seconds
  Test Name                         Raw       Processed     Evaluation
  BRank(12):64K(1)                  R= +1078  p~=  1.1e-325   FAIL !!!!!!
  ...and 437 test result(s) without anomalies
 
Preliminary PractRand Results on AESRand_parallelStream Knuth LCGRNG
----------------

 RNG_test using PractRand version 0.94
RNG = RNG_stdin, seed = unknown
test set = core, folding = standard(unknown format)

rng=RNG_stdin, seed=unknown
length= 256 megabytes (2^28 bytes), time= 2.1 seconds
  no anomalies in 213 test result(s)

rng=RNG_stdin, seed=unknown
length= 512 megabytes (2^29 bytes), time= 3.9 seconds
  no anomalies in 229 test result(s)

rng=RNG_stdin, seed=unknown
length= 1 gigabyte (2^30 bytes), time= 7.2 seconds
  no anomalies in 248 test result(s)

rng=RNG_stdin, seed=unknown
length= 2 gigabytes (2^31 bytes), time= 13.6 seconds
  no anomalies in 266 test result(s)

rng=RNG_stdin, seed=unknown
length= 4 gigabytes (2^32 bytes), time= 25.4 seconds
  no anomalies in 282 test result(s)

rng=RNG_stdin, seed=unknown
length= 8 gigabytes (2^33 bytes), time= 52.4 seconds
  no anomalies in 299 test result(s)

rng=RNG_stdin, seed=unknown
length= 16 gigabytes (2^34 bytes), time= 109 seconds
  no anomalies in 315 test result(s)

rng=RNG_stdin, seed=unknown
length= 32 gigabytes (2^35 bytes), time= 210 seconds
  no anomalies in 328 test result(s)

rng=RNG_stdin, seed=unknown
length= 64 gigabytes (2^36 bytes), time= 439 seconds
  no anomalies in 344 test result(s)

rng=RNG_stdin, seed=unknown
length= 128 gigabytes (2^37 bytes), time= 861 seconds
  no anomalies in 359 test result(s)

rng=RNG_stdin, seed=unknown
length= 256 gigabytes (2^38 bytes), time= 1637 seconds
  no anomalies in 372 test result(s)

rng=RNG_stdin, seed=unknown
length= 512 gigabytes (2^39 bytes), time= 3481 seconds
  no anomalies in 387 test result(s)

rng=RNG_stdin, seed=unknown
length= 1 terabyte (2^40 bytes), time= 7016 seconds
  no anomalies in 401 test result(s)

rng=RNG_stdin, seed=unknown
length= 2 terabytes (2^41 bytes), time= 13248 seconds
  no anomalies in 413 test result(s)

rng=RNG_stdin, seed=unknown
length= 4 terabytes (2^42 bytes), time= 27881 seconds
  no anomalies in 426 test result(s)

rng=RNG_stdin, seed=unknown
length= 8 terabytes (2^43 bytes), time= 56968 seconds
  Test Name                         Raw       Processed     Evaluation
  BRank(12):64K(1)                  R= +1078  p~=  1.1e-325   FAIL !!!!!!
  ...and 437 test result(s) without anomalies
