#include "../drivers/ports.h"
#include "../drivers/screen.h"


void main() {
    clear_screen();
    kprint_at("Hello\nFriends!", 0, 0);
}