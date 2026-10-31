#include "global.h"
#include "mechanics.h"
#include "constants/rgb.h"

// Rare color variants: about 1 Pokémon in 340 (rarer than a shiny is
// common, independent of being shiny) has its colors shifted. Which variant
// comes from the personality value alone, so it never changes and needs no
// save data; the palette is made from the Pokémon's own normal or shiny one
// when it's loaded (GetMonSpritePalFromSpeciesAndPersonality).

#define VARIANT_ODDS_BITS 12 // out of 4096
#define VARIANT_SLOTS     4  // palettes made at once, for screens showing several

static EWRAM_DATA u16 sVariantPalettes[VARIANT_SLOTS][16] = {0};
static EWRAM_DATA u8 sNextSlot = 0;

u8 Mechanics_GetColorVariant(u32 personality)
{
    u32 hash = ((personality ^ 0xA5C3B2E1) * 2654435761u) >> (32 - VARIANT_ODDS_BITS);

    if (hash >= COLOR_VARIANT_COUNT * 4)
        return COLOR_NORMAL;
    return 1 + hash / 4;
}

const u8 *Mechanics_GetColorVariantName(u8 variant)
{
    switch (variant)
    {
    case COLOR_AURORA:
        return COMPOUND_STRING("AURORA");
    case COLOR_DUSK:
        return COMPOUND_STRING("DUSK");
    case COLOR_MIDNIGHT:
        return COMPOUND_STRING("MIDNIGHT");
    }
    return COMPOUND_STRING("");
}

static u16 ShiftColor(u16 color, u8 variant)
{
    s32 r = GET_R(color), g = GET_G(color), b = GET_B(color);

    switch (variant)
    {
    case COLOR_AURORA: // hue turned a third of the way round
        return RGB(b, r, g);
    case COLOR_DUSK: // warm sepia gold
        return RGB(min(31, (r * 100 + g * 196 + b * 48) / 256),
                   min(31, (r * 89 + g * 175 + b * 43) / 256),
                   min(31, (r * 69 + g * 136 + b * 33) / 256));
    case COLOR_MIDNIGHT: // hue turned the other way, and darker
        return RGB(g * 3 / 4, b * 3 / 4, r * 3 / 4 + 4 > 31 ? 31 : r * 3 / 4 + 4);
    }
    return color;
}

// Returns `palette` (a 16-color Pokémon sprite palette), or a variant
// copy of it if this personality has a color variant.
const u16 *Mechanics_ApplyColorVariant(const u16 *palette, u32 personality)
{
    u8 variant = Mechanics_GetColorVariant(personality);
    u16 *dest;
    u32 i;

    if (variant == COLOR_NORMAL || palette == NULL)
        return palette;
    dest = sVariantPalettes[sNextSlot];
    sNextSlot = (sNextSlot + 1) % VARIANT_SLOTS;
    dest[0] = palette[0]; // the transparent color
    for (i = 1; i < 16; i++)
        dest[i] = ShiftColor(palette[i], variant);
    return dest;
}

