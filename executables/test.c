#include <neutrino/syscall.h>
#include <liballoc.h>
#include <_null.h>
#include <string.h>
#include <stdint.h>

int main() {
    char buf[128];
    uint32_t* test = (uint32_t*)malloc(sizeof(uint32_t));

    strf("malloc() returned %x", buf, test);
    neutrino_log(&(SCLogArgs){.msg = buf});

    free(test);
    strf("free() called for %x", buf, test);
    neutrino_log(&(SCLogArgs){.msg = buf});

    const char* ipc_payload = "hello world";
    neutrino_ipc(&(SCIpcArgs){
        .agent_name = "test2",
        .payload = (uintptr_t)ipc_payload,
        .size = strlen(ipc_payload),
        .type = IPC_SEND
        });
    return 0;
}
