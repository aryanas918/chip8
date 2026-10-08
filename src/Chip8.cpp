#include "Chip8.hpp"
#include <iomanip>
#include <fstream>
#include <iostream>
#include <vector>


static constexpr uint16_t FONT_START = 0x50;

static const uint8_t FONTSET[80] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

Chip8::Chip8() {
    pc = 0x200;
    for (std::size_t i = 0; i < 80; ++i) {
        memory[FONT_START + i] = FONTSET[i];
    }
}

bool Chip8::loadROM(const std::string& path) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);

    if (!file) {
        std::cerr << "Could not open ROM: " << path << '\n';
        return false;
    }

    const std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    if (size > static_cast<std::streamsize>(MEMORY_SIZE - 0x200)) {
        std::cerr << "ROM too large\n";
        return false;
    }

    if (!file.read(
            reinterpret_cast<char*>(&memory[0x200]),
            size)) {
        std::cerr << "Could not read ROM\n";
        return false;
    }

    std::cout
        << "Loaded ROM: "
        << size
        << " bytes\n";

    return true;
}

void Chip8::cycle() {
    const uint16_t opcode =
        static_cast<uint16_t>(memory[pc] << 8)
        | memory[pc + 1];

    printState();
    pc += 2;
    executeOpcode(opcode);
    // Decode and execute opcode here
}

void Chip8::printState() {
    std::cout << "PC=" << std::hex << pc
              << " I=" << I
              << " SP=" << static_cast<int>(sp)
              << '\n';

    for (int i = 0; i < 16; ++i) {
        std::cout
            << "V" << std::hex << i
            << "=" << static_cast<int>(V[i])
            << " ";
    }

    std::cout << "\n\n";
}
void Chip8::printDisplay() const {
    for (std::size_t y = 0; y < SCREEN_HEIGHT; ++y) {

        for (std::size_t x = 0; x < SCREEN_WIDTH; ++x) {

            const std::size_t index =
                y * SCREEN_WIDTH + x;

            std::cout << (display[index] ? "##" : "  ");
        }

        std::cout << '\n';
    }
}

const std::array<uint8_t, Chip8::SCREEN_WIDTH * Chip8::SCREEN_HEIGHT>&
Chip8::getDisplay() const {
    return display;
}

