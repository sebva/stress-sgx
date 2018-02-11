# Stress-SGX

The latest generation of Intel processors supports [Software Guard Extensions (SGX)](https://software.intel.com/en-us/sgx), a set of instructions that implements a Trusted Execution Environment (TEE) right inside the CPU, by means of so-called enclaves.
We present Stress-SGX, an easy-to-use stress-test tool to evaluate the performance of SGX-enabled nodes.
We build on top of the popular [stress-ng](http://kernel.ubuntu.com/~cking/stress-ng/) tool, while only keeping the workload injectors (stressors) that are meaningful in the SGX context.

## Requirements

Because Stress-SGX is based on stress-ng, the same libraries need to be installed.
They are:

* libaio-dev
* libapparmor-dev
* libattr1-dev
* libbsd-dev
* libcap-dev
* libgcrypt11-dev
* libkeyutils-dev
* libsctp-dev
* zlib1g-dev

Moreover, the [official Intel SGX SDK for Linux](https://01.org/intel-softwareguard-extensions/downloads/intel-sgx-linux-2.0-release) needs to be installed in `/opt/intel/sgxsdk`.
We support version 2.0 of the SDK.

To run in hardware enclaves, the SGX driver and Platform Software (PSW) also need to be installed on the system.

## How to build

Building Stress-SGX is based on Makefiles.
Compiling an optimized, SGX-enabled release can be done using the following command:

```bash
make SGX_DEBUG=0 SGX_PRERELEASE=1 SGX_MODE=HW
```

## Running SGX stressors

SGX stressors are selected in the same way than normal _stress-ng_ stressors.
We support the following stress methods in SGX:
```
ackermann bitops callfunc correlate crc16 dither djb2a double euler explog fft factorial fibonacci float fnv1a gamma gcd gray hamming hanoi hyperbolic idct int64 int32 int16 int8 int64float int64double int64longdouble int32float int32double int32longdouble jenkin jmp ln2 longdouble loop matrixprod nsqrt ocall omega parity phi pi pjw prime psi queens rand rgb sdbm stats sqrt trig union zeta
```

The following command-line options can be selected in addition to the standard ones from _stress-ng_:

```
--sgx N         start N SGX enclaves
--sgx-ops N     stop after N sgx cpu bogo operations
--sgx-method M  specify stress sgx method M, default is all
```

## Ensuring byte-per-byte equivalence of native and SGX code

An enclave compiled using the official SGX SDK will produce a statically-compiled shared library.
It is possible to leverage this aspect to guarantee that both native and SGX versions of a stressor execute a perfectly identical binary by dynamically linking this shared object.
This optional approach, however, limits Stress-SGX to stressors that are only available in enclave mode.

Selecting the approach is done by compiling the version of Stress-SGX located in the `loadso` branch.

