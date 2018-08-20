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

### CPU stressors

SGX CPU stressors are selected in the same way than normal _stress-ng_ CPU stressors.
We support the following stress methods in SGX:
```
ackermann bitops callfunc correlate crc16 dither djb2a double euler explog fft factorial fibonacci float fnv1a gamma gcd gray hamming hanoi hyperbolic idct int64 int32 int16 int8 int64float int64double int64longdouble int32float int32double int32longdouble jenkin jmp ln2 longdouble loop matrixprod nsqrt ocall omega parity phi pi pjw prime psi queens rand rgb sdbm stats sqrt trig union zeta
```

The following command-line options are used to configure an SGX CPU stress run:

```
--sgx N         start N SGX enclaves
--sgx-ops N     stop after N sgx cpu bogo operations
--sgx-method M  specify stress sgx method M, default is all
```

### EPC stressors

We also support stressing SGX trusted memory using the _vm_ stressors from _stress-ng_.
The following options configure how to stress the EPC.
EPC allocation is static, and is configured in `sgx/enclave_vm/trusted/vm.config.xml`.
This initial allocation only affects the maximal amount of memory that can be stressed, as well as startup times.
The actual amount of memory under stress is defined using `--sgx-vm-bytes`.
A key difference compared to _stress-ng_ is that we allocate memory using the _TCMalloc_ implementation shipped with the Intel SGX SDK, while _stress-ng_ uses the _mmap_ system call.

```
--sgx-vm N         start N SGX enclaves spinning on trusted memory
--sgx-vm-bytes N   allocate N bytes per vm worker (default 32MB)
--sgx-vm-hang N    sleep N seconds before freeing memory
--sgx-vm-keep      redirty memory instead of reallocating
--sgx-vm-ops N     stop after N vm bogo operations
--sgx-vm-method M  specify stress vm method M, default is all
```

`--sgx-vm-method` accepts the following methods as parameter:
```
all flip galpat-0 galpat-1 gray rowhammer incdec inc-nybble rand-set rand-sum read64 ror swap move-inv modulo-x prime-0 prime-1 prime-gray-0 prime-gray-1 prime-incdec walk-0d walk-1d walk-0a walk-1a write64 zero-one
```

### Examples

* Run a simple CPU stressor on 1 core for 30 seconds in an SGX enclave
```bash
./stress-ng --sgx 1 --sgx-method bitops -t 30
```
* Run all EPC stressors on 1 core, operating on 30 MB of EPC
```bash
./stress-ng --sgx-vm 1 --sgx-vm-bytes 30M
```
* Run the `fft` stressor on all CPU cores in an SGX enclave
```bash
./stress-ng --sgx 0 --sgx-method fft
```
* Run the `fft` stressor on all CPU cores in an SGX enclave and collect metrics after 10 seconds
```bash
./stress-ng -t 10 --sgx 0 --sgx-method fft --metrics
```

## Ensuring byte-per-byte equivalence of native and SGX code

An enclave compiled using the official SGX SDK will produce a statically-compiled shared library.
It is possible to leverage this aspect to guarantee that both native and SGX versions of a stressor execute a perfectly identical binary by dynamically linking this shared object.
This optional approach, however, limits Stress-SGX to stressors that are only available in enclave mode.

Selecting the approach is done by compiling the version of Stress-SGX located in the `loadso` branch.
CPU and VM stressors are compatible with this option.

