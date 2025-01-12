#pragma once

#include "./util.h"

struct Block {
    mem_map region;

    Block(uint8* start, usize size): region(start, size) {}

    inline constexpr mem* begin() {
        return region.data();
    }
    inline constexpr mem* begin() const {
        return region.data();
    }

    inline constexpr mem* end() {
        return region.data() + region.size();
    }
    inline constexpr mem* end() const {
        return region.data() + region.size();
    }
}

struct PatternTable {
    static constexpr usize blockSize = 256 * PPU::tileSize;

    const std::array<Block, 2> blocks;

    PatternTable(uint8* start): blocks{
        Block(start, blockSize),
        Block(start + blockSize, blockSize)
    } {}

    inline constexpr mem* begin() {
        return blocks[0].begin();
    }
    inline constexpr mem* begin() const {
        return blocks[0].begin();
    }

    inline constexpr mem* end() {
        return blocks[blocks.size() - 1].end();
    }
    inline constexpr mem* end() const {
        return blocks[blocks.size() - 1].end();
    }

    enum class Block : uint8 {
        LEFT = 0,
        RIGHT = 1
    };

    enum class Plane : uint8 {
        LOWER = 0,
        UPPER = 1
    };

    inline constexpr uint8* addr(Block idx, Plane plane, uint8 tileIdx, uint8 row) {
        struct PT {
            uint8 padding: 4 = 0;
            uint8 tile: 8;
            uint8 plane : 1;
            uint8 row : 3;
        };
        static_assert(Address<PT>);

        const Address addr = {
            0,
            tileIdx,
            static_cast<uint8>(plane),
            row
        };

        const auto& block = blocks[static_cast<uint8>(idx)];
        const auto offset = reinterpret_cast<uint8>(addr);

        return block.data() + offset;
    }
}

struct AttributeTable : Block {
    static constexpr usize blockSize = 64; // 1 byte per 4x4 tile area

    AttributeTable(uint8* start): Block(start, blockSize) {}
}

struct NameTable : Block {
    static constexpr usize blockSize = 32 * 30; // 1 byte per tile

    AttributeTable attrTable;

    NameTable(uint8* start):
        Block(start, blockSize),
        attrTable(block.end()) {}
}

struct Palette : Block {
    static constexpr usize blockSize = numPalettes * numColors; // each byte is a color index

    static constexpr usize numColors = 4;
    static constexpr usize numPalettes = 4;

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

    Palette(uint8* start): Block(start, blockSize) {}

    template<uint8 idx>
    inline constexpr uint8* addr() requires (idx < numPalettes) {
        return begin() + idx;
    }
}

struct PPU {
    // pixels represent palette indices
    // 0 is always transparent/background
    static constexpr uint8 pixelBits = 2;
    // a tile is 8x8 pixels and 16 bytes
    // each tile is made up of 2 8 byte planes, lower and upper
    // each pixel in the tile has one bit in the same position in each plane
    // the lower color bit is in the lower plane,
    // the upper color bit is in the upper plane
    static constexpr uint8 tileSize = pixelBits * 8 * 8 / 8;

    PatternTable    patternTable(to_ptr(0x0000));

    NameTable       nameTable0(patternTable.end());
    NameTable       nameTable1(nameTable0.attrTable.end());
    NameTable       nameTable2(nameTable1.attrTable.end());
    NameTable       nameTable3(nameTable2.attrTable.end());

    Block           padding(nameTable3.attrTable.end(), 0xf00);

    Palette         bgPalette(padding.end());
    Palette         spritePalette(bgPalette.end());
}