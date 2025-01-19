#pragma once

#include <array>

#include "util.h"

template<usize Length>
class alignas(1) Block {
    alignas(1) std::array<mem, Length> _data;
public:
    static constexpr usize size = Length;

    inline constexpr decltype(auto) data(this auto& self) {
        return self._data.data();
    }

    inline constexpr decltype(auto) begin(this auto& self) {
        return self.data();
    }
    inline constexpr decltype(auto) end(this auto& self) {
        return self.begin() + size;
    }

    inline constexpr decltype(auto) operator[](this auto& self, usize idx) {
        return self._data[idx];
    }
};

struct PPUConstants {
    // pixels represent palette indices
    // each tile is made up of 2 planes, lower and upper
    // each pixel in the tile has one bit in the same position in each plane
    // the lower color bit is in the lower plane, and the upper bit is in the upper plane
    static constexpr uint8 pixelBits = 2;
    // Tiles are 8x8 pixel squares
    static constexpr uint8 tilePixelWidth = 8;

    static constexpr uint8 tileSize = tilePixelWidth * tilePixelWidth * pixelBits / 8;

    static constexpr uint8 screenTileWidth = 32;
    static constexpr uint8 screenTileHeight = 30;
};

struct alignas(1) PatternTable {
    static constexpr usize numTiles = 256;

    static constexpr usize blockSize = numTiles * PPUConstants::tileSize;

    alignas(1) const std::array<::Block<blockSize>, 2> blocks;

    enum class Block : uint8 {
        LEFT = 0,
        RIGHT = 1
    };

    enum class Plane : uint8 {
        LOWER = 0,
        UPPER = 1
    };

    inline constexpr decltype(auto) addr(
        this auto& self,
        Block idx,
        Plane plane,
        uint8 tileIdx,
        uint8 row
    ) {
        struct PT {
            uint8 padding: 4 = 0;
            uint8 tile: 8;
            uint8 plane : 1;
            uint8 row : 3;
        };
        static_assert(Address<PT>);

        const PT addr = {
            0,
            tileIdx,
            static_cast<uint8>(plane),
            row
        };

        const auto& block = self.blocks[static_cast<uint8>(idx)];
        const auto offset = reinterpret_cast<uint8>(addr);

        return block.data() + offset;
    }
};

// 1 byte per 4x4 tile area
struct alignas(1) AttributeTable : Block<64> {
    // 0 is always transparent/background
};

// 1 byte per screen tile
struct alignas(1) NameTable : Block<
    PPUConstants::screenTileWidth *
    PPUConstants::screenTileHeight
> {
    AttributeTable attrTable;

    inline constexpr decltype(auto) addr(this auto& self, uint8 x, uint8 y) {
        // x is 5 bits because it's 32 tiles wide
        return self.begin() + ((y << 5) | x);
    }
};

struct PaletteConstants {
    static constexpr uint8 numColors = 4;
    static constexpr uint8 numPalettes = 4;
};

// each byte is a color index
struct alignas(1) Palette : PaletteConstants, Block<
    PaletteConstants::numColors *
    PaletteConstants::numPalettes
