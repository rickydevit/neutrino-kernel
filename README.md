# Neutrino Kernel Project
This project is a personal way to discover and implement a 64 bit kernel written in C and Assembly. The kernel will have basic drivers and functionality, but other features may be added in the future. The currently implemented features are listed below.

## Features
- [x] ⛱ **Limine bootloader** as the main boot-up solution for the moment
    - [x] **Stivale2 protocol compliant**   
        *The kernel asks the bootloader for a linear framebuffer, but can also use a terminal if the framebuffer is not available. The bootloader also pass some informations to the kernel.*

- [ ] 🌳 **Basic kernel drivers**
    - [x] **Kernel Services**   
        *The kernel services functions are used to provide a easier to way to interface the kernel and the drivers. These functions will NOT be available to the user space.*
    - [x] **GDT setup**   
        *Null descriptor; 32 bit code and data descriptors; 64 bit code and data descriptors. More descriptors will be added when required.*
    - [x] **IDT setup**   
        *Basic x86_64 interrupts and exceptions handled. Software interrupts will be added when necessary.*
    - [x] **Serial comms**   
        *Serial communication with write and read operations from and to a given serial port*
    - [ ] **PIT**
    - [ ] **APIC**
    - [x] **CPUID** 
    - [ ] **RTC**
    - [ ] **Memory manager**, both physical and virtual
        - [x] **Physical memory manager**   
            *Scans the loaded memory and manages it using 4KB blocks. Kernel and other reserved areas are marked accordingly*
        - [ ] **Virtual memory manager**
    - [ ] **Process manager**

- [ ] ⚙ **Advanced drivers**
    - [x] **Video driver**
        - [x] **Plot pixel**
        - [ ] **Draw line**
        - [ ] **Draw complex shapes**
    - [ ] **USB driver**
    - [ ] **Network driver**
    - [ ] **Storage drivers**
    - [ ] **Filesystem drivers**

- [ ] 👤 **Userspace**
    - [ ] **System calls**
    - [ ] *And many apps*

## Source structure
- **`kernel\`**
    - **`x86-64\`** _platform-specific code for the x86-64 architecture_
    - **`common\`** _platform-independent kernel code_
        - **`device\`** _libraries for device abstraction in the kernel_
        - **`video\`** _implementation of video driver_
- **`libs\`**
    - **`libc\`** _porting of useful C libraries_
- **`limine\`** [_limine bootloader binaries_](https://github.com/limine-bootloader/limine/tree/v2.0-branch-binary)
- **`thirdparty\`** _implementations of thirdparty headers and libraries_
- **`utils\`** _file stored as backup or utility_