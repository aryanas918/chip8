#include <array>
#include <cstdint>
#include <fstream>
#include <iostream>

int main() {

    // We need:
    //
    // Program:
    // 0x200 - 0x207
    //
    // Font is already loaded by our emulator at 0x50,
    // so we DON'T need to put font data into this ROM.

    const uint8_t rom[] = {

        0x60, 0x0A, // 0x200: V0 = 0xA

        0xF0, 0x29, // 0x202: I = address of font for V0

        0x61, 0x05, // 0x204: V1 = 5  (X position)

        0x62, 0x05, // 0x206: V2 = 5  (Y position)

        0xD1, 0x25  // 0x208: draw sprite at (V1,V2), height 5
    };

    std::ofstream file(
        "font_test.ch8",
        std::ios::binary
    );

    if (!file) {
        std::cerr << "Could not create ROM\n";
        return 1;
    }

    file.write(
        reinterpret_cast<const char*>(rom),
        sizeof(rom)
    );

    file.close();

    std::cout << "Created font_test.ch8\n";

    return 0;
}