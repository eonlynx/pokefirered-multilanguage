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

static const u16 gEnglishLanguagePal[] = INCBIN_U16("graphics/language_selector/english.gbapal");
static const u32 gEnglishLanguageTiles[] = INCBIN_U32("graphics/language_selector/english.4bpp.lz");

static const u16 gSpanishLanguagePal[] = INCBIN_U16("graphics/language_selector/spanish.gbapal");
static const u32 gSpanishLanguageTiles[] = INCBIN_U32("graphics/language_selector/spanish.4bpp.lz");

static const u16 gPortugueseLanguagePal[] = INCBIN_U16("graphics/language_selector/portuguese.gbapal");
static const u32 gPortugueseLanguageTiles[] = INCBIN_U32("graphics/language_selector/portuguese.4bpp.lz");

static const u16 gEmptyLanguagePal[] = INCBIN_U16("graphics/language_selector/empty.gbapal");
static const u32 gEmptyLanguageTiles[] = INCBIN_U32("graphics/language_selector/empty.4bpp.lz");

/*static const u16 gGermanLanguagePal[] = INCBIN_U16("graphics/language_selector/german.gbapal");
static const u32 gGermanLanguageTiles[] = INCBIN_U32("graphics/language_selector/german.4bpp.lz");

static const u16 gFrenchLanguagePal[] = INCBIN_U16("graphics/language_selector/french.gbapal");
static const u32 gFrenchLanguageTiles[] = INCBIN_U32("graphics/language_selector/french.4bpp.lz");

static const u16 gItalianLanguagePal[] = INCBIN_U16("graphics/language_selector/italian.gbapal");
static const u32 gItalianLanguageTiles[] = INCBIN_U32("graphics/language_selector/italian.4bpp.lz");*/

static const u16 gEnglishSelLanguagePal[] = INCBIN_U16("graphics/language_selector/english_selected.gbapal");
static const u32 gEnglishSelLanguageTiles[] = INCBIN_U32("graphics/language_selector/english_selected.4bpp.lz");

static const u16 gSpanishSelLanguagePal[] = INCBIN_U16("graphics/language_selector/spanish_selected.gbapal");
static const u32 gSpanishSelLanguageTiles[] = INCBIN_U32("graphics/language_selector/spanish_selected.4bpp.lz");

static const u16 gPortugueseSelLanguagePal[] = INCBIN_U16("graphics/language_selector/portuguese_selected.gbapal");
static const u32 gPortugueseSelLanguageTiles[] = INCBIN_U32("graphics/language_selector/portuguese_selected.4bpp.lz");

/*static const u16 gGermanSelLanguagePal[] = INCBIN_U16("graphics/language_selector/german_selected.gbapal");
static const u32 gGermanSelLanguageTiles[] = INCBIN_U32("graphics/language_selector/german_selected.4bpp.lz");

static const u16 gFrenchSelLanguagePal[] = INCBIN_U16("graphics/language_selector/french_selected.gbapal");
static const u32 gFrenchSelLanguageTiles[] = INCBIN_U32("graphics/language_selector/french_selected.4bpp.lz");

static const u16 gItalianSelLanguagePal[] = INCBIN_U16("graphics/language_selector/italian_selected.gbapal");
static const u32 gItalianSelLanguageTiles[] = INCBIN_U32("graphics/language_selector/italian_selected.4bpp.lz");*/

static const u32 gLanguageMap[] = INCBIN_U32("graphics/language_selector/language.bin.lz");
static const u32 gSelLanguageMap[] = INCBIN_U32("graphics/language_selector/language_selected.bin.lz");

#define RGB_BG_LANGUAGE_SELECTOR RGB(27, 25, 28)

#define BG_LANGUAGE_NORMAL     1
#define BG_LANGUAGE_SELECTED   2
#define BG_LANGUAGE_SELECTED_2 3
#define LANG_TILE_PAD_NORMAL   72
#define LANG_TILE_PAD_SELECTED 120

#define LANG_COUNT 3
#define LANG_BTN_HEIGHT 4
#define LANG_BTN_NORMAL_WIDTH 18
#define LANG_BTN_SELECTED_WIDTH 30

#define LANG_SELECTED_PLTT_OFFSET 6

enum {
    LANG_EN,
    LANG_ES,
    LANG_PT,
    LANG_EMPTY_1,
    LANG_EMPTY_2
    /*LANG_DE,
    LANG_FR,
    LANG_IT*/
};

static void Task_LanguageSelector(u8 taskId);
static void VBlankCB_LanguageSelector(void);
static void InitBgs(void);
static void LoadBg(u8 selectedIndex);
static void AddLanguageButton(u8 bgId, const u16 *pal, const u32 *tiles, const u32 *map, u8 y, u8 xPos, u8 width, u16 tileOffset, u8 plttBank);

static bool8 sTaskCreated = FALSE;

