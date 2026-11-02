#!/usr/bin/env python3
"""Writes src/data/npc_names.h: a name for every person on every map.

Run by the Makefile whenever a map changes, so a person added to a map later
gets a name too. Story characters keep their own names, trainers use their
trainer name unless other trainers share it (GRUNT and the like), and
everyone else gets a first name and a surname that nobody else has. Names
are picked by hashing the map and the object's place, so adding a person
somewhere doesn't rename anyone else (src/npc_names.c shows them).
"""
import glob
import hashlib
import json
import os
import re
import sys

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
OUT = os.path.join(ROOT, "src", "data", "npc_names.h")
MAX_LEN = 10  # fits the 8-tile name box in the small font

MALE = """ABE ADAM ALAN ALBY ALEC ALFIE ALVIN AMOS ANDY ARCH ARNE ART ASA AXEL BART
BASIL BEN BERT BILLY BOB BOYD BRET BRUCE BURT CAL CARL CASEY CHAD CHET CLAY
CLEM CLINT CODY COLE CONOR CORY CURT DALE DAMON DAN DARYL DEAN DENIS DEREK DEV
DION DIRK DON DOUG DREW DUANE DUKE EARL ED EDGAR EDWIN ELI ELMER EMIL ENZO ERNIE
EVAN EZRA FELIX FINN FLINT FORD FRANK FRED GABE GARY GENE GIL GLEN GRANT GREG
GUS GUY HAL HANK HANS HARVY HECTOR HENRY HERB HOLT HUGH HUGO IAN IGOR IRA IVAN
JACK JAKE JAN JASON JAY JED JEFF JESSE JIM JOE JOEL JON JUDE JULES KAI KARL
KEITH KEN KENT KIRK KURT KYLE LANCE LARRY LEN LEO LEON LES LEVI LIAM LLOYD LOU
LUKE LYLE MACK MANNY MARC MARIO MARK MARTY MATS MAX MEL MICK MIKE MILES MILO
MITCH MONTY MORT NATE NED NEIL NICK NIGEL NOAH NOEL OLAF OLLIE OMAR OSCAR OTIS
OTTO OWEN PABLO PAT PAUL PERCY PETE PHIL QUINN RAFE RALPH RAMON RAY REED REX
RHYS RICK RIGBY ROB ROD ROLF RON ROSS ROY RUDY RUSS RYAN SAL SAM SCOT SEAN SETH
SID SILAS SIMON SOL STAN STEVE STU TAD TED TERRY THEO TIM TOBY TODD TOM TONY
TRENT TROY TY UMAR VAN VERN VIC VINCE WADE WALT WARD WES WILL WYATT YURI ZACK
ZANE""".split()

FEMALE = """ABBY ADA ADELE AGNES AIDA ALICE ALMA AMBER AMY ANITA ANNA ANNE APRIL
ARIA AVA BELLA BERYL BESS BETH BETTY CARA CARLA CAROL CELIA CHLOE CINDY CLAIR
CORA DAISY DANA DAWN DEB DELIA DELLA DIANA DINA DORA DORIS EDNA EILA ELLA ELLIE
ELSA EMMA ENID ERIN ESME ESTER ETTA EVA FAY FERN FIONA FLORA FRAN FREYA GAIL
GEMMA GERRI GINA GRACE GRETA GWEN HANNA HAZEL HEDY HELEN HOPE IDA ILSE IMOGEN
INA INGA IRIS IRMA ISLA IVY JADE JANE JANET JEAN JILL JOAN JODI JOY JUDY JULIA
JUNE KARA KATE KATY KAYLA KIM KIT LACEY LANA LARA LAURA LEAH LENA LILA LILY
LINDA LISA LIZ LOIS LOLA LORNA LUCY LUNA LYDIA MABEL MAE MAGDA MAISY MARA MARGO
MARIA MAUD MAVIS MAY MEG MERYL MIA MILLY MINA MIRA MOLLY MONA MYRA NADIA NAN
NELL NINA NORA NOVA OLGA OLIVE OPAL PAM PAULA PEARL PEG PENNY PIA POLLY POPPY
RAE RENA RHODA RITA ROBIN ROSA ROSE RUBY RUTH SADIE SALLY SARA SELMA SHAY SONIA
STELLA SUE SUSAN TARA TESS THEA TILLY TINA TRUDY UMA UNA VERA VIDA VIOLA VIV
WANDA WENDY WILMA WREN YARA YVETTE YVONNE ZARA ZELDA ZOE""".split()

