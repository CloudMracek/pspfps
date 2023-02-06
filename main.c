#include "pspmoduleinfo.h"
#include <pspkernel.h>

#include "system/callbacks.h"

PSP_MODULE_INFO("PSPFPS", 0, 1, 0);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER);

int main(int argc, char* argv[]) {
    setup_callbacks();
    while(!should_quit()) {
        
    }
    return 0;
}