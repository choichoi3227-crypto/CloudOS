#include <keyboard.h>
#include <io.h>

static char kb_buffer[256];
static int kb_head = 0, kb_tail = 0;

const char scancode_to_ascii[128] = {
    0, 27, '1','2','3','4','5','6','7','8','9','0','-','=', '\b',
    '\t','q','w','e','r','t','y','u','i','o','p','[',']','\n',
    0, 'a','s','d','f','g','h','j','k','l',';','\'', '`',
    0, '\\', 'z','x','c','v','b','n','m',',','.','/', 0, '*', 0, ' '
};

void keyboard_init(void) {
    kb_head = 0;
    kb_tail = 0;
}

void keyboard_handle_scancode(uint8_t scancode) {
    if (scancode < 128) {
        char c = scancode_to_ascii[scancode];
        if (c != 0) {
            int next = (kb_head + 1) % 256;
            if (next != kb_tail) {
                kb_buffer[kb_head] = c;
                kb_head = next;
            }
        }
    }
}

char keyboard_getchar(void) {
    while (kb_head == kb_tail) {
        __asm__ __volatile__("hlt");
    }
    char c = kb_buffer[kb_tail];
    kb_tail = (kb_tail + 1) % 256;
    return c;
}