static const struct BgTemplate sBgTemplates[] =
{
    {
        .bg = BG_LANGUAGE_NORMAL,
        .charBaseIndex = 1,
        .mapBaseIndex = 31,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 2,
        .baseTile = 0
    },
    {
        .bg = BG_LANGUAGE_SELECTED,
        .charBaseIndex = 2,
        .mapBaseIndex = 30,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 1,
        .baseTile = 0
    },
    {
        .bg = BG_LANGUAGE_SELECTED_2,
        .charBaseIndex = 3,
        .mapBaseIndex = 29,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 0,
        .baseTile = 0
    },
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
        ShowBg(BG_LANGUAGE_NORMAL);
        ShowBg(BG_LANGUAGE_SELECTED);
        ShowBg(BG_LANGUAGE_SELECTED_2);

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
            if (tCursor > 0)
                tCursor--;
            else
                tCursor = LANG_COUNT - 1;

            LoadBg(tCursor);
        }
        else if (JOY_NEW(DPAD_DOWN))
        {
            if (tCursor < LANG_COUNT - 1)
                tCursor++;
            else
                tCursor = 0;

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
            Free(GetBgTilemapBuffer(BG_LANGUAGE_NORMAL));
            Free(GetBgTilemapBuffer(BG_LANGUAGE_SELECTED));
            Free(GetBgTilemapBuffer(BG_LANGUAGE_SELECTED_2));
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
    InitBgsFromTemplates(0, sBgTemplates, ARRAY_COUNT(sBgTemplates));

    SetBgTilemapBuffer(BG_LANGUAGE_NORMAL, AllocZeroed(BG_SCREEN_SIZE));
    SetBgTilemapBuffer(BG_LANGUAGE_SELECTED, AllocZeroed(BG_SCREEN_SIZE));
    SetBgTilemapBuffer(BG_LANGUAGE_SELECTED_2, AllocZeroed(BG_SCREEN_SIZE));

    SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_OBJ_1D_MAP | DISPCNT_OBJ_ON);

    FillBgTilemapBufferRect_Palette0(BG_LANGUAGE_NORMAL, 0, 0, 0, 32, 32);
    FillBgTilemapBufferRect_Palette0(BG_LANGUAGE_SELECTED, 0, 0, 0, 32, 32);
    FillBgTilemapBufferRect_Palette0(BG_LANGUAGE_SELECTED_2, 0, 0, 0, 32, 32);

    HideBg(0);
    HideBg(BG_LANGUAGE_NORMAL);
    HideBg(BG_LANGUAGE_SELECTED);
    HideBg(BG_LANGUAGE_SELECTED_2);
}

