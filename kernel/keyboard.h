#ifndef KEYBOARD_H
#define KEYBOARD_H

/* Blocks until a key is pressed, returns its ASCII character.
   Unmapped keys (F-keys, arrows, etc.) are silently skipped -
   this function never returns for those, it just keeps waiting. */
char keyboard_getchar(void);

#endif
