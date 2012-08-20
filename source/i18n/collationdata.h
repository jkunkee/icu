/*
*******************************************************************************
* Copyright (C) 2010-2012, International Business Machines
* Corporation and others.  All Rights Reserved.
*******************************************************************************
* collationdata.h
*
* created on: 2010oct27
* created by: Markus W. Scherer
*/

#ifndef __COLLATIONDATA_H__
#define __COLLATIONDATA_H__

#include "unicode/utypes.h"

#if !UCONFIG_NO_COLLATION

#include "unicode/ucol.h"
#include "unicode/uniset.h"
#include "collation.h"
#include "normalizer2impl.h"
#include "utrie2.h"

U_NAMESPACE_BEGIN

/**
 * Collation data container.
 */
struct U_I18N_API CollationData : public UMemory {
    /**
     * Options bit 0: Perform the FCD check on the input text and deliver normalized text.
     */
    static const int32_t CHECK_FCD = 1;
    /**
     * Options bit 1: COllate Digits As Numbers.
     * Treat digit sequences as numbers with CE sequences in numeric order,
     * rather than returning a normal CE for each digit.
     */
    static const int32_t CODAN = 2;
    /**
     * "Shifted" alternate handling, see ALTERNATE_MASK.
     */
    static const int32_t SHIFTED = 4;
    /**
     * Options bits 3..2: Alternate-handling mask. 0 for non-ignorable.
     * Reserve values 8 and 0xc for shift-trimmed and blanked.
     */
    static const int32_t ALTERNATE_MASK = 0xc;
    /**
     * Options bit 4: Sort uppercase first if caseLevel or caseFirst is on.
     */
    static const int32_t UPPER_FIRST = 0x10;
    /**
     * Options bit 5: Keep the case bits in the tertiary weight (they trump other tertiary values)
     * unless case level is on (when they are *moved* into the separate case level).
     * By default, the case bits are removed from the tertiary weight (ignored).
     *
     * When CASE_FIRST is off, UPPER_FIRST must be off too, corresponding to
     * the tri-value UCOL_CASE_FIRST attribute: UCOL_OFF vs. UCOL_LOWER_FIRST vs. UCOL_UPPER_FIRST.
     */
    static const int32_t CASE_FIRST = 0x20;
    /**
     * Options bit mask for caseFirst and upperFirst, before shifting.
     * Same value as caseFirst==upperFirst.
     */
    static const int32_t CASE_FIRST_AND_UPPER_MASK = CASE_FIRST | UPPER_FIRST;
    /**
     * Options bit 6: Insert the case level between the secondary and tertiary levels.
     */
    static const int32_t CASE_LEVEL = 0x40;
    /**
     * Options bit 7: Compare secondary weights backwards. ("French secondary")
     */
    static const int32_t BACKWARD_SECONDARY = 0x80;
    /**
     * Options bits 11..8: The 4-bit strength value bit field is shifted by this value.
     * It is the top used bit field in the options. (No need to mask after shifting.)
     */
    static const int32_t STRENGTH_SHIFT = 8;
    /** Strength options bit mask before shifting. */
    static const int32_t STRENGTH_MASK = 0xf00;

    CollationData(const Normalizer2Impl &nfc)
            : trie(NULL),
              ce32s(NULL), ces(NULL), contexts(NULL), base(NULL),
              jamoCEs(NULL),
              nfcImpl(nfc),
              options(UCOL_DEFAULT_STRENGTH << STRENGTH_SHIFT),
              variableTop(0), zeroPrimary(0x12000000),
              compressibleBytes(NULL), reorderTable(NULL),
              reorderCodes(NULL), reorderCodesLength(0),
              unsafeBackwardSet(NULL) {}

    void setStrength(int32_t value, int32_t defaultOptions, UErrorCode &errorCode);

    static int32_t getStrength(int32_t options) {
        return options >> STRENGTH_SHIFT;
    }

    int32_t getStrength() const {
        return getStrength(options);
    }