static void LoadBg(u8 selectedIndex)
{
    FillBgTilemapBufferRect_Palette0(BG_LANGUAGE_NORMAL, 0, 0, 0, 32, 32);
    FillBgTilemapBufferRect_Palette0(BG_LANGUAGE_SELECTED, 0, 0, 0, 32, 32);
    FillBgTilemapBufferRect_Palette0(BG_LANGUAGE_SELECTED_2, 0, 0, 0, 32, 32);

    AddLanguageButton(BG_LANGUAGE_NORMAL,
                      gEnglishLanguagePal,
                      gEnglishLanguageTiles,
                      gLanguageMap,
                      0, 6, LANG_BTN_NORMAL_WIDTH,
                      (LANG_EN * LANG_TILE_PAD_NORMAL) + 1,
                      1);

    AddLanguageButton(BG_LANGUAGE_NORMAL,
                      gSpanishLanguagePal,
                      gSpanishLanguageTiles,
                      gLanguageMap,
                      4, 6, LANG_BTN_NORMAL_WIDTH,
                      (LANG_ES * LANG_TILE_PAD_NORMAL) + 1,
                      2);

    AddLanguageButton(BG_LANGUAGE_NORMAL,
                      gPortugueseLanguagePal,
                      gPortugueseLanguageTiles,
                      gLanguageMap,
                      8, 6, LANG_BTN_NORMAL_WIDTH,
                      (LANG_PT * LANG_TILE_PAD_NORMAL) + 1,
                      3);

    AddLanguageButton(BG_LANGUAGE_NORMAL,
                      gEmptyLanguagePal,
                      gEmptyLanguageTiles,
                      gLanguageMap,
                      12, 6, LANG_BTN_NORMAL_WIDTH,
                      (LANG_EMPTY_1 * LANG_TILE_PAD_NORMAL) + 1,
                      4);

    AddLanguageButton(BG_LANGUAGE_NORMAL,
                      gEmptyLanguagePal,
                      gEmptyLanguageTiles,
                      gLanguageMap,
                      16, 6, LANG_BTN_NORMAL_WIDTH,
                      (LANG_EMPTY_2 * LANG_TILE_PAD_NORMAL) + 1,
                      5);

    /*AddLanguageButton(BG_LANGUAGE_NORMAL,
                      gGermanLanguagePal,
                      gGermanLanguageTiles,
                      gLanguageMap,
                      4, 6, LANG_BTN_NORMAL_WIDTH,
                      (LANG_DE * LANG_TILE_PAD_NORMAL) + 1,
                      2);

    AddLanguageButton(BG_LANGUAGE_NORMAL,
                      gFrenchLanguagePal,
                      gFrenchLanguageTiles,
                      gLanguageMap,
                      8, 6, LANG_BTN_NORMAL_WIDTH,
                      (LANG_FR * LANG_TILE_PAD_NORMAL) + 1,
                      3);

    AddLanguageButton(BG_LANGUAGE_NORMAL,
                      gItalianLanguagePal,
                      gItalianLanguageTiles,
                      gLanguageMap,
                      16, 6, LANG_BTN_NORMAL_WIDTH,
                      (LANG_IT * LANG_TILE_PAD_NORMAL) + 1,
                      5);*/

    if (selectedIndex == LANG_EN)
    {
        AddLanguageButton(BG_LANGUAGE_SELECTED,
                          gEnglishSelLanguagePal,
                          gEnglishSelLanguageTiles,
                          gSelLanguageMap,
                          0, 0, LANG_BTN_SELECTED_WIDTH,
                          (LANG_EN * LANG_TILE_PAD_SELECTED) + 1,
                          LANG_EN + LANG_SELECTED_PLTT_OFFSET);
    }
    else if (selectedIndex == LANG_ES)
    {
        AddLanguageButton(BG_LANGUAGE_SELECTED,
                          gSpanishSelLanguagePal,
                          gSpanishSelLanguageTiles,
                          gSelLanguageMap,
                          4, 0, LANG_BTN_SELECTED_WIDTH,
                          (LANG_ES * LANG_TILE_PAD_SELECTED) + 1,
                          LANG_ES + LANG_SELECTED_PLTT_OFFSET);
    }
    else if (selectedIndex == LANG_PT)
    {
        AddLanguageButton(BG_LANGUAGE_SELECTED,
                          gPortugueseSelLanguagePal,
                          gPortugueseSelLanguageTiles,
                          gSelLanguageMap,
                          8, 0, LANG_BTN_SELECTED_WIDTH,
                          (LANG_PT * LANG_TILE_PAD_SELECTED) + 1,
                          LANG_PT + LANG_SELECTED_PLTT_OFFSET);
    }
    /*else if (selectedIndex == LANG_DE)
    {
        AddLanguageButton(BG_LANGUAGE_SELECTED,
                          gGermanSelLanguagePal,
                          gGermanSelLanguageTiles,
                          gSelLanguageMap,
                          4, 0, LANG_BTN_SELECTED_WIDTH,
                          (LANG_DE * LANG_TILE_PAD_SELECTED) + 1,
                          LANG_DE + LANG_SELECTED_PLTT_OFFSET);
    }
    else if (selectedIndex == LANG_FR)
    {
        AddLanguageButton(BG_LANGUAGE_SELECTED,
                          gFrenchSelLanguagePal,
                          gFrenchSelLanguageTiles,
                          gSelLanguageMap,
                          8, 0, LANG_BTN_SELECTED_WIDTH,
                          (LANG_FR * LANG_TILE_PAD_SELECTED) + 1,
                          LANG_FR + LANG_SELECTED_PLTT_OFFSET);
    }
    else
    {
        AddLanguageButton(BG_LANGUAGE_SELECTED_2,
                          gItalianSelLanguagePal,
                          gItalianSelLanguageTiles,
                          gSelLanguageMap,
                          16, 0, LANG_BTN_SELECTED_WIDTH,
                          ((LANG_IT - 3) * LANG_TILE_PAD_SELECTED) + 1,
                          LANG_IT + LANG_SELECTED_PLTT_OFFSET);
    }*/

    CopyBgTilemapBufferToVram(BG_LANGUAGE_NORMAL);
    CopyBgTilemapBufferToVram(BG_LANGUAGE_SELECTED);
    CopyBgTilemapBufferToVram(BG_LANGUAGE_SELECTED_2);
}

static void AddLanguageButton(u8 bgId, const u16 *pal, const u32 *tiles, const u32 *map, u8 y, u8 xPos, u8 width, u16 tileOffset, u8 plttBank)
{
    void *tempBuffer;
    u16 i;
    u16 numEntries = width * LANG_BTN_HEIGHT;

    LoadPalette(pal, BG_PLTT_ID(plttBank), PLTT_SIZE_4BPP);
    DecompressAndCopyTileDataToVram(bgId, tiles, 0, tileOffset, 0);

    tempBuffer = AllocZeroed(numEntries * sizeof(u16));
    if (tempBuffer != NULL)
    {
        LZDecompressWram(map, tempBuffer);

        for (i = 0; i < numEntries; i++)
        {
            u16 tileEntry = ((u16 *)tempBuffer)[i];

            tileEntry = (tileEntry & 0x0C00) | ((tileEntry & 0x03FF) + tileOffset);
            tileEntry = (tileEntry & 0x0FFF) | (plttBank << 12);

            ((u16 *)tempBuffer)[i] = tileEntry;
        }

        CopyToBgTilemapBufferRect(bgId, tempBuffer, xPos, y, width, LANG_BTN_HEIGHT);
        Free(tempBuffer);
    }
}