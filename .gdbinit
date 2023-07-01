set architecture aarch64
target remote localhost:1234
file ./ukern/build/spring
info threads
set scheduler-locking step 
show scheduler-locking

define q
    quit
end
