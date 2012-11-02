/*
*******************************************************************************
* Copyright (C) 2010-2012, International Business Machines
* Corporation and others.  All Rights Reserved.
*******************************************************************************
* collation.h
*
* created on: 2010oct27
* created by: Markus W. Scherer
*/

#ifndef __COLLATION_H__
#define __COLLATION_H__

#include "unicode/utypes.h"

#if !UCONFIG_NO_COLLATION

U_NAMESPACE_BEGIN

/**
 * Collation v2 basic definitions and static helper functions.
 *
 * Data structures except for expansion tables store 32-bit CEs which are
 * either specials (see tags below) or are compact forms of 64-bit CEs.
 */
class U_I18N_API Collation {
public:
    // Special sort key bytes for all levels.
    static const uint8_t TERMINATOR_BYTE = 0;
    static const uint8_t LEVEL_SEPARATOR_BYTE = 1;
    /**
     * Merge-sort-key separator.
     * Must not be used as the lead byte of any CE weight,
     * nor as primary compression low terminator.
     * Otherwise usable.
     */
    static const uint8_t MERGE_SEPARATOR_BYTE = 2;
    static const uint32_t MERGE_SEPARATOR_PRIMARY = 0x02000000;  // U+FFFE
    static const uint32_t MERGE_SEPARATOR_WEIGHT16 = 0x0200;  // U+FFFE
    static const uint32_t MERGE_SEPARATOR_LOWER32 = 0x02000200;  // U+FFFE
    static const uint32_t MERGE_SEPARATOR_CE32 = 0x02000202;  // U+FFFE

    /**
     * Primary compression low terminator, must be greater than MERGE_SEPARATOR_BYTE.
     * Reserved value in primary second byte if the lead byte is compressible.
     * Otherwise usable in all CE weight bytes.
     */
    static const uint8_t PRIMARY_COMPRESSION_LOW_BYTE = 3;
    /**
     * Primary compression high terminator.
     * Reserved value in primary second byte if the lead byte is compressible.
     * Otherwise usable in all CE weight bytes.
     */
    static const uint8_t PRIMARY_COMPRESSION_HIGH_BYTE = 0xff;

    /** Default secondary/tertiary weight lead byte. */
    static const uint8_t COMMON_BYTE = 5;
    static const uint32_t COMMON_WEIGHT16 = 0x0500;
    /** Middle 16 bits of a CE with a common secondary weight. */
    static const uint32_t COMMON_SECONDARY_CE = 0x05000000;
    /** Lower 16 bits of a CE with a common tertiary weight. */
    static const uint32_t COMMON_TERTIARY_CE = 0x0500;
    /** Lower 32 bits of a CE with common secondary and tertiary weights. */
    static const uint32_t COMMON_SEC_AND_TER_CE = 0x05000500;

    /** Only the 2*6 bits for the pure tertiary weight. */
    static const uint32_t ONLY_TERTIARY_MASK = 0x3f3f;
    /** Case bits and tertiary bits. */
    static const uint32_t CASE_AND_TERTIARY_MASK = 0xff3f;

    static const uint8_t UNASSIGNED_IMPLICIT_BYTE = 0xfd;  // compressible

    static const uint8_t TRAIL_WEIGHT_BYTE = 0xfe;  // not compressible
    static const uint32_t MAX_PRIMARY = 0xfeff0000;  // U+FFFF
    static const uint32_t MAX_REGULAR_CE32 = 0xfeff0505;  // U+FFFF

    /** Primary lead byte for special tags, not used as a primary lead byte in resolved CEs. */
    static const uint8_t SPECIAL_BYTE = 0xff;

    /**
     * The lowest "special" CE32 value.
     * This value itself is used to indicate a fallback to the base collator,
     * regardless of the semantics of its tag bit field,
     * to minimize the fastpath lookup code.
     */
    static const uint32_t MIN_SPECIAL_CE32 = 0xff000000;

    static const uint32_t UNASSIGNED_CE32 = 0xffffffff;  // Compute an unassigned-implicit CE.

