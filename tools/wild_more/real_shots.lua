-- Real Pokémon that couldn't be caught before, met in their new homes
-- (tools/wild_more/add_real.py).
-- Run: python3 tools/world_shots/run.py tools/wild_more/real_shots.lua <out dir> 600000
dofile(INSTANCE_DIR .. "/../driver_lib.lua")
local D, K = Driver, Driver.KEY

local NEW = {}
for _, n in ipairs({ "AERODACTYL","ALOMOMOLA","AMAURA","ANORITH","ARBOK","ARCHEN","ARCTOVISH","ARCTOZOLT","AUDINO","BASTIODON","BAYLEEF","BEEDRILL","BEWEAR","BIBAREL","BLIPBUG","BLISSEY","BOMBIRDIER","BOUFFALANT","BRAMBLEGHAST","BULBASAUR","BUTTERFREE","CAPSAKID","CARNIVINE","CATERPIE","CHARIZARD","CHARMANDER","CHERUBI","CHIMCHAR","CLEFAIRY","COMBUSKEN","COMFEY","CORSOLA_GALAR","COTTONEE","CRANIDOS","CROCALOR","CUBONE","CYCLIZAR","CYNDAQUIL","DACHSBUN","DARUMAKA","DEDENNE","DEWOTT","DHELMISE","DOLLIV","DRACOVISH","DRACOZOLT","DREEPY","DRIZZILE","DUCKLETT","DUNSPARCE","DURANT","ELDEGOSS","ELGYEM","EMOLGA","EMPOLEON","ESPATHRA","ESPURR","EXEGGCUTE","FALINKS","FARFETCHD","FARFETCHD_GALAR","FEAROW","FENNEKIN","FERALIGATR","FERROSEED","FIDOUGH","FLAMIGO","FLOATZEL","FLORAGATO","FROGADIER","FURRET","GHOLDENGO","GLAMEOW","GLIMMET","GOTHITA","GRAPPLOCT","GRIMMSNARL","GROVYLE","GROWLITHE","HOPPIP","HYPNO","IMPIDIMP","INFERNAPE","INKAY","INTELEON","IVYSAUR","KABUTO","KANGASKHAN","KLAWF","KLEFKI","KOMALA","LARVITAR","LICKILICKY","LIEPARD","LILEEP","LOPUNNY","MALAMAR","MARSHTOMP","MASCHIFF","MEOWTH","MEOWTH_GALAR","METANG","MIENFOO","MILCERY","MR_MIME","MUNNA","NIDORINA","NIDORINO","OMANYTE","ORBEETLE","ORTHWORM","OVERQWIL","PACHIRISU","PANCHAM","PANPOUR","PANSAGE","PANSEAR","PARAS","PARASECT","PATRAT","PAWNIARD","PERRSERKER","PIDGEOTTO","PIDGEY","PIGNITE","PINCURCHIN","PORYGON","PRIMARINA","PURRLOIN","PURUGLY","QUAQUAVAL","QUILLADIN","QWILFISH","RABOOT","RABSCA","ROCKRUFF","SAWK","SCEPTILE","SCRAGGY","SCYTHER","SEEL","SERPERIOR","SERVINE","SHELLDER","SHIELDON","SHROODLE","SKIPLOOM","SLOWPOKE","SMOLIV","SMOOCHUM","SNEASLER","SOLOSIS","SPEAROW","SPIDOPS","SPRIGATITO","SPRITZEE","STONJOURNER","STUNKY","SUDOWOODO","SWANNA","SWIRLIX","TADBULB","TANDEMAUS","TANGROWTH","TAUROS","THROH","TIRTOUGA","TOEDSCOOL","TORRACAT","TORTERRA","TOXEL","TYPHLOSION","TYROGUE","TYRUNT","VAROOM","VELUZA","WARTORTLE","WATCHOG","WIGLETT","WOOPER_PALDEA","WUGTRIO","YAMPER","ZIGZAGOON_GALAR","missing:" }) do NEW[n] = true end

local function visit(label, hour, map, x, y, battles)
    D.scene(hour, map, x, y, { give = "GENGAR", level = 100, lead = true })
    local seen, met = {}, {}
    for _ = 1, battles do
        local sp = D.findBattle(K.LEFT, K.RIGHT, 1)
        local name = SPECIES_NAME[sp] or tostring(sp)
        if not seen[name] then
            seen[name] = true
            met[#met + 1] = name .. (NEW[name] and "*" or "")
            if NEW[name] then
                D.wait(260)
                D.shot(label .. "-" .. name)
            end
        end
        D.runAway() -- a Ghost-type lead can always run
    end
    D.log("MET " .. label .. ": " .. table.concat(met, ", "))
end

D.run(function()
    D.boot()
    D.wait(60)
    visit("route114-day", 13, MAPS.MAP_ROUTE114, 19, 42, 30)
    visit("route115-day", 13, MAPS.MAP_ROUTE115, 5, 6, 30)
    visit("route112", 8, MAPS.MAP_ROUTE112, 34, 7, 30)
    visit("fiery-path", 8, MAPS.MAP_FIERY_PATH, 12, 15, 30)
    visit("mt-pyre-night", 22, MAPS.MAP_MT_PYRE_EXTERIOR, 12, 22, 30)
    visit("granite-cave", 8, MAPS.MAP_GRANITE_CAVE_1F, 22, 6, 30)
    visit("new-mauville", 8, MAPS.MAP_NEW_MAUVILLE_INSIDE, 14, 15, 30)
    visit("route203", 8, MAPS.MAP_ROUTE203, 15, 3, 30)
end)
