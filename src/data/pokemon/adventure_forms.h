// Regional forms of Starfall Isle and Amberwood Hollow (docs/adventure.md):
// graphics, following-Pokémon frames and level-up moves. Their species info
// is in species_info/adventure_forms.h; the sprites are drawn by
// tools/adventure/gen_regional_forms.py.

const u32 gMonFrontPic_WingullStarfall[] = INCGFX_U32("graphics/pokemon/wingull/starfall/anim_front.png", ".4bpp.smol");
const u16 gMonPalette_WingullStarfall[] = INCGFX_U16("graphics/pokemon/wingull/starfall/normal.pal", ".gbapal");
const u32 gMonBackPic_WingullStarfall[] = INCGFX_U32("graphics/pokemon/wingull/starfall/back.png", ".4bpp.smol");
const u16 gMonShinyPalette_WingullStarfall[] = INCGFX_U16("graphics/pokemon/wingull/starfall/shiny.pal", ".gbapal");
const u8 gMonIcon_WingullStarfall[] = INCGFX_U8("graphics/pokemon/wingull/starfall/icon.png", ".4bpp");
#if OW_POKEMON_OBJECT_EVENTS
const u32 gObjectEventPic_WingullStarfall[] = INCGFX_COMP("graphics/pokemon/wingull/starfall/overworld.png", ".4bpp", "-mwidth 4 -mheight 4");
#if OW_PKMN_OBJECTS_SHARE_PALETTES == FALSE
const u16 gOverworldPalette_WingullStarfall[] = INCGFX_U16("graphics/pokemon/wingull/starfall/overworld_normal.pal", ".gbapal");
const u16 gShinyOverworldPalette_WingullStarfall[] = INCGFX_U16("graphics/pokemon/wingull/starfall/overworld_shiny.pal", ".gbapal");
#endif //OW_PKMN_OBJECTS_SHARE_PALETTES
static const struct SpriteFrameImage sPicTable_WingullStarfall[] = {
    overworld_ascending_frames(gObjectEventPic_WingullStarfall, 4, 4),
};
#endif //OW_POKEMON_OBJECT_EVENTS

const u32 gMonFrontPic_PelipperStarfall[] = INCGFX_U32("graphics/pokemon/pelipper/starfall/anim_front.png", ".4bpp.smol");
const u16 gMonPalette_PelipperStarfall[] = INCGFX_U16("graphics/pokemon/pelipper/starfall/normal.pal", ".gbapal");
const u32 gMonBackPic_PelipperStarfall[] = INCGFX_U32("graphics/pokemon/pelipper/starfall/back.png", ".4bpp.smol");
const u16 gMonShinyPalette_PelipperStarfall[] = INCGFX_U16("graphics/pokemon/pelipper/starfall/shiny.pal", ".gbapal");
const u8 gMonIcon_PelipperStarfall[] = INCGFX_U8("graphics/pokemon/pelipper/starfall/icon.png", ".4bpp");
#if OW_POKEMON_OBJECT_EVENTS
const u32 gObjectEventPic_PelipperStarfall[] = INCGFX_COMP("graphics/pokemon/pelipper/starfall/overworld.png", ".4bpp", "-mwidth 4 -mheight 4");
#if OW_PKMN_OBJECTS_SHARE_PALETTES == FALSE
const u16 gOverworldPalette_PelipperStarfall[] = INCGFX_U16("graphics/pokemon/pelipper/starfall/overworld_normal.pal", ".gbapal");
const u16 gShinyOverworldPalette_PelipperStarfall[] = INCGFX_U16("graphics/pokemon/pelipper/starfall/overworld_shiny.pal", ".gbapal");
#endif //OW_PKMN_OBJECTS_SHARE_PALETTES
static const struct SpriteFrameImage sPicTable_PelipperStarfall[] = {
    overworld_ascending_frames(gObjectEventPic_PelipperStarfall, 4, 4),
};
#endif //OW_POKEMON_OBJECT_EVENTS

