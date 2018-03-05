#
# Copyright (C) 2017-2018 Sébastien Vaucher
# Copyright (C) 2013-2018 Canonical, Ltd.
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
#

VERSION=0.09.10
#
# Codename "portable pressure producer"
#

CFLAGS += -Wall -Wextra -DVERSION='"$(VERSION)"' -O2 -std=gnu99

SGX_SDK ?= /opt/intel/sgxsdk
SGX_MODE ?= HW
SGX_DEBUG ?= 1
SGX_PRERELEASE ?= 0

#
# Pedantic flags
#
ifeq ($(PEDANTIC),1)
CFLAGS += -Wabi -Wcast-qual -Wfloat-equal -Wmissing-declarations \
	-Wmissing-format-attribute -Wno-long-long -Wpacked \
	-Wredundant-decls -Wshadow -Wno-missing-field-initializers \
	-Wno-missing-braces -Wno-sign-compare -Wno-multichar
endif

#
# Static flags, only to be used when using GCC
#
ifeq ($(STATIC),1)
LDFLAGS += -static
CFLAGS += -DBUILD_STATIC
endif

BINDIR=/usr/bin
MANDIR=/usr/share/man/man1
JOBDIR=/usr/share/stress-ng/example-jobs

#
#  Stressors
#
STRESS_SRC = \
	stress-affinity.c \
	stress-af-alg.c \
	stress-aio.c \
	stress-aio-linux.c \
	stress-apparmor.c \
	stress-atomic.c \
	stress-bigheap.c \
	stress-bind-mount.c \
	stress-branch.c \
	stress-brk.c \
	stress-bsearch.c \
	stress-cache.c \
	stress-cap.c \
	stress-chdir.c \
	stress-chmod.c \
	stress-chown.c \
	stress-chroot.c \
	stress-clock.c \
	stress-clone.c \
	stress-context.c \
	stress-copy-file.c \
	stress-cpuso.c \
	stress-cpu-online.c \
	stress-crypt.c \
	stress-cyclic.c \
	stress-daemon.c \
	stress-dccp.c \
	stress-dentry.c \
	stress-dev.c \
	stress-dir.c \
	stress-dirdeep.c \
	stress-dnotify.c \
	stress-dup.c \
	stress-epoll.c \
	stress-eventfd.c \
	stress-exec.c \
	stress-fallocate.c \
	stress-fanotify.c \
	stress-fault.c \
	stress-fcntl.c \
	stress-fiemap.c \
	stress-fifo.c \
	stress-filename.c \
	stress-flock.c \
	stress-fork.c \
	stress-fp-error.c \
	stress-fstat.c \
	stress-full.c \
	stress-funccall.c \
	stress-futex.c \
	stress-get.c \
	stress-getrandom.c \
	stress-getdent.c \
	stress-handle.c \
	stress-hdd.c \
	stress-heapsort.c \
	stress-hrtimers.c \
	stress-hsearch.c \
	stress-icache.c \
	stress-icmp-flood.c \
	stress-inode-flags.c \
	stress-inotify.c \
	stress-iomix.c \
	stress-ioport.c \
	stress-ioprio.c \
	stress-iosync.c \
	stress-itimer.c \
	stress-kcmp.c \
	stress-key.c \
	stress-kill.c \
	stress-klog.c \
	stress-lease.c \
	stress-lsearch.c \
	stress-link.c \
	stress-lockbus.c \
	stress-locka.c \
	stress-lockf.c \
	stress-lockofd.c \
	stress-longjmp.c \
	stress-madvise.c \
	stress-malloc.c \
	stress-matrix.c \
	stress-membarrier.c \
	stress-memcpy.c \
	stress-memfd.c \
	stress-memrate.c \
	stress-memthrash.c \
	stress-mergesort.c \
	stress-mincore.c \
	stress-mknod.c \
	stress-mlock.c \
	stress-mmap.c \
	stress-mmapfork.c \
	stress-mmapmany.c \
	stress-mmapaddr.c \
	stress-mremap.c \
	stress-msg.c \
	stress-msync.c \
	stress-mq.c \
	stress-netdev.c \
	stress-netlink-proc.c \
	stress-nice.c \
	stress-nop.c \
	stress-null.c \
	stress-numa.c \
	stress-oom-pipe.c \
	stress-opcode.c \
	stress-open.c \
	stress-personality.c \
	stress-physpage.c \
	stress-pipe.c \
	stress-poll.c \
	stress-procfs.c \
	stress-pthread.c \
	stress-ptrace.c \
	stress-pty.c \
	stress-quota.c \
	stress-qsort.c \
	stress-radixsort.c \
	stress-rawdev.c \
	stress-rdrand.c \
	stress-readahead.c \
	stress-remap-file-pages.c \
	stress-rename.c \
	stress-resources.c \
	stress-rlimit.c \
	stress-rmap.c \
	stress-rtc.c \
	stress-sctp.c \
	stress-schedpolicy.c \
	stress-seal.c \
	stress-seccomp.c \
	stress-seek.c \
	stress-sem.c \
	stress-sem-sysv.c \
	stress-sendfile.c \
	stress-sgx.c \
	stress-sgx-vm.c \
	stress-shm.c \
	stress-shm-sysv.c \
	stress-sigfd.c \
	stress-sigfpe.c \
	stress-sigpending.c \
	stress-sigsegv.c \
	stress-sigsuspend.c \
	stress-sigq.c \
	stress-sleep.c \
	stress-socket.c \
	stress-socket-diag.c \
	stress-socket-fd.c \
	stress-socketpair.c \
	stress-softlockup.c \
	stress-spawn.c \
	stress-splice.c \
	stress-stack.c \
	stress-stackmmap.c \
	stress-str.c \
	stress-stream.c \
	stress-swap.c \
	stress-switch.c \
	stress-sync-file.c \
	stress-sysinfo.c \
	stress-sysfs.c \
	stress-tee.c \
	stress-timer.c \
	stress-timerfd.c \
	stress-tlb-shootdown.c \
	stress-tmpfs.c \
	stress-tree.c \
	stress-tsc.c \
	stress-tsearch.c \
	stress-udp.c \
	stress-udp-flood.c \
	stress-unshare.c \
	stress-urandom.c \
	stress-userfaultfd.c \
	stress-utime.c \
	stress-vecmath.c \
	stress-vforkmany.c \
	stress-vm.c \
	stress-vm-rw.c \
	stress-vm-splice.c \
	stress-wait.c \
	stress-wcstr.c \
	stress-xattr.c \
	stress-yield.c \
	stress-zero.c \
	stress-zlib.c \
	stress-zombie.c \

