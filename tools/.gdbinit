set architecture aarch64
target remote localhost:1234
file ./ukern/build/spring.elf
info threads
set scheduler-locking step 
show scheduler-locking

layout split
focus cmd

define q
    quit
end
