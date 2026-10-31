/* ============================================================
   keyboard.c - PS/2 keyboard driver using POLLING.

   Instead of waiting for a hardware interrupt to tell us a key
   was pressed, we just repeatedly ask the keyboard controller
   "is there data ready?" in a loop. This is less efficient than
   interrupts (the CPU is busy the whole time instead of idling),
   but it's dramatically simpler and has far fewer ways to fail -
   no IDT, no PIC remapping, no assembly stubs required at all.
   ============================================================ */
#include <stdint.h>
#include "keyboard.h"
#include "io.h"

#define KB_DATA_PORT    0x60
#define KB_STATUS_PORT  0x64
#define KB_OUTPUT_FULL  0x01   /* bit 0 of the status port: 1 means
                                   a byte is waiting to be read */

#define LSHIFT_PRESS   0x2A
#define RSHIFT_PRESS   0x36
#define LSHIFT_RELEASE 0xAA
#define RSHIFT_RELEASE 0xB6

static int shift_pressed = 0;

static const char scancode_ascii[128] = {
    0,  27, '1','2','3','4','5','6','7','8','9','0','-','=','\b',
  '\t','q','w','e','r','t','y','u','i','o','p','[',']','\n',
    0,
  'a','s','d','f','g','h','j','k','l',';','\'','`',
    0,
  '\\','z','x','c','v','b','n','m',',','.','/',
    0,
  '*',
    0,
  ' ',
    0
};

static const char scancode_ascii_shift[128] = {
    0,  27, '!','@','#','$','%','^','&','*','(',')','_','+','\b',
  '\t','Q','W','E','R','T','Y','U','I','O','P','{','}','\n',
    0,
  'A','S','D','F','G','H','J','K','L',':','"','~',
    0,
  '|','Z','X','C','V','B','N','M','<','>','?',
    0,
  '*',
    0,
  ' ',
    0
};

static int key_available(void) {
    return inb(KB_STATUS_PORT) & KB_OUTPUT_FULL;
}

char keyboard_getchar(void) {
    while (1) {
        while (!key_available()) {
            /* busy-wait: keep asking until a key is ready */
        }

        uint8_t scancode = inb(KB_DATA_PORT);

        if (scancode == LSHIFT_PRESS || scancode == RSHIFT_PRESS) {
            shift_pressed = 1;
            continue;
        }
        if (scancode == LSHIFT_RELEASE || scancode == RSHIFT_RELEASE) {
            shift_pressed = 0;
            continue;
        }
        if (scancode & 0x80) {
            continue;   /* any other key release - ignore, keep waiting */
        }

        char c = shift_pressed ? scancode_ascii_shift[scancode]
                                 : scancode_ascii[scancode];
        if (c != 0) {
            return c;
        }
        /* unmapped key (F-keys, arrows, etc.) - loop and wait for
           the next one instead of returning a garbage character */
    }
}
