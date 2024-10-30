#include <pspkernel.h>
#include <psploadexec.h>

int exit_request = 0;

int should_quit(void) {
    return exit_request;
}

int exit_callback(int x, int y, void* callback) {
    exit_request = 1;
    return 0;
}

int callback_thread(SceSize args, void *argp) {
    int callback_id;

	callback_id = sceKernelCreateCallback("Exit Callback", exit_callback, NULL);
	sceKernelRegisterExitCallback(callback_id);

	sceKernelSleepThreadCB();

	return 0;
}

int setup_callbacks(void) {
    int thread_id;
    thread_id = sceKernelCreateThread("update_thread", callback_thread, 0x11, 0xFA8, 0, 0);
    if(thread_id >= 0) {
        sceKernelStartThread(thread_id, 0, 0);
        return 0;
    }
    else {
        return -1;
    }
}