#
# Stress core
#
CORE_SRC = \
	affinity.c \
	cache.c \
	helper.c \
	ignite-cpu.c \
	io-priority.c \
	job.c \
	limit.c \
	log.c \
	madvise.c \
	mincore.c \
	mlock.c \
	mmap.c \
	mounts.c \
	mwc.c \
	net.c \
	out-of-memory.c \
	parse-opts.c \
	perf.c \
	sched.c \
	setting.c \
	shim.c \
	thermal-zone.c \
	time.c \
	thrash.c \
	stress-ng.c

SRC = $(STRESS_SRC) $(CORE_SRC)
OBJS = $(SRC:.c=.o)

APPARMOR_PARSER=/sbin/apparmor_parser

LIB_APPARMOR := -lapparmor
LIB_BSD := -lbsd
LIB_Z := -lz
LIB_CRYPT := -lcrypt
LIB_RT := -lrt
LIB_PTHREAD := -lpthread
LIB_AIO = -laio
LIB_SCTP = -lsctp

#
#  Load in current config; use 'make clean' to clear this
#
-include config
ifneq ("$(wildcard config)","")
all: all_config
.PHONY: all_config enclave_cpu.signed.so enclave_vm.signed.so
all_config:
	$(MAKE) -f Makefile.config
	$(MAKE) stress-ng
endif

CFLAGS += $(CONFIG_CFLAGS)
CFLAGS += -I$(SGX_SDK)/include
LDFLAGS += $(CONFIG_LDFLAGS) ./sgx/enclave_cpu/enclave_cpu.so
ifneq ($(SGX_MODE), HW)
	LDFLAGS += -L$(SGX_SDK)/lib64 -lsgx_urts_sim -lsgx_uae_service_sim
else
	LDFLAGS += -L$(SGX_SDK)/lib64 -lsgx_urts
endif
OBJS += $(CONFIG_OBJS)

.SUFFIXES: .c .o

.o: stress-ng.h Makefile

.c.o: sgx/utils.h stress-ng.h Makefile $(SRC)
	@echo $(CC) $(CFLAGS) -c -o $@ $<
	@$(CC) $(CFLAGS) -c -o $@ $<

sgx/utils.o: sgx/utils.c sgx/utils.h
	$(CC) $(CFLAGS) -c -o $@ sgx/utils.c