    /** No CE: End of input. Only used in runtime code, not stored in data. */
    static const uint32_t NO_CE_PRIMARY = 1;  // not a left-adjusted weight
    static const uint32_t NO_CE_WEIGHT16 = 0x0100;  // weight of LEVEL_SEPARATOR_BYTE
    static const int64_t NO_CE = 0x101000100;  // NO_CE_PRIMARY, NO_CE_WEIGHT16, NO_CE_WEIGHT16

    /** Sort key levels. */
    enum Level {
        /** Unspecified level. */
        NO_LEVEL,
        PRIMARY_LEVEL,
        SECONDARY_LEVEL,
        CASE_LEVEL,
        TERTIARY_LEVEL,
        QUATERNARY_LEVEL,
        IDENTICAL_LEVEL,
        /** Beyond sort key bytes. */
        ZERO_LEVEL
    };

    /**
     * Sort key level flags: xx_FLAG = 1 << xx_LEVEL.
     * In Java, use enum Level with flag() getters, or use EnumSet rather than hand-made bit sets.
     */
    static const uint32_t NO_LEVEL_FLAG = 1;
    static const uint32_t PRIMARY_LEVEL_FLAG = 2;
    static const uint32_t SECONDARY_LEVEL_FLAG = 4;
    static const uint32_t CASE_LEVEL_FLAG = 8;
    static const uint32_t TERTIARY_LEVEL_FLAG = 0x10;
    static const uint32_t QUATERNARY_LEVEL_FLAG = 0x20;
    static const uint32_t IDENTICAL_LEVEL_FLAG = 0x40;
    static const uint32_t ZERO_LEVEL_FLAG = 0x80;

    /**
     * Special-CE32 tags, from bits 23..20 of a special 32-bit CE.
     * Bits 19..0 are used for data.
     */
    enum {
        /**
         * Tags 0..5 are used for Latin mini expansions
         * of two simple CEs [pp, 05, tt] [00, ss, 05].
         * Bits 23..16: Single-byte primary weight pp=00..5F of the first CE.
         * Bits 15.. 8: Tertiary weight tt of the first CE.
         * Bits  7.. 0: Secondary weight ss of the second CE.
         */
        MAX_LATIN_EXPANSION_TAG = 5,
        /**
         * Points to one or more non-special 32-bit CE32s.
         * Bits 19..3: Index into uint32_t table.
         * Bits  2..0: Length. If length==0 then the actual length is in the first unit.
         */
        EXPANSION32_TAG = 6,
        /**
         * Points to one or more 64-bit CEs.
         * Bits 19..3: Index into CE table.
         * Bits  2..0: Length. If length==0 then the actual length is in the first unit.
         */
        EXPANSION_TAG = 7,
        /**
         * Points to prefix trie.
         * Bits 19..0: Index into prefix/contraction data.
         */
        PREFIX_TAG = 8,
        /**
         * Points to contraction data.
         * Bits 19..2: Index into prefix/contraction data.
         * Bit      1: Set if the first character of every contraction suffix is >=U+0300.
         * Bit      0: Set if any contraction suffix ends with cc != 0.
         */
        CONTRACTION_TAG = 9,
#if 0  // TODO
        /**
         * Used only in the collation data builder.
         * Data bits point into a builder-specific data structure with non-final data.
         */
        BUILDER_CONTEXT_TAG = 10,
#endif
        /**
         * Decimal digit.
         * Bits 19..4: Index into uint32_t table for non-CODAN CE32.
         * Bits  3..0: Digit value 0..9.
         */
        DIGIT_TAG = 10,
        /**
         * Unused.
         */
        RESERVED_TAG_11 = 11,
        /**
         * Tag for a Hangul syllable.
         */
        HANGUL_TAG = 12,
        /**
         * Tag for a lead surrogate code unit.
         * Optional optimization for UTF-16 string processing.
         * Bits 19..2: Unused, 0.
         *       1..0: =0: All associated supplementary code points are unassigned-implict.
         *             =1: All associated supplementary code points fall back to the base data.
         *           else: (Normally 2) Look up the data for the supplementary code point.
         */
        LEAD_SURROGATE_TAG = 13,
        /**
         * Tag for CEs with primary weights in code point order.
         * Bits 19..0: Index into CE table, for one data "CE".
         *
         * This data "CE" has the following bit fields:
         * Bits 63..32: Three-byte primary pppppp00.
         *      31.. 8: Start/base code point of the in-order range.
         *           7: Flag isCompressible primary.
         *       6.. 0: Per-code point primary-weight increment.
         */
        OFFSET_TAG = 14,
        /**
         * Implicit CE tag. Compute an unassigned-implicit CE.
         * Also used for U+0000, for moving the NUL-termination handling
         * from the regular fastpath into specials-handling code.
         *
         * The data bits are 0 for U+0000, otherwise 0xfffff (UNASSIGNED_CE32=0xffffffff).
         */
        IMPLICIT_TAG = 15
    };

