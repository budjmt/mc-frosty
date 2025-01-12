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
    static constexpr usize blockSize = 16; // each byte is 1 / 256 colors

    static constexpr usize numColors = 4;

    // colors vary based on the hardware in each unit,
    // so the color codes are just approximations
    enum class Color : uint16 {
        WHITE           = 0x30, // #ffffff
        LIGHT_GREY      = 0x3d, // #d6d6d6
        GREY            = 0x10, // #bdbdbd
        LIGHT_DARK_GREY = 0x00, // #6e6e6e
        DARK_GREY       = 0x2d, // #4a4a4a
        DARK_DARK_GREY  = 0x1d, // #141414
        BLACK           = 0x3f, // #0a0a0a
        DARK_BLACK      = 0x2f, // #050505
        TRUE_BLACK      = 0x0f, // #000000
        DARK_BROWN      = 0x08, // #521d00
        BROWN           = 0x18, // #d14600
        RED_4           = 0x04, // #ab004a
        RED_5           = 0x05, // #de0012
        RED_6           = 0x06, // #cc0000
        RED_7           = 0x07, // #8f0000
        RED_15          = 0x15, // #ff0037
        RED             = 0x16, // #ff0000
        RED_17          = 0x17, // #f00000
        ORANGE_26       = 0x26, // #ff6c0a
        ORANGE_27       = 0x27, // #ff8400
        ORANGE_28       = 0x28, // #ffaa00
        ORANGE_36       = 0x36, // #ffc69e
        YELLOW_37       = 0x37, // #ffea8f
        YELLOW_38       = 0x38, // #fff782
        GREEN_39        = 0x39, // #c8e67c
        GREEN_29        = 0x29, // #66e300
        GREEN_2A        = 0x2a, // #00f500
        GREEN_19        = 0x19, // #007000
        GREEN_1A        = 0x1a, // #008200
        GREEN_1B        = 0x1b, // #007a3f
        GREEN_9         = 0x09, // #003600
        GREEN_A         = 0x0a, // #003800
        GREEN_B         = 0x0b, // #00381f
        TEAL_2B         = 0x2b, // #00f58f
        TEAL_3A         = 0x3a, // #6ded9c
        TEAL_3B         = 0x3b, // #64f5d1
        BLUE_3C         = 0x3c, // #42fffc
        BLUE_32         = 0x32, // #8aebff
        BLUE_31         = 0x31, // #63ffff
        BLUE_2C         = 0x2c, // #00ffff
        BLUE_22         = 0x22, // #3892ff
        BLUE_21         = 0x21, // #00d4ff
        BLUE_1C         = 0x1c, // #008bc7
        BLUE_12         = 0x12, // #0040ff
        BLUE_11         = 0x11, // #0062ff
        BLUE_C          = 0x0c, // #003054
        BLUE_2          = 0x02, // #0b00a6
        BLUE_1          = 0x01, // #00299c
        VIOLET_3        = 0x03, // #3b0087
        VIOLET_13       = 0x13, // #830fff
        PINK_14         = 0x14, // #ff00aa
        PINK_23         = 0x23, // #e561ff
        PINK_24         = 0x24, // #ff00f7
        PINK_25         = 0x25, // #ff3877
        PINK_33         = 0x33, // #e498eb
        PINK_34         = 0x34, // #ff8ffb
        PINK_35         = 0x35, // #ff94a2
    };

    Palette(uint8* start): Block(start, blockSize) {}

    template<uint8 idx>
    inline constexpr uint8* addr() requires (idx < blockSize / numColors) {
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