// Graphics and moves of the brand-new Pokémon (docs/new_species.md). The
// sprites are drawn by tools/new_species/draw.py.

const u32 gMonFrontPic_Jellume[] = INCGFX_U32("graphics/pokemon/jellume/anim_front.png", ".4bpp.smol");
const u16 gMonPalette_Jellume[] = INCGFX_U16("graphics/pokemon/jellume/normal.pal", ".gbapal");
const u32 gMonBackPic_Jellume[] = INCGFX_U32("graphics/pokemon/jellume/back.png", ".4bpp.smol");
const u16 gMonShinyPalette_Jellume[] = INCGFX_U16("graphics/pokemon/jellume/shiny.pal", ".gbapal");
const u8 gMonIcon_Jellume[] = INCGFX_U8("graphics/pokemon/jellume/icon.png", ".4bpp");

const u32 gObjectEventPic_Jellume[] = INCGFX_COMP("graphics/pokemon/jellume/overworld.png", ".4bpp", "-mwidth 4 -mheight 4");
const u16 gOverworldPalette_Jellume[] = INCGFX_U16("graphics/pokemon/jellume/overworld_normal.pal", ".gbapal");
const u16 gShinyOverworldPalette_Jellume[] = INCGFX_U16("graphics/pokemon/jellume/overworld_shiny.pal", ".gbapal");

static const struct SpriteFrameImage sPicTable_Jellume[] = {
    overworld_ascending_frames(gObjectEventPic_Jellume, 4, 4),
};

const u32 gMonFrontPic_Lumedusa[] = INCGFX_U32("graphics/pokemon/lumedusa/anim_front.png", ".4bpp.smol");
const u16 gMonPalette_Lumedusa[] = INCGFX_U16("graphics/pokemon/lumedusa/normal.pal", ".gbapal");
const u32 gMonBackPic_Lumedusa[] = INCGFX_U32("graphics/pokemon/lumedusa/back.png", ".4bpp.smol");
const u16 gMonShinyPalette_Lumedusa[] = INCGFX_U16("graphics/pokemon/lumedusa/shiny.pal", ".gbapal");
const u8 gMonIcon_Lumedusa[] = INCGFX_U8("graphics/pokemon/lumedusa/icon.png", ".4bpp");

const u32 gObjectEventPic_Lumedusa[] = INCGFX_COMP("graphics/pokemon/lumedusa/overworld.png", ".4bpp", "-mwidth 4 -mheight 4");
const u16 gOverworldPalette_Lumedusa[] = INCGFX_U16("graphics/pokemon/lumedusa/overworld_normal.pal", ".gbapal");
const u16 gShinyOverworldPalette_Lumedusa[] = INCGFX_U16("graphics/pokemon/lumedusa/overworld_shiny.pal", ".gbapal");

static const struct SpriteFrameImage sPicTable_Lumedusa[] = {
    overworld_ascending_frames(gObjectEventPic_Lumedusa, 4, 4),
};

const u32 gMonFrontPic_Aurelluma[] = INCGFX_U32("graphics/pokemon/aurelluma/anim_front.png", ".4bpp.smol");
const u16 gMonPalette_Aurelluma[] = INCGFX_U16("graphics/pokemon/aurelluma/normal.pal", ".gbapal");
const u32 gMonBackPic_Aurelluma[] = INCGFX_U32("graphics/pokemon/aurelluma/back.png", ".4bpp.smol");
const u16 gMonShinyPalette_Aurelluma[] = INCGFX_U16("graphics/pokemon/aurelluma/shiny.pal", ".gbapal");
const u8 gMonIcon_Aurelluma[] = INCGFX_U8("graphics/pokemon/aurelluma/icon.png", ".4bpp");

