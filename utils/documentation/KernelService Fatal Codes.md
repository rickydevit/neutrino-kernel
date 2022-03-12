# KernelService Fatal Error Codes
A fatal log message issued form the KernelService module will have the following format:   
`[FATAL] (0x?) <message>`   
with 0x? being the error code (described below) and message being the error message.

### Reserved codes (0x000 - 0x0ff)
| Code number   | Error name      | Description |  
|      -        |      -          |      -      |
| 0x000         | Internal Test   | *Used for internal purposes only, like testing KernelService modules* |
| 0x001         | Not implemented | *The function called is not implemented yet.* The name of the called function is specified in the fatal error message. |

### Core hardware missing or failure codes (0x100 - 0x1ff)
| Code number   | Error name      | Description |  
|      -        |      -          |      -      |
| 0x100         | No PIT          | *The PIT (Programmable Interval Timer) is not available in the current configuration of the system.* |
| 0x101         | No framebuffer  | *limine bootloader couldn't provide a valid framebuffer output.* |
| 0x102         | No LAPIC        | *The current processor configuration doesn't have a local APIC.* |

### Fatal exceptions codes (0x200 - 0x2ff)
| Code number   | Error name      | Description |  
|      -        |      -          |      -      |
| 0x200         | Generic exception | *A generic fatal exception occurred and the system couldn't recover.* |
| 0x201         | Interrupt exception | *A fatal interrupt exception occurred and the system couldn't recover.* |
| 0x202         | Out of memory   | *The physical memory of the system has saturated and no more memory is available.* |
| 0x203         | Out of heap   | *The heap memory of the system has saturated. Probably caused by a kernel memory leak.* |