const u32 gMonFrontPic_LuvdiscStarfall[] = INCGFX_U32("graphics/pokemon/luvdisc/starfall/anim_front.png", ".4bpp.smol");
const u16 gMonPalette_LuvdiscStarfall[] = INCGFX_U16("graphics/pokemon/luvdisc/starfall/normal.pal", ".gbapal");
const u32 gMonBackPic_LuvdiscStarfall[] = INCGFX_U32("graphics/pokemon/luvdisc/starfall/back.png", ".4bpp.smol");
const u16 gMonShinyPalette_LuvdiscStarfall[] = INCGFX_U16("graphics/pokemon/luvdisc/starfall/shiny.pal", ".gbapal");
const u8 gMonIcon_LuvdiscStarfall[] = INCGFX_U8("graphics/pokemon/luvdisc/starfall/icon.png", ".4bpp");
#if OW_POKEMON_OBJECT_EVENTS
const u32 gObjectEventPic_LuvdiscStarfall[] = INCGFX_COMP("graphics/pokemon/luvdisc/starfall/overworld.png", ".4bpp", "-mwidth 4 -mheight 4");
#if OW_PKMN_OBJECTS_SHARE_PALETTES == FALSE
const u16 gOverworldPalette_LuvdiscStarfall[] = INCGFX_U16("graphics/pokemon/luvdisc/starfall/overworld_normal.pal", ".gbapal");
const u16 gShinyOverworldPalette_LuvdiscStarfall[] = INCGFX_U16("graphics/pokemon/luvdisc/starfall/overworld_shiny.pal", ".gbapal");
#endif //OW_PKMN_OBJECTS_SHARE_PALETTES
static const struct SpriteFrameImage sPicTable_LuvdiscStarfall[] = {
    overworld_ascending_frames(gObjectEventPic_LuvdiscStarfall, 4, 4),
};
#endif //OW_POKEMON_OBJECT_EVENTS

const u32 gMonFrontPic_TeddiursaAmberwood[] = INCGFX_U32("graphics/pokemon/teddiursa/amberwood/anim_front.png", ".4bpp.smol");
const u16 gMonPalette_TeddiursaAmberwood[] = INCGFX_U16("graphics/pokemon/teddiursa/amberwood/normal.pal", ".gbapal");
const u32 gMonBackPic_TeddiursaAmberwood[] = INCGFX_U32("graphics/pokemon/teddiursa/amberwood/back.png", ".4bpp.smol");
const u16 gMonShinyPalette_TeddiursaAmberwood[] = INCGFX_U16("graphics/pokemon/teddiursa/amberwood/shiny.pal", ".gbapal");
const u8 gMonIcon_TeddiursaAmberwood[] = INCGFX_U8("graphics/pokemon/teddiursa/amberwood/icon.png", ".4bpp");
#if OW_POKEMON_OBJECT_EVENTS
const u32 gObjectEventPic_TeddiursaAmberwood[] = INCGFX_COMP("graphics/pokemon/teddiursa/amberwood/overworld.png", ".4bpp", "-mwidth 4 -mheight 4");
#if OW_PKMN_OBJECTS_SHARE_PALETTES == FALSE
const u16 gOverworldPalette_TeddiursaAmberwood[] = INCGFX_U16("graphics/pokemon/teddiursa/amberwood/overworld_normal.pal", ".gbapal");
const u16 gShinyOverworldPalette_TeddiursaAmberwood[] = INCGFX_U16("graphics/pokemon/teddiursa/amberwood/overworld_shiny.pal", ".gbapal");
#endif //OW_PKMN_OBJECTS_SHARE_PALETTES
static const struct SpriteFrameImage sPicTable_TeddiursaAmberwood[] = {
    overworld_ascending_frames(gObjectEventPic_TeddiursaAmberwood, 4, 4),
};
#endif //OW_POKEMON_OBJECT_EVENTS

