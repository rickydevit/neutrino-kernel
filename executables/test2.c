#include <neutrino/syscall.h>
#include <liballoc.h>
#include <_null.h>
#include <string.h>
#include <stdint.h>

int main() {
    neutrino_log(&(SCLogArgs){.msg = "IPC test started"});

    SCIpcArgs a = (SCIpcArgs){
        .type = IPC_PEEK
    };
    neutrino_ipc(&a);

    if (a.size > 0) {
        uintptr_t ptr = (uintptr_t)malloc(a.size);
        a.type = IPC_RECEIVE;
        a.payload = ptr;
        neutrino_ipc(&a);
        char buf[128];
        strf("IPC returned payload at %x size %u. Payload is %c", buf, a.payload, a.size, (char*)a.payload);
        neutrino_log(&(SCLogArgs){.msg = buf});
        free((void*)ptr);
    } else {
        neutrino_log(&(SCLogArgs){.msg = "No payload from IPC"});
    }

    return 0;
}