> {
    // colors vary based on the hardware in each unit,
    // so the color codes are just approximations
    enum class Color : uint16 {
        WHITE               = 0x30, // #ffffff
        LIGHT_GREY          = 0x3d, // #d6d6d6
        GREY                = 0x10, // #bdbdbd
        LIGHT_DARK_GREY     = 0x00, // #6e6e6e
        DARK_GREY           = 0x2d, // #4a4a4a
        DARK_DARK_GREY      = 0x1d, // #141414
        BLACK               = 0x3f, // #0a0a0a
        DARK_BLACK          = 0x2f, // #050505
        TRUE_BLACK          = 0x0f, // #000000
        DARK_BROWN          = 0x08, // #521d00
        BROWN               = 0x18, // #d14600
        DARK_RED            = 0x07, // #8f0000
        RASPBERRY           = 0x04, // #ab004a
        CAYENNE             = 0x06, // #cc0000
        SCARLET             = 0x05, // #de0012
        OFF_RED             = 0x17, // #f00000
        RED                 = 0x16, // #ff0000
        CRIMSON             = 0x15, // #ff0037
        PUMPKIN             = 0x26, // #ff6c0a
        ORANGE              = 0x27, // #ff8400
        CHEDDAR             = 0x28, // #ffaa00
        CANTALOUPE          = 0x36, // #ffc69e
        DARK_YELLOW         = 0x37, // #ffea8f
        YELLOW              = 0x38, // #fff782
        YELLOW_GREEN        = 0x39, // #c8e67c
        LAWN_GREEN          = 0x29, // #66e300
        GREEN               = 0x2a, // #00f500
        HULK_GREEN          = 0x1a, // #008200
        FOREST_GREEN        = 0x1b, // #007a3f
        CUCUMBER            = 0x19, // #007000
        DARK_GREEN_B        = 0x0b, // #00381f
        DARK_GREEN_A        = 0x0a, // #003800
        DARK_GREEN_9        = 0x09, // #003600
        SPRING_GREEN        = 0x2b, // #00f58f
        SEAFOAM_GREEN       = 0x3a, // #6ded9c
        AQUAMARINE          = 0x3b, // #64f5d1
        BABY_BLUE           = 0x31, // #63ffff
        TURQUOISE           = 0x3c, // #42fffc
        CYAN                = 0x2c, // #00ffff
        SKY_BLUE            = 0x32, // #8aebff
        NEON_BLUE           = 0x21, // #00d4ff
        CERULEAN            = 0x1c, // #008bc7
        AZURE               = 0x22, // #3892ff
        BLUE_RIBBON         = 0x11, // #0062ff
        BLUE                = 0x12, // #0040ff
        DARK_BLUE           = 0x01, // #00299c
        COBALT              = 0x02, // #0b00a6
        PRUSSIAN_BLUE       = 0x0c, // #003054
        INDIGO              = 0x03, // #3b0087
        VIOLET              = 0x13, // #830fff
        PURPLE              = 0x23, // #e561ff
        LAVENDER            = 0x33, // #e498eb
        PINK                = 0x34, // #ff8ffb
        DEEP_PINK           = 0x25, // #ff3877
        HOT_PINK            = 0x14, // #ff00aa
        MAGENTA             = 0x24, // #ff00f7
        SALMON              = 0x35, // #ff94a2
    };

    template<uint8 idx>
    inline constexpr decltype(auto) addr(this auto& self) requires (idx < numPalettes) {
        return self.begin() + idx;
    }
};

struct alignas(1) PPU : PPUConstants {
    // wait for the next TV-level frame (depending on PAL or NTSC)
    inline void waitNMI() const {
        ppu_wait_nmi();
    }

    // TODO:
    inline void setMask(uint8 mask) {
        ppu_mask(mask);
    }

    enum class Display : uint8 {
        BG = 0x01,
        SPR = 0x10,
        ALL = BG | SPR
    };

    // enable parts of the render
    template<Display mode = Display::ALL>
    inline void on() {
        if constexpr (mode == Display::ALL) {
            ppu_on_all();
        }
        else if (mode == Display::BG) {
            ppu_on_bg();
        }
        else if (mode == Display::SPR) {
            ppu_on_spr();
        }
        else {
            static_assert(false, "Invalid display mode");
        }
    }

    // Disable all rendering, NMI is still enabled
    inline void off() {
        ppu_off();
    }

    // Set the color emphasis bits
    inline void colorEmphasis(uint8 color) {
        color_emphasis(color);
    }

    enum class TVType : uint8 {
        PAL = 0,
        NTSC = 1
    };
    inline TVType getTV() const {
        return ppu_system() == 0 ? TVType::PAL : TVType::NTSC;
    }

    PatternTable    patternTable;
    NameTable       nameTable0;
    NameTable       nameTable1;
    NameTable       nameTable2;
    NameTable       nameTable3;
    alignas(1) Block<0xf00> padding;
    Palette         bgPalette;
    Palette         spritePalette;
};