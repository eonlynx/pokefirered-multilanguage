#ifndef GUARD_CONSTANTS_LOCALIZED_TEXT_H
#define GUARD_CONSTANTS_LOCALIZED_TEXT_H

#include "localized_strings.h"

// Core-series language
enum PlayerLanguage
{
    LANG_EN,
    LANG_DE,
    LANG_FR,
    LANG_ES,
    LANG_IT,
    LANG_COUNT
};

struct LocalizedText
{
    const u8 *base;
    const u8 *translations[LANG_COUNT];
};

static const struct LocalizedText sLocalizedTexts[] =
{
    {
        .base = PalletTown_Text_OakGrassUnsafeNeedMon,
        .translations =
        {
            [LANG_EN] = PalletTown_Text_OakGrassUnsafeNeedMon,
            [LANG_DE] = PalletTown_Text_OakGrassUnsafeNeedMon_DE,
            [LANG_FR] = PalletTown_Text_OakGrassUnsafeNeedMon_FR,
            [LANG_ES] = PalletTown_Text_OakGrassUnsafeNeedMon_ES,
            [LANG_IT] = PalletTown_Text_OakGrassUnsafeNeedMon_IT
        }
    },
};

#endif  // GUARD_CONSTANTS_LOCALIZED_TEXT_H
