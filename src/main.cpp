#include <cstdio>

#include <mapper.h>
#include <nesdoug.h>
#include <neslib.h>

#include "./core/bank.h"
#include "./core/ppu.h"
#include "explosion.hpp"

MAPPER_PRG_ROM_KB(32);
MAPPER_CHR_ROM_KB(128);
MAPPER_PRG_RAM_KB(8);
MAPPER_USE_VERTICAL_MIRRORING;

constexpr char hello[] = "Hello, NES!";

constexpr uint8 background_pal[] = {
    0x0f, 0x10, 0x20, 0x30, // grayscale
    0x0f, 0x10, 0x20, 0x30, // grayscale
    0x0f, 0x10, 0x20, 0x30, // grayscale
    0x0f, 0x10, 0x20, 0x30, // grayscale
};

constexpr uint8 sprite_pal[] = {
    0x0f, 0x10, 0x26, 0x30, // cogwheel
    0x0f, 0x11, 0x2a, 0x16, // explosions
    0x0f, 0x10, 0x20, 0x30, // unused
    0x0f, 0x10, 0x20, 0x30, // unused
};

PPU* ppu = reinterpret_cast<PPU*>(0x0000);
ChrMapper<1> chr1;

void init_ppu() {
    // Disable the PPU so we can freely modify its state
    ppu->off();

    // Set up bufferd VRAM operations (see `multi_vram_buffer_horz` below)
    set_vram_buffer();

    // Use lower half of PPU memory for background tiles
    bank_bg(0);

    // Set the background palette
    pal_bg(background_pal);

    // Fill the background with space characters to clear the screen
    vram_adr(NAMETABLE_A);
    vram_fill(' ', NameTable::size);

    // Write a message
    vram_adr(to_addr(ppu->nameTable0.addr(10, 10)));
    vram_write(hello, sizeof(hello) - 1);

    // Use the upper half of PPU memory for sprites
    bank_spr(1);

    // Set the sprite palette
    pal_spr(sprite_pal);

    // Turn the PPU back on
    ppu->on<PPU::Display::ALL>();
}

int main() {
    init_ppu();

    // Counters to cycle through palette colors, changing every half second
    uint8 palette_color = 0;
    uint8 counter = 0;

    // Start with the first sprite bank
    uint8 sprite_bank = 1;

    // Cogwheel position
    uint8 cog_x = 15 * PPU::tilePixelWidth;
    uint8 cog_y = 14 * PPU::tilePixelWidth;

    // Store pad state across frames to check for changes
    uint8 prev_pad_state = 0;

    for (;;) {
        // Wait for the NMI routine to end so we can start working on the next frame
        ppu->waitNMI();

        // Set the MMC1 to use the chosen CHR bank for the upper half of the PPU
        // pattern table. Do this first thing after NMI finishes so that we are
        // still in VBLANK.
        chr1.bank(sprite_bank);

        // The OAM (object attribute memory) is an area of RAM that contains data
        // about all the sprites that will be drawn next frame.
        oam_clear();

        // Note: if you don't poll a controller during a frame, emulators will
        // report that as lag
        const uint8 pad_state = pad_poll(0);

        // Speed up when pressing B
        const uint8 speed = pad_state & PAD_B ? 2 : 1;

        // Move the cogwheel in response to pad directions
        if (pad_state & PAD_UP) {
            cog_y -= speed;
        }
        else if (pad_state & PAD_DOWN) {
            cog_y += speed;
        }

        if (pad_state & PAD_LEFT) {
            cog_x -= speed;
        }
        else if (pad_state & PAD_RIGHT) {
            cog_x += speed;
        }

        if (pad_state & PAD_A) {
            // Create an explosion immediately when A is pressed, and then every 8
            // frames as long as A is held
            // `& 0x7` is equivalent to % `8`
            if (!(prev_pad_state & PAD_A) || !(get_frame_count() & 0x7)) {
                const uint8 x = cog_x + (rand8() & 0xF);
                const uint8 y = cog_y + 8 + (rand8() & 0xF);
                addExplosion(x, y);
            }
        }

        if (prev_pad_state & PAD_SELECT && !(pad_state & PAD_SELECT)) {
            // Select was released - swap CHR banks
            sprite_bank = sprite_bank == 1 ? 2 : 1;
        }

        prev_pad_state = pad_state;

        animateExplosions();

        // Adding Cogwheel after the explosions means the explosions will be prioritized
        for (uint8 row = 0; row < 3; ++row) {
            for (uint8 col = 0; col < 3; ++col) {
                // Convert row/col to pixels and add to cog position
                uint8 const sprite_x = cog_x + (col << 3);
                uint8 const sprite_y = cog_y + (row << 3);

                // There are 16 tiles per row; shift by 4
                uint8 const tile = (row << 4) + col;
                oam_spr(sprite_x, sprite_y, tile, 0);
            }
        }

        // Change the color every half second (60 fps)
        if (++counter == 30) {
            counter = 0;
            if (++palette_color == 64) palette_color = 0;
            pal_col(3, palette_color);

            // Print the current palette color in hex
            char buffer[4];
            std::snprintf(buffer, sizeof(buffer), "$%02x", palette_color);

            // Copy the text into the VRAM buffer. This will draw characters at the
            // given VRAM address during the next vertical blank period.
            multi_vram_buffer_horz(buffer, 3, to_addr(ppu->nameTable0.addr(14, 12)));
        }
    }

    return 0;
}
