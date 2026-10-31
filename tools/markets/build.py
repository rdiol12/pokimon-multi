"""Builds the MARKET HALL in Slateport and the AUCTION HOUSE in Lilycove:
puts the buildings (tools/markets/art.py) into the two towns, makes the two
rooms, their NPCs and scripts, and moves the auctioneer out of the Lilycove
Department Store.

    python3 tools/markets/art.py      # the art, into the tilesets
    python3 tools/markets/build.py    # the maps

Both rooms use the market and auction that already exist: the MARKET MASTER
opens the market board (Netplay_EventScript_MarketBoard, src/netplay.c), the
auctioneer opens the auction (Life_EventScript_AuctionCollect,
src/life_auction.c). Running this again gives the same result.
"""
import json, os, struct, sys
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', 'areas'))
from tiles import ROOT, layouts, load_blocks
from build import register_layout, register_group, register_scripts, add_warp, npc

SOLID, WALK = 0x0400, 0x3000          # collision 1 / elevation 0, and collision 0 / elevation 3


def art(name):
    return json.load(open(f'{HERE}/{name}_tiles.json'))['objects']


def blocks_of(layout_id):
    return load_blocks(layouts()[layout_id])


def save_blocks(path, grid):
    h, w = grid.shape
    with open(f'{ROOT}/{path}', 'wb') as f:
        f.write(struct.pack(f'<{w * h}H', *[int(v) for v in grid.flatten()]))


def put_object(grid, rows, x, y, walk=(), coll=SOLID):
    """Writes an art object's metatiles at (x, y); -1 cells are left alone."""
    for r, row in enumerate(rows):
        for c, m in enumerate(row):
            if m >= 0:
                grid[y + r, x + c] = m | (WALK if (r, c) in walk else coll)


def new_layout(lid, name, w, h, primary, secondary, grid, border_from):
    d = f'data/layouts/{name}'
    os.makedirs(f'{ROOT}/{d}', exist_ok=True)
    save_blocks(f'{d}/map.bin', grid)
    src = layouts()[border_from]['border_filepath']
    open(f'{ROOT}/{d}/border.bin', 'wb').write(open(f'{ROOT}/{src}', 'rb').read())
    register_layout({'id': lid, 'name': f'{name}_Layout', 'width': w, 'height': h, 'primary_tileset': primary,
                     'secondary_tileset': secondary, 'border_filepath': f'{d}/border.bin',
                     'blockdata_filepath': f'{d}/map.bin'})


def new_map(name, map_id, layout, music, mapsec, group, objects, warps, bgs, scripts, after):
    m = {'id': map_id, 'name': name, 'layout': layout, 'music': music, 'region': 'REGION_HOENN',
         'region_map_section': mapsec, 'requires_flash': False, 'weather': 'WEATHER_NONE',
         'map_type': 'MAP_TYPE_INDOOR', 'allow_cycling': False, 'allow_escaping': False, 'allow_running': False,
         'show_map_name': False, 'battle_scene': 'MAP_BATTLE_SCENE_NORMAL', 'connections': None,
         'object_events': objects, 'warp_events': warps, 'coord_events': [], 'bg_events': bgs}
    d = f'{ROOT}/data/maps/{name}'
    os.makedirs(d, exist_ok=True)
    with open(f'{d}/map.json', 'w') as f:
        json.dump(m, f, indent=2, ensure_ascii=False)
        f.write('\n')
    open(f'{d}/scripts.inc', 'w').write(scripts.strip('\n') + '\n')
    register_group(group, name, after=after)
    register_scripts(name)


def sign(x, y, script):
    return {'type': 'sign', 'x': x, 'y': y, 'elevation': 0, 'player_facing_dir': 'BG_EVENT_PLAYER_FACING_NORTH',
            'script': script}


def text(label, *pages):
    """Each page is a list of lines (two per box); pages are joined with \\p."""
    out = [f'{label}:']
    for i, page in enumerate(pages):
        for j, line in enumerate(page):
            end = '$' if (i == len(pages) - 1 and j == len(page) - 1) else ('\\p' if j == len(page) - 1 else
                                                                             ('\\n' if j % 2 == 0 else '\\l'))
            out.append(f'\t.string "{line}{end}"')
    return '\n'.join(out) + '\n'


