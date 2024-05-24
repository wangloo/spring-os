## Introduce

This is a demo of micro-kernel based operating system.

## Platform

AArch64 simulated by QEMU.


## Promise

To motivate myself, submit at least once every two days.


## Build

### With KMon

```shell
# Build thiryparty
cd ukern/thirdparty
./build_thirdparty.sh
# Build kernl
cd ../..  # back to rootpath
make kernel
# Build runtime library and user
make 
```


