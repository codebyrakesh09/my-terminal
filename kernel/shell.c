/* ============================================================
   shell.c - A minimal command loop: print a prompt, read a
   line of typed input, match it against known commands.
   ============================================================ */
#include "shell.h"
#include "vga.h"
#include "keyboard.h"

#define LINE_MAX 128

/* No standard library here, so a tiny string-equals helper. */
static int streq(const char *a, const char *b) {
    while (*a && *b) {
        if (*a != *b) return 0;
        a++;
        b++;
    }
    return *a == *b;
}

static void read_line(char *buf, int max_len) {
    int i = 0;
    while (1) {
        char c = keyboard_getchar();

        if (c == '\n') {
            vga_putc('\n');
            break;
        } else if (c == '\b') {
            if (i > 0) {
                i--;
                vga_putc('\b');
            }
        } else if (i < max_len - 1) {
            buf[i++] = c;
            vga_putc(c);
        }
    }
    buf[i] = '\0';
}

static void run_command(const char *cmd) {
    if (streq(cmd, "")) {
        return;   /* empty line - just reprompt */
    } else if (streq(cmd, "help")) {
        vga_print("Available commands:\n");
        vga_print("  help  - show this list\n");
        vga_print("  about - about this kernel\n");
        vga_print("  clear - clear the screen\n");
    } else if (streq(cmd, "about")) {
        vga_print("A hobby kernel, built from a bootloader up.\n");
        vga_print("Boots via a custom Real Mode bootloader,\n");
        vga_print("switches to 32-bit Protected Mode, and runs\n");
        vga_print("this shell entirely in C.\n");
    } else if (streq(cmd, "clear")) {
        vga_init();
        vga_print("Screen cleared.\n");
    } else {
        vga_print("Unknown command: ");
        vga_print(cmd);
        vga_print("\n");
    }
}

void shell_run(void) {
    char line[LINE_MAX];
    while (1) {
        vga_print("> ");
        read_line(line, LINE_MAX);
        run_command(line);
    }
}
