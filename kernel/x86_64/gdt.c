#include "gdt.h"
#include "kservice.h"

struct GDT_entry GDT[GDT_SIZE];

//* Load the Global Descriptor Table in the following way: 
// GDT[0] = Null Descriptor
// GDT[1] = 32 bit code descriptor
// GDT[2] = 32 bit data descriptor
// GDT[3] = 64 bit code descriptor
// GDT[4] = 64 bit data descriptor
void init_gdt() {
    uint64_t gdt_address;
    struct GDT_pointer gdt_ptr;

    // Null descriptor
    GDT[0] = GDT_ENTRY(0, 0, 0, 0);

    // 32-bit code and data descriptors
    GDT[1] = GDT_ENTRY(0, 0xffffffff, GDT_PRESENT | GDT_READWRITE | GDT_EXECUTABLE, GDT_FLAGS_32BIT);
    GDT[2] = GDT_ENTRY(0, 0xffffffff, GDT_PRESENT | GDT_READWRITE                 , GDT_FLAGS_32BIT);

    // 64-bit code and data descriptors
    GDT[3] = GDT_ENTRY(0, 0xffffffff, GDT_PRESENT | GDT_READWRITE | GDT_EXECUTABLE, GDT_FLAGS_64BIT);
    GDT[4] = GDT_ENTRY(0, 0xffffffff, GDT_PRESENT | GDT_READWRITE                 , GDT_FLAGS_64BIT);

    gdt_address = (uint64_t)GDT;
    gdt_ptr.size = (sizeof(struct GDT_entry) * GDT_SIZE);
    gdt_ptr.offset = gdt_address;

    ks.dbg("GDT built at %x. Loading to register...", gdt_address);
    load_gdt((uint32_t)&gdt_ptr);
	ks.dbg("Global Descriptor Table loaded successfully.");
}