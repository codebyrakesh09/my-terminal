/* ============================================================
   kernel.c
   ============================================================ */
#include "vga.h"
#include "shell.h"

void kmain(void) {
    vga_init();
    vga_print("MyOS booted successfully!\n");
    vga_print("Type 'help' for a list of commands.\n\n");

    shell_run();   /* never returns */
}
