#include "Chip8.hpp"
#include <SDL3/SDL.h>
#include <iostream>

constexpr int SCALE = 10;

int main() {

    Chip8 chip8;

    // Load your ROM
    if (!chip8.loadROM("./roms/2-ibm-logo.ch8")) {
        std::cerr << "Failed to load ROM\n";
        return 1;
    }

    // Initialize SDL
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << "SDL_Init failed: "
                  << SDL_GetError() << '\n';
        return 1;
    }

    // Create window
    SDL_Window* window = SDL_CreateWindow(
        "CHIP-8 Emulator",
        Chip8::SCREEN_WIDTH * SCALE,
        Chip8::SCREEN_HEIGHT * SCALE,
        0
    );

    if (!window) {
        std::cerr << "SDL_CreateWindow failed: "
                  << SDL_GetError() << '\n';

        SDL_Quit();
        return 1;
    }

    // Create renderer
    SDL_Renderer* renderer =
        SDL_CreateRenderer(window, nullptr);

    if (!renderer) {
        std::cerr << "SDL_CreateRenderer failed: "
                  << SDL_GetError() << '\n';

        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    bool running = true;
    SDL_Event event;

    while (running) {

        // -------------------------
        // Handle SDL events
        // -------------------------
        while (SDL_PollEvent(&event)) {

            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            }
        }

        // -------------------------
        // Execute CHIP-8 instructions
        // -------------------------
        for (int i = 0; i < 10; ++i) {
            chip8.cycle();
        }

        // -------------------------
        // Clear screen
        // -------------------------
        SDL_SetRenderDrawColor(
            renderer,
            0, 0, 0, 255
        );

        SDL_RenderClear(renderer);

        // -------------------------
        // Draw CHIP-8 pixels
        // -------------------------
        SDL_SetRenderDrawColor(
            renderer,
            255, 255, 255, 255
        );

        const auto& display = chip8.getDisplay();

        for (int y = 0; y < Chip8::SCREEN_HEIGHT; ++y) {

            for (int x = 0; x < Chip8::SCREEN_WIDTH; ++x) {

                const int index =
                    y * Chip8::SCREEN_WIDTH + x;

                if (display[index]) {

                    SDL_FRect pixel{
                        static_cast<float>(x * SCALE),
                        static_cast<float>(y * SCALE),
                        static_cast<float>(SCALE),
                        static_cast<float>(SCALE)
                    };

                    SDL_RenderFillRect(
                        renderer,
                        &pixel
                    );
                }
            }
        }

        // -------------------------
        // Present frame
        // -------------------------
        SDL_RenderPresent(renderer);

        // ~60 FPS
        SDL_Delay(16);
    }

    // -------------------------
    // Cleanup
    // -------------------------
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_Quit();

    return 0;
}