def talker(label, text_label, gfx, x, y, move='MOVEMENT_TYPE_FACE_DOWN', rx=0, ry=0):
    o = npc(gfx, x, y, label, move)
    o['movement_range_x'], o['movement_range_y'] = rx, ry
    return o, f'{label}::\n\tmsgbox {text_label}, MSGBOX_NPC\n\tend\n'


# ---------------------------------------------------------------- Slateport: the MARKET HALL
SLATE_X, SLATE_Y = 12, 16              # top left of the 5 x 4 building; the door is at (14, 19)
MH = 'SlateportCity_MarketHall'


def slateport_outside():
    lay = layouts()['LAYOUT_SLATEPORT_CITY']
    grid = load_blocks(lay)
    obj = art('slateport')['market_hall']
    put_object(grid, obj, SLATE_X, SLATE_Y, walk={(3, 2)})
    save_blocks(lay['blockdata_filepath'], grid)
    return add_warp('SlateportCity', {'x': SLATE_X + 2, 'y': SLATE_Y + 3, 'elevation': 0,
                                      'dest_map': 'MAP_SLATEPORT_CITY_MARKET_HALL', 'dest_warp_id': '0'})


def market_room():
    W, H = 17, 12
    f2 = blocks_of('LAYOUT_LILYCOVE_CITY_DEPARTMENT_STORE_2F')
    f3 = blocks_of('LAYOUT_LILYCOVE_CITY_DEPARTMENT_STORE_3F')
    mart = blocks_of('LAYOUT_MART')
    shop = art('shop')
    g = f3[0:1, 0:1].repeat(H, 0).repeat(W, 1).copy()
    g[:, :] = 0x3201                     # floor
    g[:, 0] = 0x3206                     # the wall's shadow on the left
    g[0, :], g[1, :] = 0x0688, 0x0690    # back wall
    g[2, :] = 0x3208
    g[2, 0] = 0x320B
    for x in (13, 15):                   # windows
        g[0, x], g[1, x] = 0x0710, 0x0711
    # the MARKET MASTER's counter, from the department store's 3F
    g[0:4, 6:12] = f3[0:4, 6:12]
    g[4, 6], g[4, 7:12], g[4, 12] = 0x320F, 0x3208, 0x3202
    # the market board on the back wall
    put_object(g, shop['board_top'], 2, 0)
    put_object(g, shop['board_bottom'], 2, 1)
    # shelves of goods on both sides, with their shadows
    for x in (1, 13):
        g[6:10, x:x + 3] = f2[4:8, 2:5]
    # fruit stalls: crates and scales between the shelves and the aisle
    for x, y in ((5, 6), (5, 7), (11, 6), (11, 7)):
        put_object(g, shop['produce_crate'], x, y)
    put_object(g, shop['scale_stand'], 5, 8)
    put_object(g, shop['scale_stand'], 11, 8)
    # potted plants in the top corners
    for x in (1, 16):
        g[3, x] = f2[4, 17]
    g[11, 8], g[11, 9] = mart[7, 3], mart[7, 4]   # door mat
    new_layout('LAYOUT_SLATEPORT_CITY_MARKET_HALL', MH, W, H, 'gTileset_Building', 'gTileset_Shop', g,
               'LAYOUT_MART')
    return (8, 11), (9, 11)


MH_SCRIPTS = f"""
{MH}_MapScripts::
	.byte 0

@ The MARKET MASTER runs the hall: a word from him, then the market board
@ (data/scripts/netplay.inc), the same one as on every POKé MART wall.
{MH}_EventScript_MarketMaster::
	lock
	faceplayer
	goto_if_set FLAG_TEMP_1, {MH}_EventScript_MarketMasterAgain
	setflag FLAG_TEMP_1
	msgbox {MH}_Text_MarketMasterHello, MSGBOX_DEFAULT
	goto Netplay_EventScript_MarketBoard
	end

{MH}_EventScript_MarketMasterAgain::
	msgbox {MH}_Text_MarketMasterAgain, MSGBOX_DEFAULT
	goto Netplay_EventScript_MarketBoard
	end
"""