    static inline UBool isSpecialCE32(uint32_t ce32) {
        // Java: Emulate unsigned-int less-than comparison.
        // return (ce32^0x80000000)>=0x7f000000;
        return ce32 >= MIN_SPECIAL_CE32;
    }

    static inline int32_t getSpecialCE32Tag(uint32_t ce32) {
        return (int32_t)((ce32 >> 20) & 0xf);
    }

    /** Returns a 64-bit CE from a non-special CE32. */
    static inline int64_t ceFromCE32(uint32_t ce32) {
        uint32_t tertiary = ce32 & 0xff;
        if(tertiary > 1) {
            // normal form ppppsstt -> pppp0000ss00tt00
            return ((int64_t)(ce32 & 0xffff0000) << 32) | ((ce32 & 0xff00) << 16) | (tertiary << 8);
        } else if(tertiary == 1) {
            // long-primary form pppppp01 -> pppppp00050000500
            return ((int64_t)(ce32 - 1) << 32) | COMMON_SEC_AND_TER_CE;
        } else /* tertiary == 0 */ {
            // long-secondary form sssstt00 -> 00000000sssstt00,
            // including the tertiary-ignorable, all-zero CE
            // Java: Use a mask to work around sign extension.
            // return (long)ce32&0xffffffff;
            return ce32;
        }
    }

    /** Is ce32 a long-primary pppppp01? */
    static inline UBool isLongPrimaryCE32(uint32_t ce32) {
        return !isSpecialCE32(ce32) && (ce32 & 0xff) == 1;
    }

    /** Turns the long-primary CE32 into a primary weight pppppp00. */
    static inline uint32_t primaryFromLongPrimaryCE32(uint32_t ce32) {
        return ce32 - 1;
    }

    /**
     * Increments a 3-byte primary by a code point offset.
     */
    static uint32_t incThreeBytePrimaryByOffset(uint32_t basePrimary, UBool isCompressible,
                                                int32_t offset);

    /**
     * Computes a 3-byte primary for c's OFFSET_TAG data "CE".
     */
    static uint32_t getThreeBytePrimaryForOffsetData(UChar32 c, int64_t dataCE);

    /**
     * Returns the unassigned-character implicit primary weight for any valid code point c.
     */
    static uint32_t unassignedPrimaryFromCodePoint(UChar32 c);
    // TODO: Set [first unassigned] to unassignedPrimaryFromCodePoint(-1).
    // TODO: Set [last unassigned] to unassignedPrimaryFromCodePoint(0x10ffff).

    static inline int64_t unassignedCEFromCodePoint(UChar32 c) {
        int64_t ce = unassignedPrimaryFromCodePoint(c);
        return (ce << 32) | COMMON_SEC_AND_TER_CE;
    }

    static inline uint32_t reorder(const uint8_t reorderTable[256], uint32_t primary) {
        return ((uint32_t)reorderTable[primary >> 24] << 24) | (primary & 0xffffff);
    }

private:
    Collation();  // No instantiation.
};

U_NAMESPACE_END

#endif  // !UCONFIG_NO_COLLATION
#endif  // __COLLATION_H__