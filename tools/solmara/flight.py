"""The flight desk in every Solmara Pokémon Center: fly to any Solmara town
the player has visited (VAR_0x8008 = the town you're in, left off the list)."""
import sol
from sol import text, register_script_file, map_id

# Where the flight lands in each town: just outside its Pokémon Center door.
LANDING = {}


def write(ctx, towns, names):
    lines = ['@ The Solmara flight desk (tools/solmara/flight.py).', 'Solmara_EventScript_FlightDesk::', '\tlock',
             '\tfaceplayer', '\tmsgbox Solmara_Text_FlightAsk, MSGBOX_DEFAULT']
    for i, t in enumerate(towns):
        lines += [f'\tgoto_if_eq VAR_0x8008, {i}, Solmara_EventScript_FlightSkip{i}',
                  f'\tgoto_if_unset {ctx.visited(t)}, Solmara_EventScript_FlightSkip{i}',
                  f'\tdynmultipush Solmara_Text_Town{i}, {i}',
                  f'Solmara_EventScript_FlightSkip{i}:']
    lines += ['\tdynmultipush Solmara_Text_FlightCancel, 127',
              '\tdynmultistack 0, 0, FALSE, 6, FALSE, 0, DYN_MULTICHOICE_CB_NONE',
              '\tgoto_if_eq VAR_RESULT, 127, Solmara_EventScript_FlightNo',
              '\tgoto_if_eq VAR_RESULT, MULTI_B_PRESSED, Solmara_EventScript_FlightNo']
    for i, t in enumerate(towns):
        lines.append(f'\tgoto_if_eq VAR_RESULT, {i}, Solmara_EventScript_Fly{i}')
    lines += ['Solmara_EventScript_FlightNo::', '\tmsgbox Solmara_Text_FlightNo, MSGBOX_DEFAULT', '\trelease', '\tend', '']
    for i, t in enumerate(towns):
        x, y = LANDING[t]
        lines += [f'Solmara_EventScript_Fly{i}::', '\tmsgbox Solmara_Text_FlightGo, MSGBOX_DEFAULT', '\tclosemessage',
                  '\tplayse SE_M_FLY', '\tfadescreen FADE_TO_WHITE', f'\twarpsilent {map_id(t)}, {x}, {y}', '\twaitstate',
                  '\trelease', '\tend', '']
    for i, n in enumerate(names):
        lines += [f'Solmara_Text_Town{i}:', f'\t.string "{n}$"', '']
    lines += ['Solmara_Text_FlightCancel:', '\t.string "CANCEL$"', '',
              'Solmara_Text_FlightAsk:', text('SWANNA AIR, at your service!', 'Where shall we fly today?'),
              'Solmara_Text_FlightGo:', text('Hold on tight!'),
              'Solmara_Text_FlightNo:', text('Come fly with us any time!')]
    register_script_file('solmara_flight', '\n'.join(lines))
