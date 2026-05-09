#include "global.h"
#include "bg.h"
#include "decompress.h"
#include "palette.h"
#include "gpu_regs.h"
#include "task.h"
#include "malloc.h"
#include "new_menu_helpers.h"
#include "intro.h"
#include "language_selector.h"
#include "main.h"
#include "scanline_effect.h"

static const u16 gLanguageSelectorPal[] = INCBIN_U16("graphics/language_selector/languages_low.gbapal");
static const u32 gLanguageSelectorTiles[] = INCBIN_U32("graphics/language_selector/languages_low.4bpp.lz");
static const u32 gLanguageSelectorMap[] = INCBIN_U32("graphics/language_selector/languages_low.bin.lz");

#define RGB_BG_LANGUAGE_SELECTOR RGB(27, 25, 28)

#define BG_LANGUAGE 1

#define LANG_COUNT 5
#define LANG_BTN_HEIGHT 4
#define LANG_BTN_WIDTH 30
#define LANG_TILE_PAD (LANG_BTN_WIDTH * LANG_BTN_HEIGHT)

enum {
    LANG_EN,
    LANG_DE,
    LANG_FR,
    LANG_ES,
    LANG_IT
};

static void Task_LanguageSelector(u8 taskId);
static void VBlankCB_LanguageSelector(void);
static void InitBgs(void);
static void LoadBg(u8 selectedIndex);
static void AddLanguageButton(u8 bgId, u8 langIndex, bool8 isSelected, u8 y);

static bool8 sTaskCreated = FALSE;

static const struct BgTemplate sBgTemplate =
{
    .bg = BG_LANGUAGE,
    .charBaseIndex = 1,
    .mapBaseIndex = 31,
    .screenSize = 0,
    .paletteMode = 0,
    .priority = 0,
    .baseTile = 0
};

void CB2_LanguageSelector(void)
{
    RunTasks();
    AnimateSprites();
    BuildOamBuffer();
    UpdatePaletteFade();

    if (!sTaskCreated)
    {
        CreateTask(Task_LanguageSelector, 0);
        sTaskCreated = TRUE;
    }
}

#define tState  gTasks[taskId].data[0]
#define tCursor gTasks[taskId].data[1]

static void Task_LanguageSelector(u8 taskId)
{
    switch (tState)
    {
    case 0:
        SetVBlankCallback(VBlankCB_LanguageSelector);
        InitBgs();
        tCursor = LANG_EN;

        CpuFastFill16(RGB_BLACK, gPlttBufferFaded, PLTT_SIZE);
        gPlttBufferUnfaded[0] = RGB_BG_LANGUAGE_SELECTOR;

        LoadBg(tCursor);
        ShowBg(BG_LANGUAGE);

        BeginNormalPaletteFade(PALETTES_ALL, 0, 16, 0, RGB_BLACK);
        tState++;
        break;
    case 1:
        if (!gPaletteFade.active)
            tState++;
        break;
    case 2:
        if (JOY_NEW(DPAD_UP))
        {
            tCursor = (tCursor == 0) ? LANG_COUNT - 1 : tCursor - 1;
            LoadBg(tCursor);
        }
        else if (JOY_NEW(DPAD_DOWN))
        {
            tCursor = (tCursor == LANG_COUNT - 1) ? 0 : tCursor + 1;
            LoadBg(tCursor);
        }
        else if (JOY_NEW(A_BUTTON | START_BUTTON))
        {
            BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
            tState++;
        }
        break;
    case 3:
        if (!gPaletteFade.active)
        {
            Free(GetBgTilemapBuffer(BG_LANGUAGE));
            sTaskCreated = FALSE;
            DestroyTask(taskId);
            SetMainCallback2(CB2_WaitFadeBeforeSetUpIntro);
        }
        break;
    }
}

static void VBlankCB_LanguageSelector(void)
{
    LoadOam();
    ProcessSpriteCopyRequests();
    TransferPlttBuffer();
}

static void InitBgs(void)
{
    ResetBgsAndClearDma3BusyFlags(0);
    InitBgsFromTemplates(0, &sBgTemplate, 1);

    SetBgTilemapBuffer(BG_LANGUAGE, AllocZeroed(BG_SCREEN_SIZE));

    SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_OBJ_1D_MAP | DISPCNT_OBJ_ON);

    FillBgTilemapBufferRect_Palette0(BG_LANGUAGE, 0, 0, 0, 32, 32);
    HideBg(0);
    HideBg(BG_LANGUAGE);
    HideBg(2);
    HideBg(3);
}

static void LoadBg(u8 selectedIndex)
{
    FillBgTilemapBufferRect_Palette0(BG_LANGUAGE, 0, 0, 0, 32, 32);

    LoadPalette(gLanguageSelectorPal, BG_PLTT_ID(1), 13 * PLTT_SIZE_4BPP);
    DecompressAndCopyTileDataToVram(BG_LANGUAGE, gLanguageSelectorTiles, 0, 1, 0);

    for (u8 i = 0; i < LANG_COUNT; i++)
    {
        u8 y = i * LANG_BTN_HEIGHT;
        AddLanguageButton(BG_LANGUAGE, i, selectedIndex == i, y);
    }

    CopyBgTilemapBufferToVram(BG_LANGUAGE);
}

static void AddLanguageButton(u8 bgId, u8 langIndex, bool8 isSelected, u8 y)
{
    void *tempBuffer;
    u16 tileOffset;
    langIndex = isSelected ? LANG_COUNT + langIndex : langIndex;
    tileOffset = (langIndex * LANG_TILE_PAD) + 1;

    tempBuffer = AllocZeroed(LANG_TILE_PAD * sizeof(u16));
    if (tempBuffer == NULL)
        return;

    LZDecompressWram(gLanguageSelectorMap, tempBuffer);

    for (u16 i = 0; i < LANG_TILE_PAD; i++)
    {
        u16 tileEntry = ((u16 *)tempBuffer)[i];
        tileEntry = (tileEntry & 0x0C00) | ((tileEntry & 0x03FF) + tileOffset);
        tileEntry = (tileEntry & 0x0FFF) | (1 << 12);
        ((u16 *)tempBuffer)[i] = tileEntry;
    }

    CopyToBgTilemapBufferRect(bgId, tempBuffer, 0, y, LANG_BTN_WIDTH, LANG_BTN_HEIGHT);
    Free(tempBuffer);
}