void Chip8::executeOpcode(uint16_t opcode) {
    const uint8_t x  = (opcode & 0x0F00) >> 8;
    const uint8_t y  = (opcode & 0x00F0) >> 4;
    const uint8_t nn = opcode & 0x00FF;
    const uint16_t nnn = opcode & 0x0FFF;

    switch (opcode & 0xF000) {

        case 0x0000:
            switch (opcode) {
                case 0x00E0:
                    // Clear screen
                    display.fill(0);
                    break;

                case 0x00EE:
                    // Return from subroutine
                    --sp;
                    pc = stack[sp];
                    break;

                default:
                    std::cerr << "Unknown 0x0000 opcode: "
                              << std::hex << opcode << '\n';
                    break;
            }
            break;

        case 0x1000:
            // 1NNN - jump
            pc = nnn;
            break;

        case 0x2000:
            // 2NNN - call subroutine
            stack[sp] = pc;
            ++sp;
            pc = nnn;
            break;

        case 0x3000:
            // 3XNN - skip if VX == NN
            if (V[x] == nn) {
                pc += 2;
            }
            break;

        case 0x4000:
            // 4XNN - skip if VX != NN
            if (V[x] != nn) {
                pc += 2;
            }
            break;

        case 0x5000:
            // 5XY0 - skip if VX == VY
            if ((opcode & 0x000F) == 0) {
                if (V[x] == V[y]) {
                    pc += 2;
                }
            }
            break;

        case 0x6000:
            // 6XNN - VX = NN
            V[x] = nn;
            break;

        case 0x7000:
            // 7XNN - VX += NN
            V[x] += nn;
            break;

        case 0x8000:
            switch (opcode & 0x000F) {

                case 0x0:
                    // 8XY0 - VX = VY
                    V[x] = V[y];
                    break;

                case 0x1:
                    // 8XY1 - VX = VX OR VY
                    V[x] |= V[y];
                    break;

                case 0x2:
                    // 8XY2 - VX = VX AND VY
                    V[x] &= V[y];
                    break;

                case 0x3:
                    // 8XY3 - VX = VX XOR VY
                    V[x] ^= V[y];
                    break;

                case 0x4: {
                    // 8XY4 - VX = VX + VY
                    const uint16_t sum =
                        static_cast<uint16_t>(V[x]) +
                        static_cast<uint16_t>(V[y]);

                    V[0xF] = sum > 0xFF ? 1 : 0;
                    V[x] = static_cast<uint8_t>(sum);
                    break;
                }

                case 0x5: {
                    // 8XY5 - VX = VX - VY
                    //
                    // VF = 1 if there is NO borrow
                    // VF = 0 if there IS a borrow

                    const uint8_t vx = V[x];
                    const uint8_t vy = V[y];

                    V[0xF] = vx >= vy ? 1 : 0;
                    V[x] = vx - vy;
                    break;
                }

                case 0x6: {
                    // 8XY6 - shift VX right by 1
                    //
                    // VF stores the bit that gets shifted out

                    const uint8_t vx = V[x];

                    V[0xF] = vx & 0x01;
                    V[x] = vx >> 1;
                    break;
                }

                case 0x7: {
                    // 8XY7 - VX = VY - VX

                    const uint8_t vx = V[x];
                    const uint8_t vy = V[y];

                    V[0xF] = vy >= vx ? 1 : 0;
                    V[x] = vy - vx;
                    break;
                }

                case 0xE: {
                    // 8XYE - shift VX left by 1
                    //
                    // VF stores the most significant bit

                    const uint8_t vx = V[x];

                    V[0xF] = (vx & 0x80) >> 7;
                    V[x] = vx << 1;
                    break;
                }

                default:
                    std::cerr
                        << "Unknown 8XY? opcode: 0x"
                        << std::hex
                        << opcode
                        << '\n';
                    break;
            }
            break;
        case 0x9000:
            // 9XY0 - skip if VX != VY
            if ((opcode & 0x000F) == 0) {
                if (V[x] != V[y]) {
                    pc += 2;
                }
            }
            break;

        case 0xA000:
            // ANNN - I = NNN
            I = nnn;
            break;
        case 0xD000: {
            const uint8_t height = opcode & 0x000F;

            const uint8_t startX = V[x] % SCREEN_WIDTH;
            const uint8_t startY = V[y] % SCREEN_HEIGHT;

            V[0xF] = 0;

            for (uint8_t row = 0; row < height; ++row) {

                const uint8_t spriteByte = memory[I + row];

                for (uint8_t column = 0; column < 8; ++column) {

                    const uint8_t spritePixel =
                        spriteByte & (0x80 >> column);

                    if (spritePixel == 0)
                        continue;

                    const uint8_t pixelX =
                        (startX + column) % SCREEN_WIDTH;

                    const uint8_t pixelY =
                        (startY + row) % SCREEN_HEIGHT;

                    const std::size_t index =
                        pixelY * SCREEN_WIDTH + pixelX;

                    // If an ON pixel is about to be turned OFF,
                    // a collision occurred.
                    if (display[index] == 1) {
                        V[0xF] = 1;
                    }

                    // CHIP-8 drawing uses XOR
                    display[index] ^= 1;
                }
            }

            break;
        }
        case 0xF000:
            switch (opcode & 0x00FF) {

                case 0x29:
                    // FX29 - point I at font sprite for VX
                    I = FONT_START + (V[x] * 5);
                    break;

                default:
                    std::cerr
                        << "Unknown FX?? opcode: 0x"
                        << std::hex
                        << opcode
                        << '\n';
                    break;
            }
            break;
        default:
            std::cerr << "Unknown opcode: 0x"
                      << std::hex << opcode << '\n';
            break;
    }
}

