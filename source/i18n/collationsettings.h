/*
*******************************************************************************
* Copyright (C) 2013, International Business Machines
* Corporation and others.  All Rights Reserved.
*******************************************************************************
* collationsettings.h
*
* created on: 2013feb07
* created by: Markus W. Scherer
*/

#ifndef __COLLATIONSETTINGS_H__
#define __COLLATIONSETTINGS_H__

#include "unicode/utypes.h"

#if !UCONFIG_NO_COLLATION

#include "unicode/ucol.h"

U_NAMESPACE_BEGIN

/**
 * Collation settings/options/attributes.
 * These are the values that can be changed via API.
 */
struct U_I18N_API CollationSettings : public UMemory {
    /**
     * Options bit 0: Perform the FCD check on the input text and deliver normalized text.
     */
    static const int32_t CHECK_FCD = 1;
    /**
     * Options bit 1: Numeric collation.
     * Also known as CODAN = COllate Digits As Numbers.
     *
     * Treat digit sequences as numbers with CE sequences in numeric order,
     * rather than returning a normal CE for each digit.
     */
    static const int32_t NUMERIC = 2;
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
     * Options bits 7..4: The 4-bit maxVariable value bit field is shifted by this value.
     */
    static const int32_t MAX_VARIABLE_SHIFT = 4;
    /** maxVariable options bit mask before shifting. */
    static const int32_t MAX_VARIABLE_MASK = 0xf0;
    /**
     * Options bit 8: Sort uppercase first if caseLevel or caseFirst is on.
     */
    static const int32_t UPPER_FIRST = 0x100;
    /**
     * Options bit 9: Keep the case bits in the tertiary weight (they trump other tertiary values)
     * unless case level is on (when they are *moved* into the separate case level).
     * By default, the case bits are removed from the tertiary weight (ignored).
     *
     * When CASE_FIRST is off, UPPER_FIRST must be off too, corresponding to
     * the tri-value UCOL_CASE_FIRST attribute: UCOL_OFF vs. UCOL_LOWER_FIRST vs. UCOL_UPPER_FIRST.
     */
    static const int32_t CASE_FIRST = 0x200;
    /**
     * Options bit mask for caseFirst and upperFirst, before shifting.
     * Same value as caseFirst==upperFirst.
     */
    static const int32_t CASE_FIRST_AND_UPPER_MASK = CASE_FIRST | UPPER_FIRST;
    /**
     * Options bit 10: Insert the case level between the secondary and tertiary levels.
     */
    static const int32_t CASE_LEVEL = 0x400;
    /**
     * Options bit 11: Compare secondary weights backwards. ("French secondary")
     */
    static const int32_t BACKWARD_SECONDARY = 0x800;
    /**
     * Options bits 15..12: The 4-bit strength value bit field is shifted by this value.
     * It is the top used bit field in the options. (No need to mask after shifting.)
     */
    static const int32_t STRENGTH_SHIFT = 12;
    /** Strength options bit mask before shifting. */
    static const int32_t STRENGTH_MASK = 0xf000;

    /** maxVariable values */
    enum MaxVariable {
        MAX_VAR_SPACE,
        MAX_VAR_PUNCT,
        MAX_VAR_SYMBOL,
        MAX_VAR_CURRENCY
    };

    CollationSettings()
            : options((UCOL_DEFAULT_STRENGTH << STRENGTH_SHIFT) |
                      (MAX_VAR_PUNCT << MAX_VARIABLE_SHIFT)),
              variableTop(0),
              reorderTable(NULL),
              reorderCodes(NULL), reorderCodesLength(0) {}

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

    UColAttributeValue getCaseFirst() const {
        int32_t option = options & CASE_FIRST_AND_UPPER_MASK;
        return (option == 0) ? UCOL_OFF :
                (option == CASE_FIRST) ? UCOL_LOWER_FIRST : UCOL_UPPER_FIRST;
    }

    void setAlternateHandling(UColAttributeValue value,
                              int32_t defaultOptions, UErrorCode &errorCode);

    UColAttributeValue getAlternateHandling() const {
        return ((options & ALTERNATE_MASK) == 0) ? UCOL_NON_IGNORABLE : UCOL_SHIFTED;
    }

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

    inline UBool dontCheckFCD() const {
        return (options & CHECK_FCD) == 0;
    }

    inline UBool hasBackwardSecondary() const {
        return (options & BACKWARD_SECONDARY) != 0;
    }

    inline UBool isNumeric() const {
        return (options & NUMERIC) != 0;
    }

    /** CHECK_FCD etc. */
    int32_t options;
    /** Variable-top primary weight. */
    uint32_t variableTop;
    /** 256-byte table for reordering permutation of primary lead bytes; NULL if no reordering. */
    const uint8_t *reorderTable;
    /** Array of reorder codes; NULL if no reordering. */
    const int32_t *reorderCodes;
    int32_t reorderCodesLength;
};

U_NAMESPACE_END

#endif  // !UCONFIG_NO_COLLATION
#endif  // __COLLATIONSETTINGS_H__