const u32 gObjectEventPic_Aurelluma[] = INCGFX_COMP("graphics/pokemon/aurelluma/overworld.png", ".4bpp", "-mwidth 4 -mheight 4");
const u16 gOverworldPalette_Aurelluma[] = INCGFX_U16("graphics/pokemon/aurelluma/overworld_normal.pal", ".gbapal");
const u16 gShinyOverworldPalette_Aurelluma[] = INCGFX_U16("graphics/pokemon/aurelluma/overworld_shiny.pal", ".gbapal");

static const struct SpriteFrameImage sPicTable_Aurelluma[] = {
    overworld_ascending_frames(gObjectEventPic_Aurelluma, 4, 4),
};

static const struct LevelUpMove sJellumeLevelUpLearnset[] = {
    LEVEL_UP_MOVE( 1, MOVE_THUNDER_SHOCK),
    LEVEL_UP_MOVE( 1, MOVE_WATER_GUN),
    LEVEL_UP_MOVE( 4, MOVE_SUPERSONIC),
    LEVEL_UP_MOVE( 8, MOVE_BUBBLE),
    LEVEL_UP_MOVE(12, MOVE_THUNDER_WAVE),
    LEVEL_UP_MOVE(16, MOVE_BUBBLE_BEAM),
    LEVEL_UP_MOVE(20, MOVE_SPARK),
    LEVEL_UP_MOVE(24, MOVE_CONFUSE_RAY),
    LEVEL_UP_MOVE(28, MOVE_AQUA_RING),
    LEVEL_UP_MOVE(32, MOVE_DISCHARGE),
    LEVEL_UP_END
};

static const struct LevelUpMove sLumedusaLevelUpLearnset[] = {
    LEVEL_UP_MOVE( 0, MOVE_PARABOLIC_CHARGE),
    LEVEL_UP_MOVE( 1, MOVE_THUNDER_SHOCK),
    LEVEL_UP_MOVE( 1, MOVE_WATER_GUN),
    LEVEL_UP_MOVE( 1, MOVE_SUPERSONIC),
    LEVEL_UP_MOVE( 8, MOVE_BUBBLE),
    LEVEL_UP_MOVE(12, MOVE_THUNDER_WAVE),
    LEVEL_UP_MOVE(16, MOVE_BUBBLE_BEAM),
    LEVEL_UP_MOVE(21, MOVE_SPARK),
    LEVEL_UP_MOVE(26, MOVE_CONFUSE_RAY),
    LEVEL_UP_MOVE(31, MOVE_AQUA_RING),
    LEVEL_UP_MOVE(36, MOVE_DISCHARGE),
    LEVEL_UP_MOVE(41, MOVE_SCALD),
    LEVEL_UP_MOVE(46, MOVE_CHARGE_BEAM),
    LEVEL_UP_END
};

static const struct LevelUpMove sAurellumaLevelUpLearnset[] = {
    LEVEL_UP_MOVE( 0, MOVE_RISING_VOLTAGE),
    LEVEL_UP_MOVE( 1, MOVE_PARABOLIC_CHARGE),
    LEVEL_UP_MOVE( 1, MOVE_THUNDER_SHOCK),
    LEVEL_UP_MOVE( 1, MOVE_WATER_GUN),
    LEVEL_UP_MOVE( 1, MOVE_SUPERSONIC),
    LEVEL_UP_MOVE(16, MOVE_BUBBLE_BEAM),
    LEVEL_UP_MOVE(21, MOVE_SPARK),
    LEVEL_UP_MOVE(26, MOVE_CONFUSE_RAY),
    LEVEL_UP_MOVE(31, MOVE_AQUA_RING),
    LEVEL_UP_MOVE(36, MOVE_DISCHARGE),
    LEVEL_UP_MOVE(42, MOVE_SCALD),
    LEVEL_UP_MOVE(48, MOVE_TAIL_GLOW),
    LEVEL_UP_MOVE(54, MOVE_THUNDER),
    LEVEL_UP_MOVE(60, MOVE_HYDRO_PUMP),
    LEVEL_UP_END
};