    /** Sets the options bit for an on/off attribute. */
    void setFlag(int32_t bit, UColAttributeValue value,
                 int32_t defaultOptions, UErrorCode &errorCode);

    UColAttributeValue getFlag(int32_t bit) const {
        return ((options & bit) != 0) ? UCOL_ON : UCOL_OFF;
    }

    void setCaseFirst(UColAttributeValue value, int32_t defaultOptions, UErrorCode &errorCode);

    void setAlternateHandling(UColAttributeValue value,
                              int32_t defaultOptions, UErrorCode &errorCode);

    static uint32_t getTertiaryMask(int32_t options) {
        // Remove the case bits from the tertiary weight when caseLevel is on or caseFirst is off.
        return ((options & (CASE_LEVEL | CASE_FIRST)) == CASE_FIRST) ?
                Collation::CASE_AND_TERTIARY_MASK : Collation::ONLY_TERTIARY_MASK;
    }

    static UBool sortsTertiaryUpperCaseFirst(int32_t options) {
        // On tertiary level, consider case bits and sort uppercase first
        // if caseLevel is off and caseFirst==upperFirst.
        return (options & (CASE_LEVEL | CASE_FIRST_AND_UPPER_MASK)) == CASE_FIRST_AND_UPPER_MASK;
    }

    uint32_t getCE32(UChar32 c) const {
        return UTRIE2_GET32(trie, c);
    }

    uint32_t getCE32FromSupplementary(UChar32 c) const {
        return UTRIE2_GET32_FROM_SUPP(trie, c);
    }

#if 0
    // TODO: Try v1 approach of building a runtime CollationData instance for canonical closure,
    // rather than using the builder and its dynamic data structures for lookups.
    // If this is acceptable, then we can revert the BUILDER_CONTEXT_TAG to a RESERVED_TAG.
    /**
     * Resolves the ce32 with a BUILDER_CONTEXT_TAG into another CE32.
     */
    virtual uint32_t nextCE32FromBuilderContext(CollationIterator &iter, uint32_t ce32,
                                                UErrorCode &errorCode) const {
        if(U_SUCCESS(errorCode)) { errorCode = U_INTERNAL_PROGRAM_ERROR; }
        return 0;
    }
#endif

    UBool isUnsafeBackward(UChar32 c) const {
        return unsafeBackwardSet->contains(c);
    }

    UBool isCompressibleLeadByte(uint32_t b) const {
        return compressibleBytes[b];
    }

    inline UBool isCompressiblePrimary(uint32_t p) const {
        return isCompressibleLeadByte(p >> 24);
    }

    /**
     * Returns the FCD16 value for code point c. c must be >= 0.
     */
    uint16_t getFCD16(UChar32 c) const {
        return nfcImpl.getFCD16(c);
    }

    /** Main lookup trie. */
    const UTrie2 *trie;
    /**
     * Array of CE32 values.
     * At index 0 there must be CE32(U+0000)
     * which has a special-tag for NUL-termination handling.
     */
    const uint32_t *ce32s;
    /** Array of CE values for expansions and OFFSET_TAG. */
    const int64_t *ces;
    /** Array of prefix and contraction-suffix matching data. */
    const UChar *contexts;
    /** Base collation data, or NULL if this data itself is a base. */
    const CollationData *base;
    /**
     * Simple array of 19+21+27 CEs, one per canonical Jamo L/V/T.
     * For fast handling of HANGUL_TAG.
     */
    const int64_t *jamoCEs;
        // TODO
        // Build & return a simple array of CEs.
        // Tailoring: Only necessary if Jamos are tailored.
    const Normalizer2Impl &nfcImpl;
    /** Collation::CHECK_FCD etc. */
    int32_t options;
    /** Variable-top primary weight. 0 if "shifted" mode is off. */
    uint32_t variableTop;
    /** The single-byte primary weight (xx000000) for '0' (U+0030). */
    uint32_t zeroPrimary;
    /** 256 flags for which primary-weight lead bytes are compressible. */
    const UBool *compressibleBytes;
    /** 256-byte table for reordering permutation of primary lead bytes; NULL if no reordering. */
    const uint8_t *reorderTable;
    /** Array of reorder codes; NULL if no reordering. */
    const int32_t *reorderCodes;
    int32_t reorderCodesLength;
    /**
     * Set of code points that are unsafe for starting string comparison after an identical prefix,
     * or in backwards CE iteration.
     */
    const UnicodeSet *unsafeBackwardSet;
};