const u32 gMonFrontPic_UrsaringAmberwood[] = INCGFX_U32("graphics/pokemon/ursaring/amberwood/anim_front.png", ".4bpp.smol");
const u16 gMonPalette_UrsaringAmberwood[] = INCGFX_U16("graphics/pokemon/ursaring/amberwood/normal.pal", ".gbapal");
const u32 gMonBackPic_UrsaringAmberwood[] = INCGFX_U32("graphics/pokemon/ursaring/amberwood/back.png", ".4bpp.smol");
const u16 gMonShinyPalette_UrsaringAmberwood[] = INCGFX_U16("graphics/pokemon/ursaring/amberwood/shiny.pal", ".gbapal");
const u8 gMonIcon_UrsaringAmberwood[] = INCGFX_U8("graphics/pokemon/ursaring/amberwood/icon.png", ".4bpp");
#if OW_POKEMON_OBJECT_EVENTS
const u32 gObjectEventPic_UrsaringAmberwood[] = INCGFX_COMP("graphics/pokemon/ursaring/amberwood/overworld.png", ".4bpp", "-mwidth 4 -mheight 4");
#if OW_PKMN_OBJECTS_SHARE_PALETTES == FALSE
const u16 gOverworldPalette_UrsaringAmberwood[] = INCGFX_U16("graphics/pokemon/ursaring/amberwood/overworld_normal.pal", ".gbapal");
const u16 gShinyOverworldPalette_UrsaringAmberwood[] = INCGFX_U16("graphics/pokemon/ursaring/amberwood/overworld_shiny.pal", ".gbapal");
#endif //OW_PKMN_OBJECTS_SHARE_PALETTES
static const struct SpriteFrameImage sPicTable_UrsaringAmberwood[] = {
    overworld_ascending_frames(gObjectEventPic_UrsaringAmberwood, 4, 4),
};
#endif //OW_POKEMON_OBJECT_EVENTS

static const struct LevelUpMove sWingullStarfallLevelUpLearnset[] = {
    LEVEL_UP_MOVE( 1, MOVE_GROWL),
    LEVEL_UP_MOVE( 1, MOVE_CONFUSION),
    LEVEL_UP_MOVE( 5, MOVE_QUICK_ATTACK),
    LEVEL_UP_MOVE(10, MOVE_SUPERSONIC),
    LEVEL_UP_MOVE(15, MOVE_WING_ATTACK),
    LEVEL_UP_MOVE(20, MOVE_PSYBEAM),
    LEVEL_UP_MOVE(26, MOVE_AGILITY),
    LEVEL_UP_MOVE(30, MOVE_AIR_SLASH),
    LEVEL_UP_MOVE(35, MOVE_COSMIC_POWER),
    LEVEL_UP_MOVE(40, MOVE_ROOST),
    LEVEL_UP_MOVE(45, MOVE_HURRICANE),
    LEVEL_UP_END
};

static const struct LevelUpMove sPelipperStarfallLevelUpLearnset[] = {
    LEVEL_UP_MOVE( 0, MOVE_PSYCHIC),
    LEVEL_UP_MOVE( 1, MOVE_GROWL),
    LEVEL_UP_MOVE( 1, MOVE_CONFUSION),
    LEVEL_UP_MOVE( 1, MOVE_SUPERSONIC),
    LEVEL_UP_MOVE( 1, MOVE_TAILWIND),
    LEVEL_UP_MOVE(15, MOVE_WING_ATTACK),
    LEVEL_UP_MOVE(20, MOVE_PSYBEAM),
    LEVEL_UP_MOVE(28, MOVE_AGILITY),
    LEVEL_UP_MOVE(33, MOVE_AIR_SLASH),
    LEVEL_UP_MOVE(39, MOVE_COSMIC_POWER),
    LEVEL_UP_MOVE(44, MOVE_ROOST),
    LEVEL_UP_MOVE(50, MOVE_HURRICANE),
    LEVEL_UP_MOVE(55, MOVE_FUTURE_SIGHT),
    LEVEL_UP_END
};

