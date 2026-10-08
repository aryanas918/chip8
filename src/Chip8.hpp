#pragma once

#include <array>
#include <cstdint>
#include <string>

class Chip8 {
public:
    static constexpr std::size_t MEMORY_SIZE = 4096;
    static constexpr std::size_t NUM_REGISTERS = 16;
    static constexpr std::size_t STACK_SIZE = 16;

    static constexpr std::size_t SCREEN_WIDTH = 64;
    static constexpr std::size_t SCREEN_HEIGHT = 32;

    Chip8();

    bool loadROM(const std::string& path);
    void cycle();
    void executeOpcode(uint16_t opcode);
    void printState();
    void printDisplay() const;
    const std::array<uint8_t, SCREEN_WIDTH * SCREEN_HEIGHT>& getDisplay() const;
    
private:
    // 4 KB RAM
    std::array<uint8_t, MEMORY_SIZE> memory{};

    // V0 - VF
    std::array<uint8_t, NUM_REGISTERS> V{};

    // Address/index register
    uint16_t I = 0;

    // Program counter
    uint16_t pc = 0x200;

    // Stack
    std::array<uint16_t, STACK_SIZE> stack{};
    uint8_t sp = 0;

    // Timers
    uint8_t delayTimer = 0;
    uint8_t soundTimer = 0;

    // 64 × 32 monochrome display
    std::array<uint8_t, SCREEN_WIDTH * SCREEN_HEIGHT> display{};

    // 16-key hexadecimal keypad
    std::array<uint8_t, 16> keypad{};
};