enclave_cpu.signed.so enclave_vm.signed.so sgx/enclave_*/untrusted/enclave_u.o:
	$(MAKE) -f sgx/Makefile SGX_MODE=$(SGX_MODE) SGX_DEBUG=$(SGX_DEBUG) SGX_PRERELEASE=$(SGX_PRERELEASE)
	cp --preserve=all --reflink=auto sgx/enclave_cpu/enclave_cpu.signed.so enclave_cpu.signed.so
	cp --preserve=all --reflink=auto sgx/enclave_vm/enclave_vm.signed.so enclave_vm.signed.so

stress-ng: sgx/utils.o enclave_cpu.signed.so enclave_vm.signed.so $(OBJS)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(OBJS) sgx/utils.o sgx/enclave_cpu/untrusted/enclave_u.o sgx/enclave_vm/untrusted/vm_u.o -lm $(LDFLAGS) -lc -o $@

#
#  generate apparmor data using minimal core utils tools from apparmor
#  parser output
#
apparmor-data.o: usr.bin.pulseaudio.eg
	@$(APPARMOR_PARSER) -Q usr.bin.pulseaudio.eg  -o apparmor-data.bin
	@echo "#include <stddef.h>" > apparmor-data.c
	@echo "char g_apparmor_data[]= { " >> apparmor-data.c
	@od -tx1 -An -v < apparmor-data.bin | \
		sed 's/[0-9a-f][0-9a-f]/0x&,/g' | \
		sed '$$ s/.$$//' >> apparmor-data.c
	@echo "};" >> apparmor-data.c
	@echo "const size_t g_apparmor_data_len = sizeof(g_apparmor_data);" >> apparmor-data.c
	$(CC) -c apparmor-data.c -o apparmor-data.o
	@rm -rf apparmor-data.c

#
#  extract the PER_* personality enums
#
personality.h:
	@$(CPP) personality.c | grep -e "PER_[A-Z0-9]* =.*," | cut -d "=" -f 1 \
	| sed "s/.$$/,/" > personality.h

stress-personality.c: personality.h

perf.o: perf.c perf-event.c
	@gcc -E perf-event.c | grep "PERF_COUNT" | sed 's/,/ /' | awk {'print "#define _SNG_" $$1 " (1)"'} > perf-event.h
	@echo $(CC) $(CFLAGS) -c -o $@ $<
	@$(CC) $(CFLAGS) -c -o $@ $<

stress-vecmath.o: stress-vecmath.c
	@echo $(CC) $(CFLAGS) -fno-builtin -c -o $@ $<
	@$(CC) $(CFLAGS) -fno-builtin -c -o $@ $<
	@touch stress-ng.c

$(OBJS): stress-ng.h Makefile

stress-ng.1.gz: stress-ng.1
	gzip -c $< > $@

.PHONY: dist
dist:
	rm -rf stress-ng-$(VERSION)
	mkdir stress-ng-$(VERSION)
	cp -rp Makefile Makefile.config $(SRC) stress-ng.h stress-ng.1 \
		personality.c COPYING syscalls.txt mascot README \
		README.Android test snapcraft smatchify.sh config TODO \
		perf-event.c usr.bin.pulseaudio.eg stress-version.h \
		example-jobs stress-ng-$(VERSION)
	tar -Jcf stress-ng-$(VERSION).tar.xz stress-ng-$(VERSION)
	rm -rf stress-ng-$(VERSION)

.PHONY: pdf
pdf:
	man -t ./stress-ng.1 | ps2pdf - > stress-ng.pdf


.PHONY: clean
clean:
	$(MAKE) -f sgx/Makefile clean
	@rm -f stress-ng $(OBJS) stress-ng.1.gz stress-ng.pdf
	@rm -f stress-ng-$(VERSION).tar.xz
	@rm -f personality.h
	@rm -f perf-event.h
	@rm -f *.o
	@:> config

.PHONY: fast-test-all
fast-test-all: stress-ng
	STRESS_NG=./stress-ng debian/tests/fast-test-all

.PHONY: install
install: stress-ng stress-ng.1.gz
	mkdir -p ${DESTDIR}${BINDIR}
	cp stress-ng ${DESTDIR}${BINDIR}
	mkdir -p ${DESTDIR}${MANDIR}
	cp stress-ng.1.gz ${DESTDIR}${MANDIR}
	mkdir -p ${DESTDIR}${JOBDIR}
	cp -rp example-jobs/*.job ${DESTDIR}${JOBDIR}