MH_TEXTS = [
    text(f'{MH}_Text_MarketMasterHello',
         ["Welcome to SLATEPORT's MARKET HALL!", "I'm BARNABY, the MARKET MASTER."],
         ["Trainers from every town post", "POKéMON here for sale."],
         ["Pick one off the board and it's", "yours, or post one of your own."],
         ["Let's see what's on the board", "today!"]),
    text(f'{MH}_Text_MarketMasterAgain',
         ["BARNABY: Fresh listings came in!", "Let's have a look."]),
    text(f'{MH}_Text_Shopper',
         ["I came for the fruit stalls, but", "the real bargains are on the board."],
         ["Somebody sold a shiny ZIGZAGOON for", "next to nothing this morning!"]),
    text(f'{MH}_Text_Sailor',
         ["Every port in HOENN has a MART,", "but only SLATEPORT has a MARKET HALL."],
         ["I list the POKéMON I fish up at sea", "and pick up my money on the next trip."]),
    text(f'{MH}_Text_Hiker',
         ["Those trainers who roam HOENN sell", "here too, you know."],
         ["Their POKéMON are well trained.", "Worth a look before you buy!"]),
    text(f'{MH}_Text_Boy',
         ["Mom said I can buy one POKéMON", "if I save my allowance."],
         ["The board in a POKé MART shows the", "same listings. This hall is nicer!"]),
    text(f'{MH}_Text_Scale',
         ["A fruit seller's scale.", "It's perfectly balanced."]),
]


def market_hall(slate_warp):
    mat = market_room()
    objs = [npc('OBJ_EVENT_GFX_MARKET_MASTER', 8, 2, f'{MH}_EventScript_MarketMaster')]
    scripts = [MH_SCRIPTS]
    for label, gfx, x, y, move, r in (('Shopper', 'OBJ_EVENT_GFX_WOMAN_2', 4, 7, 'MOVEMENT_TYPE_WANDER_UP_AND_DOWN', 1),
                                      ('Sailor', 'OBJ_EVENT_GFX_SAILOR', 12, 9, 'MOVEMENT_TYPE_FACE_LEFT', 0),
                                      ('Hiker', 'OBJ_EVENT_GFX_HIKER', 15, 10, 'MOVEMENT_TYPE_FACE_UP', 0),
                                      ('Boy', 'OBJ_EVENT_GFX_LITTLE_BOY', 3, 10, 'MOVEMENT_TYPE_WANDER_AROUND', 1)):
        o, s = talker(f'{MH}_EventScript_{label}', f'{MH}_Text_{label}', gfx, x, y, move, r, r)
        objs.append(o)
        scripts.append(s)
    scripts.append(f'{MH}_EventScript_Scale::\n\tmsgbox {MH}_Text_Scale, MSGBOX_SIGN\n\tend\n')
    bgs = [sign(2, 1, 'Netplay_EventScript_MarketBoard'), sign(3, 1, 'Netplay_EventScript_MarketBoard'),
           sign(5, 8, f'{MH}_EventScript_Scale'), sign(11, 8, f'{MH}_EventScript_Scale')]
    for b in bgs[2:]:
        b['player_facing_dir'] = 'BG_EVENT_PLAYER_FACING_ANY'
    warps = [{'x': x, 'y': y, 'elevation': 0, 'dest_map': 'MAP_SLATEPORT_CITY', 'dest_warp_id': str(slate_warp)}
             for x, y in mat]
    new_map(MH, 'MAP_SLATEPORT_CITY_MARKET_HALL', 'LAYOUT_SLATEPORT_CITY_MARKET_HALL', 'MUS_POKE_MART',
            'MAPSEC_SLATEPORT_CITY', 'gMapGroup_IndoorSlateport', objs, warps, bgs,
            '\n'.join(scripts) + '\n' + '\n'.join(MH_TEXTS), after='SlateportCity_Mart')


# ---------------------------------------------------------------- Lilycove: the AUCTION HOUSE
LILY_X, LILY_Y = 45, 19                # top left of the 6 x 5 building; the doors are at (47, 23) and (48, 23)
AH = 'LilycoveCity_AuctionHouse'


