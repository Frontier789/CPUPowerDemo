# Power of CPUs

## My CPU
- Ryzen 9 5950x 
- 16 cores
- 3.4GHz
- AVX2 (256-bit)
- Cache 
    - L1: 64K (per core)
    - L2: 512K (per core)
    - L3: 64MB


Zen 3 architecture, FPU mostly unchanged from [Zen 2](https://en.wikichip.org/wiki/amd/microarchitectures/zen_2)

( 2 AVX-256 instructions ) * ( 3.4GHZ frequency ) * ( 16 cores ) * ( 8 SP float / AVX-256 instruction ) = 870 GFLOPS

Raw image decoder:
2.1ms for 5M pixels