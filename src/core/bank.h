#pragma once

#include <mapper.h>

#include "util.h"

class Mapper {
protected:
    uint8 _bank;
public:
    Mapper(uint8 bank): _bank(bank) {}

    virtual void bank(uint8 bank) = 0;
    inline uint8 bank() const {
        return _bank;
    }
};

struct PrgMapper : Mapper {
    PrgMapper(): Mapper(get_prg_bank()) {}

    inline void bank(uint8 bank) override {
        if (bank == _bank) return;
        set_prg_bank(bank);
        _bank = bank;
    }
};

template<uint8 chunk>
struct ChrMapper : Mapper {
    static_assert(chunk == 0 || chunk == 1,
        "chunk must be 0 or 1");

    // TODO:
    ChrMapper(): Mapper(0) {
        // if constexpr (chunk == 0) {
        //     _bank = get_chr_bank_0();
        // }
        // else {
        //     _bank = get_chr_bank_1();
        // }
    }

    inline void bank(uint8 bank) override {
        if (bank == _bank) return;
        if constexpr (chunk == 0) {
            set_chr_bank_0(bank);
        }
        else {
            set_chr_bank_1(bank);
        }
        _bank = bank;
    }

    // Switches the bank immediately, persisting across NMIs
    // and retrying if interrupted
    inline void bankWithRetry(uint8 bank) {
        if (bank == _bank) return;
        if constexpr (chunk == 0) {
            set_chr_bank_0_retry(bank);
        }
        else {
            set_chr_bank_1_retry(bank);
        }
        _bank = bank;
    }

    // Switches the bank immediately, but will be
    // overwritten by the normal bank on the next frame
    inline void bankSplit(uint8 bank) {
        // don't cache the temp bank
        // it will be overwritten by the normal bank on the next frame
        if constexpr (chunk == 0) {
            split_chr_bank_0(bank);
        }
        else {
            split_chr_bank_1(bank);
        }
    }
};