/**
 * Collation data container with additional data for the collation base (root/default).
 */
struct U_I18N_API CollationBaseData : public CollationData {
public:
    CollationBaseData(const Normalizer2Impl &nfc)
            : CollationData(nfc),
              scripts(NULL), scriptsLength(0) {}

    /**
     * @return the lowest primary weight for the script,
     *         or 0 if the script does not occur in the data
     */
    uint32_t getLowestPrimaryForScript(int32_t script) const;

    int32_t getEquivalentScripts(int32_t script,
                                 int32_t dest[], int32_t capacity, UErrorCode &errorCode) const;

    /**
     * Writes the permutation table for the given reordering of scripts and groups,
     * mapping from default-order primary-weight lead bytes to reordered lead bytes.
     * The caller checks for illegal arguments and
     * takes care of [DEFAULT] and memory allocation.
     */
    void makeReorderTable(const int32_t *reorder, int32_t length,
                          uint8_t table[256], UErrorCode &errorCode) const;

    /**
     * Data for scripts and reordering groups.
     * Uses include building a reordering permutation table and
     * providing script boundaries to AlphabeticIndex.
     *
     * This data is a sorted list of primary-weight lead byte ranges (reordering groups),
     * each with a list of pairs sorted in base collation order;
     * each pair contains a script/reorder code and the lowest primary weight for that script.
     *
     * Data structure:
     * - Each reordering group is encoded in n+1 integers.
     *   - First integer:
     *     Bits 31..24: First byte of the reordering group's range.
     *     Bits 23..16: Last byte of the reordering group's range.
     *     Bits  7.. 0: Length n of the list of primary/script pairs.
     *   - Each pair is an integer with the at-most-three-byte lowest primary weight for the script,
     *     and the script/reorder code in the low byte, encoded by scriptByteFromInt().
     */
    const uint32_t *scripts;
    int32_t scriptsLength;

    /**
     * Maps a script or reorder code to a byte value.
     * When we need to represent script codes 248 and higher,
     * or reorder codes 0x1008 and higher,
     * then we need to make an incompatible change to this mapping and
     * thus to the CollationBaseData data structure.
     */
    static int32_t scriptByteFromInt(int32_t script) {
        if(script < SCRIPT_BYTE_LIMIT) { return script; }
        script -= UCOL_REORDER_CODE_FIRST;
        if(0 <= script && script <= (0xff - SCRIPT_BYTE_LIMIT)) {
            return SCRIPT_BYTE_LIMIT + script;
        }
        return USCRIPT_INVALID_CODE;  // -1
    }
    /** Inverse of scriptByteFromInt(). */
    static int32_t scriptIntFromByte(int32_t b) {
        // assert 0 <= b <= 0xff
        return b < SCRIPT_BYTE_LIMIT ? b : UCOL_REORDER_CODE_FIRST + b - SCRIPT_BYTE_LIMIT;
    }

private:
    int32_t findScript(int32_t script) const;

    /**
     * Constant for scriptByteFromInt() and scriptIntFromByte().
     * Codes for scripts encoded in Unicode (e.g., USCRIPT_GREEK) must be below this limit.
     * Reorder codes (e.g., UCOL_REORDER_CODE_PUNCTUATION) are offset to start from here,
     * so that UCOL_REORDER_CODE_FIRST maps to this value.
     * Changing this value changes the collation base data format.
     */
    static const int32_t SCRIPT_BYTE_LIMIT = 0xf8;
};

U_NAMESPACE_END

#endif  // !UCONFIG_NO_COLLATION
#endif  // __COLLATIONDATA_H__