SURNAMES = """ABBOT ACRE ALDER ASH ASTER BAILY BAKER BANKS BARR BATES BAY BEAM
BECK BELL BIRCH BLAIR BLAKE BLOOM BOND BOONE BRAND BRAY BRIAR BROOK BROWN BURKE
BYRNE CAIN CANE CARR CASH CHASE CLARK CLAY COLE COOK COPE CORR COX CRANE CROFT
CROSS DALE DANE DAVIS DAWES DAY DEAN DELL DOVE DRAKE DREW DUNN DYER EARL EAST
ELLIS ELM EVANS FALK FARR FAY FENN FERRY FIELD FINCH FISK FLINT FORD FOX FROST
FRY GALE GATES GILL GLASS GOLD GOOD GRAY GREEN GROVE HALE HALL HART HAWK HAYES
HEATH HILL HOLT HOOD HOPE HOWE HUNT IVES JAMES JUDD KANE KAY KEEL KEMP KENT KING
KNOX LAKE LAMB LANE LARK LAW LEE LOCK LONG LOVE LOWE LUND LYLE LYNN MARSH MASON
MAY MEAD MILES MILLS MOON MOORE MORSE MOSS NASH NEAL NOBLE NORTH OAK OAKES OWEN
PAGE PAINE PARK PEARL PECK PIKE PINE PIPER POOLE POPE PRICE QUAY RAIN RAMS REED
REID RICH RIDGE RILEY ROSS ROWE RUSH RYE SAGE SALT SANDS SHAW SHORE SLATE SMALL
SNOW SOUTH SPARK STARK STERN STONE STORM SWAN TATE TERN THORN TODD TOWER TRAVIS
TULL VALE VANCE VINE WADE WALSH WARD WARE WEBB WELLS WEST WHITE WILDE WINTER
WOLFE WOOD WREN WYNN YATES YORK YOUNG""".split()

# Characters the story already names (script label endings and graphics).
FIXED_BY_SCRIPT = {
    "Agatha": "AGATHA", "Archie": "ARCHIE", "Bill": "BILL", "Birch": "PROF. BIRCH",
    "Blaine": "BLAINE", "Brawly": "BRAWLY", "Briney": "MR. BRINEY", "MrBriney": "MR. BRINEY",
    "Brock": "BROCK", "Bruno": "BRUNO", "CaptStern": "CAPT. STERN", "Celio": "CELIO",
    "Daisy": "DAISY", "Drake": "DRAKE", "Erika": "ERIKA", "Gabby": "GABBY", "Giovanni": "GIOVANNI",
    "Glacia": "GLACIA", "Juan": "JUAN", "Koga": "KOGA", "Lance": "LANCE", "Lanette": "LANETTE",
    "Lorelei": "LORELEI", "LtSurge": "LT. SURGE", "Matt": "MATT", "Maxie": "MAXIE", "Misty": "MISTY",
    "Mom": "MOM", "MrFuji": "MR. FUJI", "MrStone": "MR. STONE", "Norman": "NORMAN", "Phoebe": "PHOEBE",
    "ProfCozmo": "PROF. COZMO", "ProfOak": "PROF. OAK", "Roxanne": "ROXANNE", "Sabrina": "SABRINA",
    "Scott": "SCOTT", "Sidney": "SIDNEY", "Steven": "STEVEN", "Tabitha": "TABITHA",
    "TateAndLiza": "TATE & LIZA", "Ty": "TY", "Wallace": "WALLACE", "Wally": "WALLY",
    "EntranceWally": "WALLY", "ExitWally": "WALLY", "Wattson": "WATTSON", "Winona": "WINONA",
    "Kiri": "KIRI", "Lostelle": "LOSTELLE", "Wanda": "WANDA", "Vivi": "VIVI", "Norma": "NORMA",
    "Shelly": "SHELLY", "Courtney": "COURTNEY", "Rydel": "RYDEL", "Selphy": "SELPHY",
}
FIXED_BY_GFX = {
    "PROF_BIRCH": "PROF. BIRCH", "PROF_OAK": "PROF. OAK", "MOM": "MOM", "MOM_FRLG": "MOM",
    "STEVEN": "STEVEN", "WALLY": "WALLY", "WALLACE": "WALLACE", "ARCHIE": "ARCHIE", "MAXIE": "MAXIE",
    "NORMAN": "NORMAN", "ROXANNE": "ROXANNE", "BRAWLY": "BRAWLY", "WATTSON": "WATTSON",
    "FLANNERY": "FLANNERY", "WINONA": "WINONA", "JUAN": "JUAN", "SIDNEY": "SIDNEY", "PHOEBE": "PHOEBE",
    "GLACIA": "GLACIA", "DRAKE": "DRAKE", "SCOTT": "SCOTT", "BRANDON": "BRANDON", "BILL": "BILL",
    "CELIO": "CELIO", "DAISY": "DAISY", "MR_FUJI": "MR. FUJI", "BROCK": "BROCK", "MISTY": "MISTY",
    "LT_SURGE": "LT. SURGE", "ERIKA": "ERIKA", "KOGA": "KOGA", "SABRINA": "SABRINA", "BLAINE": "BLAINE",
    "GIOVANNI": "GIOVANNI", "LORELEI": "LORELEI", "BRUNO": "BRUNO", "AGATHA": "AGATHA", "LANCE": "LANCE",
    "BLUE": "BLUE", "TATE": "TATE", "LIZA": "LIZA",
}

