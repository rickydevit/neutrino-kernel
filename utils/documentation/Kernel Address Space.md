# Neutrino Address Space
```c
  0 ⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛ 🟪🟪⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛   31
 32 ⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛ ⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛   63
 64 ⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛ ⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛   95 
 96 ⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛ ⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛   127
128 ⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛ ⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛   159
160 ⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛ ⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛   191
192 ⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛ ⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛   223
224 ⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛ ⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛   255
256 ⬜⬜⬜⬜⬜⬜⬜⬜⬜⬜⬜⬜⬜⬜⬜⬜ ⬜⬜⬜⬜⬜⬜⬜⬜⬜⬜⬜⬜⬜⬜⬜⬜   287
288 ⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛ ⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛   319
320 ⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛ ⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛   351
352 ⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛ ⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛   383
384 ⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛ ⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛   415
416 ⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛ ⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛   447
448 ⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛ ⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛   479
480 🟦🟦🟦🟦⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛ ⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛🟩🟧🟧🟩   511
```

| Region name | Region type | Region virtual address range | Size | Description | 
| :--- | :---: | :--- | :---: | :--- |
| 🟪 Process space | Process | `0xffff080000000000` - `0xffff08ffffffffff` | *Undefined* | MMIO mapped devices
| Available low half | | `0x0000000000000000` - `0xffff7fffffffffff` | *A lot* | Low memory area available for future use
| **System reserved** 
| ⬜ Physical memory mirror | System | `0xffff800000000000` - `0xfff8fffffffffff` | Same as physical memory. Potentially ≃1,152PB | Load point of the kernel. Functions and local variables should all have this base address
| RSDP probing area | System | `0xffff800000080000` - `0xffff800000100000` | 524,288kB | Area where the RSDP could be located according to its specification
| **Kernel reserved** 
| 🟦 MMIO devices | MMIO | `0xfffff00000000000` - `0xfffff1ffffffffff` | *Undefined* | MMIO mapped devices
| 🟩 Kernel heap | Dynamic | `0xfffffe0000000000` - `0xfffffe007fffffff` | 2,147GB | Kernel heap area
| 🟧 Inactive recursive page edit | Recurse point | `0xfffffe8000000000` - `undefined` | *Undefined* | Address used to perform recursive page editing on another page
| 🟧 Active recursive page edit | Recurse point | `0xffffff0000000000` - `undefined` | *Undefined* | Address used to perform recursive page editing on the active page
| 🟩 Kernel load point | Kernel | `0xffffffff80000000` - `unknown` | Same as kernel image size. *Unkown a priori* | Load point of the kernel. Functions and local variables should all have this base address