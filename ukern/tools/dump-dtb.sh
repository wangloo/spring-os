#!/bin/bash

qemu-system-aarch64 -machine virt,dumpdtb=qemu_virt.dtb

# write in .dts file
# dtc -o qemu_virt.dts -O dts -I dtb qemu_virt.dtb

# stdout
dtc  -O dts -I dtb qemu_virt.dtb

rm qemu_virt.dtb