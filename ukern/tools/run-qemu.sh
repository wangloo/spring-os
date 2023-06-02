#!/bin/bash

qemu-system-aarch64 \
  -machine virt,gic-version=3,its=off\
  -cpu cortex-a57            \
  -smp 4                    \
  -m 2G                      \
  -kernel build/spring      \
  -serial mon:stdio         \
  -gdb tcp::1234              \
  -nographic               \
  -semihosting              \
  $1