# Graphics that aren't people (Pokémon, items, rocks, vehicles, props...).
NOT_PEOPLE = set("""ARTICUNO AZUMARILL AZURILL BERRY_TREE BIRCHS_BAG BREAKABLE_ROCK
BREAKABLE_ROCK_FRLG CABLE_CAR CHANSEY CLEFAIRY CLIPBOARD CUBONE CUTTABLE_TREE CUTTABLE_TREE_FRLG
DEOXYS DEOXYS_TRIANGLE DODUO DUSCLOPS FEAROW FOSSIL FOSSIL_FRLG GROUDON_ASLEEP GROUDON_FRONT
GROUDON_SIDE HOOH ITEM_BALL JIGGLYPUFF KANGASKHAN KECLEON KECLEON_BRIDGE_SHADOW KIRLIA
KYOGRE_ASLEEP KYOGRE_FRONT KYOGRE_SIDE LAPRAS LAPRAS_DOLL LUGIA MACHOKE MACHOP MEOWTH METEORITE
MEW MEWTWO MOLTRES MOVING_BOX MR_BRINEYS_BOAT MUDKIP_DOLL NIDORAN_F NIDORAN_M NIDORINO OLD_AMBER
PICHU_DOLL PIDGEOT PIDGEY PIKACHU PIKACHU_DOLL PIKACHU_FRLG POKEDEX POLIWRATH POOCHYENA
PUSHABLE_BOULDER PUSHABLE_BOULDER_FRLG RAYQUAZA RAYQUAZA_STILL REGICE REGIROCK REGISTEEL SEAGALLOP
SEEL SKITTY SKITTY_DOLL SLOWBRO SLOWPOKE SNORLAX SPEAROW SS_ANNE SS_TIDAL SUBMARINE_SHADOW
SUDOWOODO SWABLU_DOLL TORCHIC_DOLL TOWN_MAP TREECKO_DOLL TRICK_HOUSE_STATUE TRUCK
VIGOROTH_CARRYING_BOX VIGOROTH_FACING_AWAY VOLTORB WIGGLYTUFF WINGULL ZAPDOS ZIGZAGOON_1
ZIGZAGOON_2 PSYDUCK""".split())

FEMALE_GFX = re.compile(r"WOMAN|GIRL|LASS|_F$|_F_|BEAUTY|TWIN|NURSE|PICNICKER|LADY|TEALA|HEX_MANIAC|"
                        r"CHANNELER|RECEPTIONIST|DAISY|MOM|AGATHA|LORELEI|ERIKA|MISTY|SABRINA|ROXANNE|"
                        r"FLANNERY|WINONA|PHOEBE|GLACIA|LIZA|SAPPHIRE|RIVAL_MAY")


def is_person(gfx):
    return gfx and not gfx.startswith("VAR_") and not gfx.startswith("SPECIES(") and gfx not in NOT_PEOPLE


def map_numbers():
    groups = json.load(open(os.path.join(ROOT, "data", "maps", "map_groups.json")))
    out = {}
    for g, group in enumerate(groups["group_order"]):
        for n, name in enumerate(groups[group]):
            out[name] = (g, n)
    return out


def trainer_names():
    names = {}
    for path in glob.glob(os.path.join(ROOT, "src", "data", "*.party")):
        current = None
        for line in open(path, encoding="utf-8"):
            m = re.match(r"===\s*(TRAINER_\w+)\s*===", line)
            if m:
                current = m.group(1)
            elif current and line.startswith("Name:"):
                names[current] = line[5:].strip().upper()
                current = None
    return names


def script_trainers():
    """Script label -> the first trainer it battles."""
    out = {}
    files = glob.glob(os.path.join(ROOT, "data", "maps", "*", "scripts.inc")) + \
        glob.glob(os.path.join(ROOT, "data", "scripts", "*.inc"))
    for path in files:
        label = None
        for line in open(path, encoding="utf-8", errors="replace"):
            m = re.match(r"^(\w+)::?", line)
            if m:
                label = m.group(1)
                continue
            m = re.search(r"trainerbattle\w*\s+(TRAINER_\w+)", line)
            if m and label and label not in out:
                out[label] = m.group(1)
    return out