static const struct LevelUpMove sLuvdiscStarfallLevelUpLearnset[] = {
    LEVEL_UP_MOVE( 1, MOVE_TACKLE),
    LEVEL_UP_MOVE( 1, MOVE_CHARM),
    LEVEL_UP_MOVE( 4, MOVE_WATER_GUN),
    LEVEL_UP_MOVE( 7, MOVE_FAIRY_WIND),
    LEVEL_UP_MOVE(10, MOVE_SWEET_KISS),
    LEVEL_UP_MOVE(14, MOVE_WATER_PULSE),
    LEVEL_UP_MOVE(17, MOVE_DRAINING_KISS),
    LEVEL_UP_MOVE(22, MOVE_AQUA_RING),
    LEVEL_UP_MOVE(26, MOVE_ATTRACT),
    LEVEL_UP_MOVE(31, MOVE_DAZZLING_GLEAM),
    LEVEL_UP_MOVE(35, MOVE_MOONLIGHT),
    LEVEL_UP_MOVE(40, MOVE_SOAK),
    LEVEL_UP_MOVE(46, MOVE_MOONBLAST),
    LEVEL_UP_END
};

static const struct LevelUpMove sTeddiursaAmberwoodLevelUpLearnset[] = {
    LEVEL_UP_MOVE( 1, MOVE_SCRATCH),
    LEVEL_UP_MOVE( 1, MOVE_LEER),
    LEVEL_UP_MOVE( 1, MOVE_LICK),
    LEVEL_UP_MOVE( 1, MOVE_FAKE_TEARS),
    LEVEL_UP_MOVE( 4, MOVE_LEAFAGE),
    LEVEL_UP_MOVE( 8, MOVE_FURY_SWIPES),
    LEVEL_UP_MOVE(13, MOVE_RAZOR_LEAF),
    LEVEL_UP_MOVE(17, MOVE_BABY_DOLL_EYES),
    LEVEL_UP_MOVE(22, MOVE_SWEET_SCENT),
    LEVEL_UP_MOVE(25, MOVE_SLASH),
    LEVEL_UP_MOVE(29, MOVE_SEED_BOMB),
    LEVEL_UP_MOVE(33, MOVE_REST),
    LEVEL_UP_MOVE(33, MOVE_SNORE),
    LEVEL_UP_MOVE(37, MOVE_THRASH),
    LEVEL_UP_MOVE(41, MOVE_WOOD_HAMMER),
    LEVEL_UP_END
};

static const struct LevelUpMove sUrsaringAmberwoodLevelUpLearnset[] = {
    LEVEL_UP_MOVE( 0, MOVE_BULLDOZE),
    LEVEL_UP_MOVE( 1, MOVE_SCRATCH),
    LEVEL_UP_MOVE( 1, MOVE_LEER),
    LEVEL_UP_MOVE( 1, MOVE_LICK),
    LEVEL_UP_MOVE( 1, MOVE_FAKE_TEARS),
    LEVEL_UP_MOVE( 1, MOVE_LEAFAGE),
    LEVEL_UP_MOVE( 8, MOVE_FURY_SWIPES),
    LEVEL_UP_MOVE(13, MOVE_RAZOR_LEAF),
    LEVEL_UP_MOVE(17, MOVE_BABY_DOLL_EYES),
    LEVEL_UP_MOVE(22, MOVE_SWEET_SCENT),
    LEVEL_UP_MOVE(25, MOVE_SLASH),
    LEVEL_UP_MOVE(29, MOVE_SEED_BOMB),
    LEVEL_UP_MOVE(35, MOVE_REST),
    LEVEL_UP_MOVE(35, MOVE_SNORE),
    LEVEL_UP_MOVE(41, MOVE_HORN_LEECH),
    LEVEL_UP_MOVE(48, MOVE_EARTHQUAKE),
    LEVEL_UP_MOVE(56, MOVE_WOOD_HAMMER),
    LEVEL_UP_END
};
