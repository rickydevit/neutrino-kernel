# Neutrino Kernel Project
This project is a personal way to discover and implement a 64 bit kernel written in C and Assembly. The kernel will have basic drivers and functionality, but other features may be added in the future. The currently implemented features are listed below.

## Features
- [x] ⛱ **Limine bootloader** as the main boot-up solution for the moment
    - [x] **Stivale2 protocol compliant** 

- [ ] 🌳 **Basic kernel drivers**
    - [ ] **GDT** setup
    - [ ] **IDT** setup
    - [x] **Serial comms**
    - [ ] **PIT**
    - [ ] **APIC**
    - [ ] **CPUID** 
    - [ ] **RTC**
    - [ ] **Memory manager**, both physical and virtual
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