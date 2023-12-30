# Put this file in the same directory with libelf and libdwarf

install_lib_dir=$(realpath ../lib)
install_inc_dir=$(realpath ../inc/lib)
libelf_dir=$(realpath ./libelf)
libdwarf_dir=$(realpath ./libdwarf/libdwarf)


if [ ! -d "$libelf_dir" ] || [ ! -d "$libdwarf_dir" ]; then
  echo "Can not find source path of libelf or libdwarf"
  exit 1
fi

## Build libelf(libdwarf need it)
cd "$libelf_dir"
rm -rf build install

mkdir build && cd build
../configure  --prefix=$libelf_dir/install
make CC=aarch64-none-linux-gnu-gcc LD=aarch64-none-linux-gnu-ld AR=aarch64-none-linux-gnu-ar XCFLAGS="--include=$libelf_dir/lib/spring.h"
make install


## Build libdwarf(Must after libelf)
cd "$libdwarf_dir"
rm -rf build

mkdir build && cd build
# ../configure --host=aarch64-none-linux-gnu CFLAGS="-I$libelf_dir/install/include -g" LDFLAGS="-L$libelf_dir/install/lib"
../configure --host=aarch64-none-linux-gnu CFLAGS="-I$libelf_dir/install/include" LDFLAGS="-L$libelf_dir/install/lib"
make HOSTCC=cc gennames
make HOSTCC=cc errmsg_check
make   POSTINCS="--include=$libdwarf_dir/spring.h" 


## Install lib and header
cp $libelf_dir/install/lib/libelf.a $install_lib_dir
cp $libdwarf_dir/build/libdwarf.a   $install_lib_dir
cp $libdwarf_dir/build/libdwarf.h   $install_inc_dir
cp $libdwarf_dir/libdwarf_ukern.h   $install_inc_dir