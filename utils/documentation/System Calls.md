# Neutrino System Calls implementation
System calls in Neutrino are implemented on 3 different levels: [library](#Library%20level), [kernel](#Kernel%20level) and [architecture](#Architecture%20level). Each level implement a diferrent piece of the [system call flow](#System%20call%20flow).

### Library level
Files are located in [libs/neutrino](/libs/neutrino/) (`syscall.c` and `syscall.h`) and include syscall related enums and types and **syscall function prototypes** which are different if library is compiled with kernel or with executables.   
In executable-compilation syscalls are exposed as `neutrino_<name>()` and can be called accordingly; executing a syscall simply execute the `syscall` instruction (on x86_64) which is later handled in the architecture implementation.   
In kernel-compilation syscalls are exposed as `sys_<name>()` thus exposing kernel-level implementations of syscalls. Executing a syscall in kernel code results in executing directly the code associated with the syscall without executing a syscall (since it's not necessary).

### Kernel level
Files are located in [kernel/common](/kernel/common) (`syscall.c` and `syscall.h`) and include implementation of syscalls and the `syscall_execute()` function, that is responsible to check the syscall is valid and executing it therefore.

### Architecture level
Files are located in architecture folders, such as [kernel/x86_64](/kernel/x86_64) (`syscall.c`, `syscall.h` and `sysc.asm`), and include a initialization function to setup registers and structures used in syscalls and the `syscall_handler()` which is called from the `_syscall` assembly routine after saving and changing context.

## System call flow
The general order of execution of a system call is as follows:   
- A program calls a `neutrino_<name>` syscall function from user space, which executes the relative `syscall` instruction
- The system call is handled on architecture level as:
  - **x86_64**: user data and code segment are swapped with kernel data and code segment as defined in the STAR register (see [`init_syscall()`](/kernel/x86_64/syscall.c#L15)) and RIP content is now LSTAR content, so code pass to assembly routine `_syscall`. The assembly routine saves user RSP and RIP and call [`syscall_handler()`](/kernel/x86_64/syscall.c#L35) which passes control to architecture-independent function `syscall_execute()`
- Kernel function [`syscall_execute()`](kernel/common/syscall.c#L22) checks the syscall id and set the current task as in syscall
- The syscall is executed and the value returned to the user program