def lilycove_outside():
    lay = layouts()['LAYOUT_LILYCOVE_CITY']
    grid = load_blocks(lay)
    put_object(grid, art('lilycove')['auction_house'], LILY_X, LILY_Y, walk={(4, 2), (4, 3)})
    save_blocks(lay['blockdata_filepath'], grid)
    return [add_warp('LilycoveCity', {'x': LILY_X + 2 + i, 'y': LILY_Y + 4, 'elevation': 0,
                                      'dest_map': 'MAP_LILYCOVE_CITY_AUCTION_HOUSE', 'dest_warp_id': str(i)})
            for i in range(2)]


def auction_room():
    """The old R/S contest hall Emerald left unused (red curtains, a raised
    stage), turned into the sale room: a podium, a lot board and chairs."""
    g = blocks_of('LAYOUT_FALLARBOR_TOWN_LEFTOVER_RSCONTEST_HALL')
    con = art('contest')
    for x, y in ((11, 9), (12, 9), (13, 9), (12, 10)):   # contest marks off the stage floor
        g[y, x] = 0x3271
    rows = con['podium']
    g[8, 10] = rows[0][0] | WALK                         # the auctioneer stands here, behind the top
    g[9, 10] = rows[1][0] | SOLID                        # the front: a counter to talk across
    put_object(g, con['lot_board'], 1, 5)
    put_object(g, con['lot_board'], 18, 5)
    for x in (6, 7, 8, 12, 13, 14):
        put_object(g, con['chair'], x, 16)
    h, w = g.shape
    new_layout('LAYOUT_LILYCOVE_CITY_AUCTION_HOUSE', AH, w, h, 'gTileset_Building', 'gTileset_Contest', g,
               'LAYOUT_FALLARBOR_TOWN_LEFTOVER_RSCONTEST_HALL')
    return (10, 17), (11, 17)


AH_SCRIPTS = f"""
{AH}_MapScripts::
	.byte 0

@ GAVIN runs the sales from the podium: a word from him, then the auction
@ (data/scripts/life/auction.inc): won lots and sales are paid out first.
{AH}_EventScript_Auctioneer::
	lock
	faceplayer
	goto_if_set FLAG_TEMP_1, {AH}_EventScript_AuctioneerAgain
	setflag FLAG_TEMP_1
	msgbox {AH}_Text_AuctioneerHello, MSGBOX_DEFAULT
	goto Life_EventScript_AuctionCollect
	end

{AH}_EventScript_AuctioneerAgain::
	msgbox {AH}_Text_AuctioneerAgain, MSGBOX_DEFAULT
	goto Life_EventScript_AuctionCollect
	end

{AH}_EventScript_LotBoard::
	msgbox {AH}_Text_LotBoard, MSGBOX_SIGN
	goto Life_EventScript_Auction
	end
"""

AH_TEXTS = [
    text(f'{AH}_Text_AuctioneerHello',
         ["Order, order! Welcome to the", "LILYCOVE AUCTION HOUSE!"],
         ["I'm GAVIN, the AUCTIONEER.", "Trainers all over the world bid here."],
         ["Put a POKéMON up for sale, or bid", "on another trainer's lot."],
         ["The highest bid when the hammer", "falls takes it home!"]),
    text(f'{AH}_Text_AuctioneerAgain',
         ["GAVIN: Back for more? Let me check", "your lots first!"]),
    text(f'{AH}_Text_LotBoard',
         ["The LOT BOARD lists every POKéMON", "going under the hammer today."]),
    text(f'{AH}_Text_Clerk',
         ["Welcome! I keep the books for the", "AUCTION HOUSE."],
         ["When your POKéMON sells, the house", "keeps 5% and you get the rest."],
         ["Bids are held until somebody", "outbids you. Then they come back."]),
    text(f'{AH}_Text_Lady',
         ["I've been bidding on a lovely", "MILOTIC all week."],
         ["Somebody keeps outbidding me by", "exactly ¥100… The nerve!"]),
    text(f'{AH}_Text_RichBoy',
         ["Daddy says a real collector never", "bids first. You wait, then pounce!"]),
    text(f'{AH}_Text_Collector',
         ["Even the trainers who roam HOENN", "bid here. They're shrewd, too!"]),
    text(f'{AH}_Text_Usher',
         ["The sale room is up the stairs.", "Talk to GAVIN at the podium."]),
]


