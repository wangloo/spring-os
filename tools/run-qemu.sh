#!/bin/bash

qemu-system-aarch64 \
  -machine virt,gic-version=3,its=off\
  -cpu cortex-a57            \
  -smp 4                    \
  -m 2G                      \
  -kernel out/kern/spring.elf      \
  -serial mon:stdio         \
  -drive if=pflash,file=out/ramdisk.bin,unit=1,format=raw \
  -gdb tcp::1234              \
  -nographic               \
  -semihosting              \
  $1

	# -drive file=mydisk_128M,if=none,format=raw,id=hd \
  # -device virtio-blk-device,drive=hd \
	# -global virtio-mmio.force-legacy=false \