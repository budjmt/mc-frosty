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

    enum class Color : uint16 {
        WHITE           = 0x30, // #ffffff
        LIGHT_GREY      = 0x3d, // #d6d6d6
        GREY            = 0x10, // #bdbdbd
        LIGHT_DARK_GREY = 0x00, // #6e6e6e
        DARK_GREY       = 0x2d, // #4a4a4a
        BLACK           = 0x3f, // #0a0a0a
        DARK_BLACK      = 0x2f, // #050505
        TRUE_BLACK      = 0x0f, // #000000
        BLUE1           = 0x01, // #00299c
        BLUE2           = 0x02, // #0b00a6
        BLUE11          = 0x11, // #0062ff
        BLUE12          = 0x12, // #0040ff
        BLUE21          = 0x21, // #00d4ff
        BLUE22          = 0x22, // #3892ff
        BLUE31          = 0x31, // #63ffff
        BLUE32          = 0x32, // #8aebff
        BLUE3C          = 0x3c, // #42fffc
        BLUE2C          = 0x2c, // #00ffff
        BLUE1C          = 0x1c, // #008bc7
        BLUEC           = 0x0c, // #003054
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