def auction_house(lily_warps):
    mat = auction_room()
    objs = [npc('OBJ_EVENT_GFX_AUCTIONEER', 10, 8, f'{AH}_EventScript_Auctioneer')]
    scripts = [AH_SCRIPTS]
    for label, gfx, x, y, move, r in (('Clerk', 'OBJ_EVENT_GFX_TEALA', 19, 15, 'MOVEMENT_TYPE_FACE_LEFT', 0),
                                      ('Lady', 'OBJ_EVENT_GFX_BEAUTY', 7, 15, 'MOVEMENT_TYPE_FACE_UP', 0),
                                      ('RichBoy', 'OBJ_EVENT_GFX_RICH_BOY', 13, 15, 'MOVEMENT_TYPE_FACE_UP', 0),
                                      ('Collector', 'OBJ_EVENT_GFX_POKEFAN_M', 1, 10, 'MOVEMENT_TYPE_WANDER_UP_AND_DOWN', 2),
                                      ('Usher', 'OBJ_EVENT_GFX_MAN_3', 9, 16, 'MOVEMENT_TYPE_FACE_DOWN', 0)):
        o, s = talker(f'{AH}_EventScript_{label}', f'{AH}_Text_{label}', gfx, x, y, move, r, r)
        objs.append(o)
        scripts.append(s)
    bgs = [sign(x, 6, f'{AH}_EventScript_LotBoard') for x in (1, 2, 18, 19)]
    warps = [{'x': x, 'y': y, 'elevation': 0, 'dest_map': 'MAP_LILYCOVE_CITY', 'dest_warp_id': str(w)}
             for (x, y), w in zip(mat, lily_warps)]
    new_map(AH, 'MAP_LILYCOVE_CITY_AUCTION_HOUSE', 'LAYOUT_LILYCOVE_CITY_AUCTION_HOUSE', 'MUS_LILYCOVE_MUSEUM',
            'MAPSEC_LILYCOVE_CITY', 'gMapGroup_IndoorLilycove', objs, warps, bgs,
            '\n'.join(scripts) + '\n' + '\n'.join(AH_TEXTS), after='LilycoveCity_DepartmentStore_1F')


def move_dept_store_auctioneer():
    """The department store's auctioneer now points the way to the new house."""
    p = f'{ROOT}/data/maps/LilycoveCity_DepartmentStore_1F/map.json'
    d = json.load(open(p))
    for o in d['object_events']:
        if o.get('script') in ('Life_EventScript_Auctioneer', 'LilycoveCity_DepartmentStore_1F_EventScript_AuctionMoved'):
            o['script'] = 'LilycoveCity_DepartmentStore_1F_EventScript_AuctionMoved'
    with open(p, 'w') as f:
        json.dump(d, f, indent=2, ensure_ascii=False)
        f.write('\n')
    s = open(f'{ROOT}/data/maps/LilycoveCity_DepartmentStore_1F/scripts.inc').read()
    if 'AuctionMoved' not in s:
        s = s.rstrip('\n') + '\n\n' + """LilycoveCity_DepartmentStore_1F_EventScript_AuctionMoved::
	msgbox LilycoveCity_DepartmentStore_1F_Text_AuctionMoved, MSGBOX_NPC
	end

LilycoveCity_DepartmentStore_1F_Text_AuctionMoved:
	.string "The auctions have moved to their own\\n"
	.string "AUCTION HOUSE, east of the CONTEST HALL.\\p"
	.string "Look for the purple roof and the\\n"
	.string "golden gavel!$"
"""
        open(f'{ROOT}/data/maps/LilycoveCity_DepartmentStore_1F/scripts.inc', 'w').write(s)


def main():
    market_hall(slateport_outside())
    auction_house(lilycove_outside())
    move_dept_store_auctioneer()
    print('ok: MAP_SLATEPORT_CITY_MARKET_HALL, MAP_LILYCOVE_CITY_AUCTION_HOUSE')


if __name__ == '__main__':
    main()
