// Window glass that lights up at night (src/world_daynight.c). Each entry is
// a tileset and the colours in it (palette, index) that only appear in
// windows, picked with tools/daynight/render_map.py. In the evening and at
// night these colours blend to warm lamplight instead of darkening.
// The General tileset's house windows cover most towns.

#define LIGHT(pal, idx) ((pal) << 4 | (idx))

static const struct DayNightLights sDayNightLights[] =
{
    { &gTileset_General,   { LIGHT(5, 3) } },             // houses (Oldale, Petalburg, Slateport...)
    { &gTileset_Petalburg, { LIGHT(6, 9), LIGHT(8, 9) } }, // Littleroot's houses
    { &gTileset_Rustboro,  { LIGHT(11, 10), LIGHT(10, 10) } }, // stone and slate buildings
    { &gTileset_Mauville,  { LIGHT(7, 15), LIGHT(10, 3) } }, // houses, the shopping mall
    { &gTileset_Fallarbor, { LIGHT(7, 10) } },
    { &gTileset_Lilycove,  { LIGHT(8, 10) } },
};