def rover_names():
    path = os.path.join(ROOT, "src", "data", "rovers_cast.h")
    if not os.path.exists(path):
        return set()
    return {n.upper() for n in re.findall(r'\{_\("([^"]+)"\)', open(path, encoding="utf-8").read())}


def h(key):
    return int(hashlib.sha1(key.encode()).hexdigest()[:12], 16)


def main():
    numbers = map_numbers()
    tnames = trainer_names()
    strainers = script_trainers()
    taken_firsts = rover_names() | set(FIXED_BY_SCRIPT.values()) | set(FIXED_BY_GFX.values())
    male = [f for f in MALE if f not in taken_firsts]
    female = [f for f in FEMALE if f not in taken_firsts]

    people = []  # (group, num, localId, mapName, gfx, script)
    for path in sorted(glob.glob(os.path.join(ROOT, "data", "maps", "*", "map.json"))):
        m = json.load(open(path, encoding="utf-8"))
        if m.get("name") not in numbers:
            continue
        g, n = numbers[m["name"]]
        for i, o in enumerate(m.get("object_events", [])):
            if o.get("type") == "clone":
                continue
            gfx = o.get("graphics_id", "").replace("OBJ_EVENT_GFX_", "")
            if not is_person(gfx):
                continue
            people.append((g, n, i + 1, m["name"], gfx, o.get("script", ""), o.get("trainer_type", "")))

    # Trainer names shared by several map trainers aren't anyone's own name.
    trainer_of = {}
    counts = {}
    for p in people:
        t = strainers.get(p[5])
        if t and t in tnames:
            trainer_of[p] = tnames[t]
            counts[tnames[t]] = counts.get(tnames[t], 0) + 1

    used = set()
    out = []
    pending = []
    shared = []
    for p in people:
        g, n, lid, name, gfx, script, _ = p
        suffix = script.split("EventScript_")[-1] if "EventScript_" in script else ""
        fixed = FIXED_BY_SCRIPT.get(suffix) or FIXED_BY_GFX.get(gfx)
        if gfx.startswith("MOM") and not suffix.startswith("Mom"):
            fixed = None  # someone else's mother (the rival's, Wally's...)
        if fixed:
            out.append((g, n, lid, fixed, name))
        elif p in trainer_of and counts[trainer_of[p]] == 1 and len(trainer_of[p]) <= MAX_LEN:
            out.append((g, n, lid, trainer_of[p], name))
            used.add(trainer_of[p])
        elif p in trainer_of and counts[trainer_of[p]] <= 3 and " " not in trainer_of[p]:
            shared.append(p)  # two CALVINs: the name the battle shows, and a surname
        else:
            pending.append(p)

    for p in sorted(shared, key=lambda p: (p[0], p[1], p[2])):
        g, n, lid, name = p[:4]
        first = trainer_of[p]
        lasts = [s for s in SURNAMES if len(first) + 1 + len(s) <= MAX_LEN] or [s for s in SURNAMES if len(s) <= 3]
        i = h(f"{name}:{lid}") % len(lasts)
        while f"{first} {lasts[i]}" in used:
            i = (i + 1) % len(lasts)
        used.add(f"{first} {lasts[i]}")
        out.append((g, n, lid, f"{first} {lasts[i]}", name))

    for p in sorted(pending, key=lambda p: (p[0], p[1], p[2])):
        g, n, lid, name, gfx, _, _ = p
        firsts = female if FEMALE_GFX.search(gfx) else male
        combos = [(f, s) for f in firsts for s in SURNAMES if len(f) + 1 + len(s) <= MAX_LEN]
        i = h(f"{name}:{lid}") % len(combos)
        while True:
            full = f"{combos[i][0]} {combos[i][1]}"
            if full not in used:
                break
            i = (i + 1) % len(combos)
        used.add(full)
        out.append((g, n, lid, full, name))

    out.sort()
    lines = ["// Generated by tools/npc_names/make_names.py from the maps. Do not edit.", "",
             "static const struct NpcName sNpcNames[] =", "{"]
    for g, n, lid, full, name in out:
        lines.append(f'    {{{(g << 8) | n:#06x}, {lid}, COMPOUND_STRING("{full}")}}, // {name}')
    lines += ["};", ""]
    text = "\n".join(lines)
    if not os.path.exists(OUT) or open(OUT, encoding="utf-8").read() != text:
        with open(OUT, "w", encoding="utf-8") as f:
            f.write(text)
    print(f"npc_names: {len(out)} people, {len(set(o[3] for o in out))} names", file=sys.stderr)


if __name__ == "__main__":
    main()
