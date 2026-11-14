"""Exercise the scrolling battlefields through the emulator input callback."""

import re
import sys
from pathlib import Path

from smoke import HEIGHT, WIDTH, render, write_png

B, SELECT, START, UP, DOWN, LEFT, RIGHT, A, L = 0, 2, 3, 4, 5, 6, 7, 8, 10
CURSOR_GOLD = bytes((255, 247, 148))


def cursor_corners(rgb):
    return {
        (i % WIDTH, i // WIDTH)
        for i in range(WIDTH * HEIGHT)
        if rgb[3 * i:3 * i + 3] == CURSOR_GOLD
    }


def check_cursor(frame, x, y):
    left, top = x * 16, y * 16
    gold = cursor_corners(frame)
    assert len(gold) == 44, "Expected one cursor with four gold brackets"
    assert all(left <= px < left + 16 and top <= py < top + 16 for px, py in gold)
    assert {(left, top), (left + 15, top + 15)} <= gold


def check_terrain(before, after, *sprite_cells):
    for y in range(HEIGHT):
        for x in range(WIDTH):
            if (x // 16, y // 16) in sprite_cells:
                continue
            i = 3 * (y * WIDTH + x)
            assert before[i:i + 3] == after[i:i + 3], "Cursor movement changed terrain"


# Every terrain has a distinct set of colors. Check its center pixel, which
# stays visible even when the cursor surrounds the square.
TERRAIN_COLORS = {
    ".": ((8, 13, 8), (6, 10, 7), (11, 16, 9)),
    "=": ((17, 13, 8), (14, 10, 7), (20, 16, 11)),
    "#": ((7, 8, 10), (16, 17, 18), (20, 21, 21), (11, 12, 14), (24, 24, 22)),
    "~": ((4, 10, 15), (5, 12, 18), (8, 16, 21)),
    "T": ((3, 8, 6), (5, 12, 7), (10, 17, 8)),
    ",": ((8, 11, 9), (12, 14, 8), (7, 13, 12)),
    ":": ((10, 10, 12), (15, 15, 17), (19, 18, 20)),
    "+": ((11, 7, 5), (21, 14, 8), (25, 18, 11)),
}


def check_view(frame, rows, camera_x, camera_y):
    for y in range(HEIGHT // 16):
        for x in range(WIDTH // 16):
            world_x, world_y = camera_x + x, camera_y + y
            symbol = rows[world_y][world_x]
            offset = 3 * ((16 * y + 8) * WIDTH + 16 * x + 8)
            color = tuple(channel >> 3 for channel in frame[offset:offset + 3])
            assert color in TERRAIN_COLORS[symbol], f"Incorrect terrain at {(world_x, world_y)}"


def check_fighter(frame, x, y, counter, in_water=False, *, budget=30):
    screen_y = y * 16 + 8

    def color_at(sprite_x, sprite_y):
        offset = 3 * ((screen_y + sprite_y) * WIDTH + 16 * x + sprite_x)
        return tuple(v >> 3 for v in frame[offset:offset + 3])

    if in_water:
        assert color_at(1, 10) == (26, 30, 30), "Missing swimming wake"
        assert color_at(5, 14) in TERRAIN_COLORS["~"], "Standing boots visible underwater"
    else:
        assert color_at(8, 6) == (19, 5, 6), "Missing standing fighter"
        assert color_at(5, 14) == (7, 8, 11), "Missing standing boots"
    assert read_status(frame, 0, length=14) == f"MOVE {counter}/{budget} FT"


def check_animation(core, rom, enter):
    swim_inputs = enter | {50: (RIGHT,)}
    glide = render(core, rom, swim_inputs, frames=120)
    stroke = render(core, rom, swim_inputs, frames=128)
    repeated = render(core, rom, swim_inputs, frames=136)
    for frame in (glide, stroke):
        check_fighter(frame, 10, 4, "10", True)
    assert glide != stroke and glide == repeated, "Swim poses should repeat every 16 frames"
    check_terrain(glide, stroke, (10, 4), (10, 5))

    walk_inputs = enter | {50: (DOWN,)}
    stepping = render(core, rom, walk_inputs, frames=54)
    standing = render(core, rom, walk_inputs, frames=70)
    check_fighter(standing, 9, 5, "05")
    boot = 3 * ((5 * 16 + 8 + 15) * WIDTH + 9 * 16 + 10)
    assert stepping[boot:boot + 3] != standing[boot:boot + 3], "A step should lift one boot"
    check_terrain(stepping, standing, (9, 4), (9, 5), (9, 6))

    def tunic_top(frame):
        return next(y for y in range(72, 104)
                    if tuple(v >> 3 for v in frame[3 * (y * WIDTH + 152):
                                                   3 * (y * WIDTH + 152) + 3]) == (19, 5, 6))

    assert 78 < tunic_top(stepping) < tunic_top(standing), "Expected an in-between sprite position"
    idle = render(core, rom, enter, frames=120)
    breath = render(core, rom, enter, frames=152)
    assert idle != breath and idle == render(core, rom, enter, frames=184)
    check_terrain(idle, breath, (9, 4), (9, 5))


def check_camera_animation(core, rom):
    inputs = {36: (SELECT,)} | {frame: (RIGHT,) for frame in (40, 46, 52, 58, 64)}
    before = render(core, rom, inputs, frames=64)
    for frame_number, scroll in ((66, 4), (67, 8), (68, 12), (69, 16)):
        frame = render(core, rom, inputs, frames=frame_number)
        # The ruin is well away from the moving sprite and the fixed HUD.
        for y in range(25, 54):
            start = 3 * (y * WIDTH + 20)
            end = 3 * (y * WIDTH + 100)
            assert frame[start:end] == before[start + 3 * scroll:end + 3 * scroll]


def check_top_right(core, rom, screenshot):
    inputs = {28: (LEFT,), 50: (SELECT,)}
    inputs |= {frame: (UP,) for frame in (32, 36, 40, 44)}
    for pair in range(29):
        frame = 56 + 12 * pair
        inputs |= {frame: (RIGHT,), frame + 4: (RIGHT,), frame + 8: (B,)}
    at_edge = render(core, rom, inputs)
    check_fighter(at_edge, 14, 0, "00")
    assert read_status(at_edge, length=14) == "MOVE 00/30 FT"
    write_png(screenshot.with_name("fighter-top-right.png"), at_edge)


def check_bottom_edge(core, rom):
    inputs = {49: (SELECT,)} | {frame: (RIGHT,) for frame in (56, 60, 64, 68)}
    for pair in range(31):
        frame = 80 + 12 * pair
        inputs |= {frame: (B,), frame + 4: (DOWN,), frame + 8: (DOWN,)}
    at_edge = render(core, rom, inputs)
    pixel = 3 * ((144 + 6) * WIDTH + 13 * 16 + 8)
    assert tuple(value >> 3 for value in at_edge[pixel:pixel + 3]) == (19, 5, 6)
    assert read_status(at_edge, length=14) == "MOVE 00/30 FT"


def read_status(frame, tile_row=0, *, start=0, length=30, ink=31):
    art = (Path(__file__).resolve().parents[1] / "src/battlefield-fighter.h").read_text()
    shapes = re.findall(r"\{([\d, ]+)\},\s*/\*", art)
    letters = re.search(r'hud_letters\[\] = "([^"]+)"', art)[1]
    glyphs = {tuple(map(int, shape.split(","))): letter
              for shape, letter in zip(shapes, letters, strict=True)}
    status = []
    for column in range(start, start + length):
        pixels = []
        for row in range(7):
            bits = 0
            for x in range(5):
                offset = 3 * ((row + tile_row * 8) * WIDTH + column * 8 + 1 + x)
                bits = (bits << 1) | (frame[offset] >> 3 == ink)
            pixels.append(bits)
        status.append(glyphs[tuple(pixels)])
    return "".join(status).strip()


def check_character_panel(core, rom, screenshot):
    assert render(core, rom, {40: (L,)}) == render(core, rom), "L alone must keep Alden in the trial"
    open_panel = {40: (START,)}
    panel = render(core, rom, open_panel)
    assert read_status(panel, start=15, length=15) == "SEL GEAR"
    for row, label, ink in (
        (4, "ALDEN", 31), (5, "FIGHTER", 24), (6, "HP  12/12", 24),
        (7, "AC   16", 24), (8, "SPD 30 FT", 24), (9, "ATH  +04", 24), (10, "SWIM OFF", 24),
    ):
        assert read_status(panel, row, start=7, length=9, ink=ink) == label
    swim_panel = render(core, rom, {30: (SELECT,), 40: (START,)})
    assert read_status(swim_panel, 10, start=7, length=9, ink=24) == "SWIM ON"
    portrait = {
        panel[3 * (y * WIDTH + x):3 * (y * WIDTH + x) + 3]
        for y in range(40, 72) for x in range(16, 48)
    }
    assert len(portrait) >= 8, "Expected the illustrated portrait"
    assert render(core, rom, open_panel | {50: (LEFT,), 60: (A,)}) == panel
    closed = render(core, rom, open_panel | {60: (START,)})
    check_fighter(closed, 9, 4, "00")
    check_terrain(closed, render(core, rom), (9, 4), (9, 5))
    assert render(core, rom, open_panel | {60: (B,), 80: (START,)}) == panel
    moving = render(core, rom, {30: (RIGHT,), 40: (START,), 60: (B,)})
    check_fighter(moving, 10, 4, "10", True)
    assert read_status(moving) == "MOVE 10/30 FT"
    write_png(screenshot.with_name("character-panel.png"), panel)
    toggle = open_panel | {44: (SELECT,)}
    unarmored = render(core, rom, toggle)
    assert read_status(unarmored, 7, start=7, length=9, ink=24) == "AC   11"
    restored = render(core, rom, toggle | {50: (SELECT,)})
    assert read_status(restored, 7, start=7, length=9, ink=24) == "AC   16"
    check_terrain(restored, panel, (9, 4), (9, 5))
    gear_off = toggle | {50: (B,)}
    base = render(core, rom, {40: (START,), 50: (B,)})
    stripped = render(core, rom, gear_off)
    assert stripped != base
    check_terrain(stripped, base, (9, 4), (9, 5))
    smash = {65: (A,), 70: (A,), 75: (A,)}
    equipped_scene = render(core, rom, {40: (START,), 50: (B,)} | smash, frames=85)
    bare_scene = render(core, rom, gear_off | smash, frames=85)
    assert read_smash(bare_scene) == "SMASH  FENCE"
    assert bare_scene != equipped_scene
    write_png(screenshot.with_name("smash-unarmed.png"), bare_scene)
    print("Character panel OK. Stats, portrait, modal input and restored HUD verified.")


def read_smash(frame):
    return read_status(frame, 18, start=1, length=28)


def check_turn_banner(core, rom, screenshot):
    inputs = {50: (B,)}
    centered = render(core, rom, inputs, frames=55)
    sliding = render(core, rom, inputs, frames=69)
    for frame, start in ((centered, 57), (sliding, 129)):
        offset = 3 * ((75 if start == 57 else 69) * WIDTH + start)
        assert tuple(channel >> 3 for channel in frame[offset:offset + 3]) == (24, 15, 6)
    assert render(core, rom, inputs, frames=80) == render(core, rom, frames=80)
    assert render(core, rom, inputs | {55: (A,)}) == render(core, rom, {55: (A,)})
    moved = render(core, rom, inputs | {55: (DOWN,)})
    check_fighter(moved, 9, 5, "05")
    write_png(screenshot.with_name("new-turn.png"), centered)


def check_equipment_menu(core, rom, screenshot):
    opened = {40: (A,), 44: (LEFT,), 48: (LEFT,), 52: (A,)}
    def notice(inputs, frames=120):
        return read_status(render(core, rom, inputs, frames=frames), start=15, length=15)

    panel = render(core, rom, opened)
    assert read_status(panel, 6, start=8, length=5) == "ITEMS"
    assert read_status(panel, 7, start=8, length=6) == "HAMMER"
    assert read_status(panel, 9, start=8, length=6, ink=23) == "SHIELD"
    assert read_status(panel, 9, start=16, length=6, ink=23) == "ON"
    assert read_status(panel, 11, start=7, length=16, ink=23) == "FREE STOW"
    write_png(screenshot.with_name("equipment-menu.png"), panel)
    highlighted = opened | {64: (DOWN,)}
    assert read_status(render(core, rom, highlighted), 11, start=7, length=16, ink=23) == "STOW AND DOFF"
    confirm = highlighted | {68: (A,)}
    prompt = render(core, rom, confirm, frames=72)
    assert read_status(prompt, 7, start=7, length=16, ink=23) == "STOW HAMMER FREE"
    assert read_status(prompt, 8, start=7, length=16, ink=23) == "REMOVE SHIELD"
    assert read_status(prompt, 9, start=7, length=16, ink=23) == "USES ACTION"
    assert render(core, rom, confirm | {74: (B,)}) == render(core, rom, highlighted)
    write_png(screenshot.with_name("bow-confirmation.png"), prompt)
    prep = confirm | {74: (A,)}
    assert notice(prep, frames=80) == "ACTION SPENT"
    selected_bow = render(core, rom, prep, frames=95)
    assert read_status(selected_bow, 19) == "LONGBOW SELECTED"
    pixel = 3 * (80 * WIDTH + 158)
    assert tuple(v >> 3 for v in selected_bow[pixel:pixel + 3]) == (28, 20, 9)
    aiming = prep | {100: (B,), 110: (A,), 115: (DOWN,), 120: (DOWN,), 125: (A,)}
    frame = render(core, rom, aiming, frames=132)
    assert read_status(frame, start=15, length=15) == "DRAW AND SHOOT"
    assert read_status(frame, 3, start=2, length=12) == "BARREL"
    write_png(screenshot.with_name("bow-targeting.png"), frame)
    barrel = aiming | {135: (A,)}
    shot = render(core, rom, barrel, frames=155)
    assert read_status(shot, 18, start=1, length=5) == "SHOOT"
    assert read_status(shot, 18, start=8, length=6) == "BARREL"
    returned = barrel | {180: (A,), 190: (START,)}
    assert notice(returned, frames=205) == "BOW 19 ARROWS"
    idle = render(core, rom, barrel | {180: (A,)}, frames=245)
    offset = 3 * (80 * WIDTH + 158)
    assert tuple(v >> 3 for v in idle[offset:offset + 3]) == (28, 20, 9)
    write_png(screenshot.with_name("bow-equipped.png"), idle)

    # With the hammer already stowed, the next turn can doff and draw together.
    stowed = opened | {68: (A,), 100: (B,), 110: (A,), 114: (LEFT,),
                       118: (LEFT,), 122: (A,), 126: (DOWN,)}
    hint = render(core, rom, stowed, frames=132)
    assert read_status(hint, 11, start=7, length=16, ink=23) == "DOFF AND DRAW"
    drawn = render(core, rom, stowed | {135: (A,), 141: (A,)}, frames=175)
    assert read_status(drawn, 19) == "FREE INTERACTION BOW DRAWN"
    assert tuple(v >> 3 for v in drawn[offset:offset + 3]) == (28, 20, 9)

    # Doff only the shield. Keeping the hammer must keep the melee command.
    shield_only = opened | {64: (UP,), 68: (UP,), 72: (A,), 100: (B,),
                            110: (A,), 120: (A,), 135: (A,)}
    assert read_smash(render(core, rom, shield_only, frames=155)) == "SMASH  FENCE"
    print("Items menu OK. Bow preparation, Draw and Shoot, and shieldless melee verified.")


def check_longbow(core, rom, screenshot):
    setup = {40: (START,), 44: (RIGHT,), 55: (B,), 60: (A,), 65: (DOWN,), 70: (A,)}
    panel = render(core, rom, setup, frames=52)
    assert read_status(panel, start=15, length=15) == "BOW 20 ARROWS"
    assert read_status(panel, 7, start=7, length=9, ink=24) == "AC   14"
    miss = setup | {75: (A,)}
    hit = setup | {76: (A,)}
    assert read_smash(render(core, rom, miss, frames=118)).endswith("AC15 MISS")
    assert read_smash(render(core, rom, hit, frames=120)).endswith("AC15 BROKE")
    returned = render(core, rom, hit | {124: (A,), 129: (START,)})
    assert read_status(returned, start=15, length=15) == "BOW 19 ARROWS"
    write_png(screenshot.with_name("longbow-flight.png"), render(core, rom, hit, frames=97))
    print("Longbow OK. Distant target, hit, miss, ammo consumption and shield removal verified.")


def check_repeated_weapon_swaps(core, rom):
    inputs = {40: (START,), 44: (RIGHT,), 55: (B,)}
    now = 65

    def keys(*sequence):
        nonlocal now
        for key in sequence:
            inputs[now] = (key,)
            now += 6

    def attack(*targets, expected):
        nonlocal now
        keys(A, *targets, A, A)
        frame = render(core, rom, inputs, frames=now + 13)
        assert read_status(frame, 18, start=1, length=5) == expected, (now, read_status(frame), read_smash(frame))
        now += 44
        keys(A, B)

    def items(*selection):
        nonlocal now
        keys(A)
        root = render(core, rom, inputs, frames=now)
        # A broken fence can leave the root focused on Bonus instead of Action.
        offset = 3 * (118 * WIDTH + 33)
        on_action = tuple(v >> 3 for v in root[offset:offset + 3]) == (31, 25, 11)
        keys(*((LEFT, LEFT) if on_action else (RIGHT,)), A, *selection, A)
        now += 4

    attack(DOWN, DOWN, expected="SHOOT")
    items()  # Stow bow and draw hammer in one confirmation.
    swapped = render(core, rom, inputs, frames=now + 6)
    assert read_status(swapped, 19) == "UTILIZE HAMMER DRAWN"
    assert read_status(swapped, 0, start=15, length=15) == "HAMMER DRAWN"
    keys(B)
    attack(expected="SMASH")
    items(DOWN)  # Request the bow explicitly.
    keys(A)
    attack(UP, expected="SHOOT")
    items()  # Swap back to hammer again.
    keys(B, DOWN)
    attack(expected="SMASH")
    print("Repeated swaps OK. Bow, hammer, bow, hammer all used their own attack.")


def check_new_breakables(core, rom):
    targets = ("BULLSEYE", "TARGET", "DUMMY", "STOOL", "BASKET", "PALLET")
    for count, target in enumerate(targets, 1):
        aim = {40: (START,), 44: (RIGHT,), 55: (B,), 65: (A,), 100: (A,)}
        aim.update({70 + 4 * step: (UP,) for step in range(count)})
        for fire_frame in range(108, 128):
            inputs = aim | {fire_frame: (A,)}
            result = render(core, rom, inputs, frames=fire_frame + 43)
            if read_smash(result).endswith("BROKE"):
                windup = render(core, rom, inputs, frames=fire_frame + 12)
                assert read_status(windup, 18, start=8, length=8) == target
                break
        else:
            raise AssertionError(f"Could not destroy {target}")
    print("Breakable props OK. Dummy, both archery targets, pallet, basket and stool destroyed.")


def check_attack_range(core, rom, screenshot):
    melee_inputs = {50: (A,), 55: (A,)}
    bow_inputs = {40: (START,), 44: (RIGHT,), 55: (B,), 65: (A,), 70: (A,)}

    def pixel(frame, x, y):
        offset = 3 * (y * WIDTH + x)
        return tuple(v >> 3 for v in frame[offset:offset + 3])

    for name, inputs, opened, lit, unlit in (
        ("melee", melee_inputs, 55, (132, 78), (180, 78)),
        ("bow", bow_inputs, 70, (180, 78), (100, 62)),
    ):
        dim = render(core, rom, inputs, frames=opened + 5)
        peak = render(core, rom, inputs, frames=opened + 23)
        dim_again = render(core, rom, inputs, frames=opened + 53)
        closed = render(core, rom, inputs | {opened + 5: (B,), opened + 10: (B,)},
                        frames=opened + 20)
        x, y = lit
        # Check a patch of the original texture, including several shades.
        patch = [(x + dx, y + dy) for dx in range(4) for dy in range(4)]
        assert len({pixel(closed, *point) for point in patch}) > 1
        for point in patch:
            original = pixel(closed, *point)
            assert pixel(peak, *point) == tuple(v + (31 - v) * 3 // 16 for v in original)
            assert pixel(dim, *point) == tuple(v + (31 - v) // 16 for v in original)
            assert pixel(dim_again, *point) == pixel(dim, *point)
        assert pixel(peak, *unlit) == pixel(closed, *unlit)
        assert len(cursor_corners(peak)) == 44
        assert cursor_corners(peak) == cursor_corners(dim)
        write_png(screenshot.with_name(f"{name}-range.png"), peak)

    # The bow-to-hammer swap uses Utilize. Show melee reach on the map
    # immediately, even though the Action menu is unavailable this turn.
    swap = {40: (START,), 44: (RIGHT,), 55: (B,), 65: (A,),
            70: (LEFT,), 75: (LEFT,), 80: (A,), 85: (A,)}
    drawn = render(core, rom, swap, frames=90)
    peak = render(core, rom, swap, frames=108)
    faded = render(core, rom, swap, frames=175)
    assert read_status(drawn, 19) == "UTILIZE HAMMER DRAWN"
    near, far = (132, 78), (180, 78)
    assert pixel(faded, *near) < pixel(drawn, *near) < pixel(peak, *near), "Hammer reach missing after swap"
    assert pixel(drawn, *far) == pixel(peak, *far) == pixel(faded, *far), "Bow range lingered after swap"
    assert not cursor_corners(drawn)
    next_turn = render(core, rom, swap | {110: (B,), 140: (A,), 146: (A,)}, frames=169)
    assert pixel(next_turn, *near) == pixel(peak, *near)
    assert pixel(next_turn, *far) == pixel(faded, *far)
    assert len(cursor_corners(next_turn)) == 44
    write_png(screenshot.with_name("hammer-swap-range.png"), peak)
    print("Attack range pulse OK. Terrain texture and steady target brackets verified.")


def check_fence(core, rom, screenshot):
    def intact(frame):
        offset = 3 * (62 * WIDTH + 152)  # Middle of the upper wooden rail.
        return tuple(v >> 3 for v in frame[offset:offset + 3]) == (21, 12, 6)

    blocked = render(core, rom, {50: (UP,)})
    assert intact(blocked) and read_status(blocked) == "MOVE 00/30 FT"
    check_fighter(blocked, 9, 4, "00")

    def pixel(frame, x, y):
        offset = 3 * (y * WIDTH + x)
        return tuple(v >> 3 for v in frame[offset:offset + 3])

    open_menu = {46: (A,)}
    selected = render(core, rom, open_menu)
    assert intact(selected)
    assert pixel(selected, 144, 56) == (31, 30, 18), "Expected gold fence brackets"
    assert pixel(selected, 33, 118) == (31, 25, 11), "Expected gold Action frame"
    assert pixel(selected, 193, 118) == (24, 15, 6), "Expected bronze Cancel frame"
    assert pixel(selected, 8, 16) == (13, 20, 29), "Expected a blue side panel"
    assert pixel(selected, 105, 94) == pixel(blocked, 105, 94), "Target card area should show terrain"
    assert read_status(selected, 3, start=2, length=12) == "FENCE"
    assert read_status(selected, 4, start=2, length=12, ink=23) == "BREAKABLE"
    assert read_status(selected, 5, start=2, length=12, ink=23) == "SEL DETAILS"

    details_inputs = open_menu | {50: (SELECT,)}
    details = render(core, rom, details_inputs)
    assert read_status(details, 3, start=2, length=12) == "DETAILS    B"
    assert read_status(details, 4, start=2, length=12, ink=23) == "WOODEN FENCE"
    assert read_status(details, 5, start=2, length=12, ink=23) == "BLOCKS PATH"
    assert render(core, rom, details_inputs | {54: (RIGHT,), 58: (DOWN,)}) == details
    assert render(core, rom, details_inputs | {54: (B,)}) == selected
    assert render(core, rom, details_inputs | {54: (A,)}) == selected
    assert render(core, rom, details_inputs | {54: (SELECT,), 58: (B,)}) == blocked
    write_png(screenshot.with_name("object-details.png"), details)
    assert pixel(selected, 96, 98) == pixel(blocked, 96, 98), "Expected terrain between cards"
    write_png(screenshot.with_name("fence-selector.png"), selected)

    cancel_selected = render(core, rom, open_menu | {48: (LEFT,)})
    assert pixel(cancel_selected, 33, 118) == (24, 15, 6)
    assert pixel(cancel_selected, 193, 118) == (31, 25, 11)
    check_fighter(cancel_selected, 9, 4, "00")
    assert render(core, rom, open_menu | {48: (LEFT,), 50: (A,)}) == blocked
    assert render(core, rom, open_menu | {50: (B,)}) == blocked
    assert render(core, rom, open_menu | {48: (LEFT,), 50: (RIGHT,)}) == selected

    action_menu = open_menu | {48: (A,)}
    submenu = render(core, rom, action_menu)
    assert intact(submenu) and read_status(submenu, start=15, length=15) == "ACTION"
    assert pixel(submenu, 144, 56) == (31, 30, 18), "The selected target stays marked"
    assert render(core, rom, action_menu | {52: (B,)}) == selected
    write_png(screenshot.with_name("action-options.png"), submenu)
    for frame_number, success in ((52, True), (55, False)):
        inputs = action_menu | {frame_number: (A,)}
        closeup = render(core, rom, inputs, frames=frame_number + 42)
        attempt = render(core, rom, inputs | {frame_number + 44: (A,)})
        assert read_status(attempt) == "MOVE 00/30 FT"
        assert read_status(closeup, 17, start=22, length=6) == "X NEXT"
        assert read_status(closeup, 17, start=1, length=20) == (
            "WOOD SPLINTERS" if success else "THE WOOD HOLDS"
        )
        if success:
            windup = render(core, rom, inputs, frames=frame_number + 12)
            impact = render(core, rom, inputs, frames=frame_number + 28)
            assert read_smash(windup) == "SMASH  FENCE"
            assert len({windup, impact, closeup}) == 3
            entrance = render(core, rom, inputs, frames=frame_number + 5)
            assert entrance != windup
            skipped = render(core, rom, inputs | {frame_number + 7: (A,)}, frames=frame_number + 11)
            assert skipped == closeup
            write_png(screenshot.with_name("battle-entrance.png"), entrance)
            assert render(core, rom, inputs, frames=frame_number + 170) == closeup
            assert render(core, rom, inputs | {frame_number + 12: (UP,)},
                          frames=frame_number + 42) == closeup
            write_png(screenshot.with_name("smash-windup.png"), windup)
            write_png(screenshot.with_name("smash-impact.png"), impact)
            write_png(screenshot.with_name("smash-closeup.png"), closeup)
        result = re.fullmatch(r"D20 (\d{2}) \+04 = (\d{2}) DC12 (BROKE|FAIL)", read_smash(closeup))
        assert result, "Expected the Athletics die, total, DC and result"
        die, total = map(int, result.groups()[:2])
        assert 1 <= die <= 20 and total == die + 4
        assert (total >= 12) == success == (result[3] == "BROKE")
        assert intact(attempt) != success
        check_fighter(attempt, 9, 4, "00")

    failed = render(core, rom, action_menu | {55: (A,), 99: (A,), 101: (A,), 105: (B,), 109: (UP,)})
    assert intact(failed) and read_status(failed, length=14) == "MOVE 00/30 FT", "No second attempt in one turn"
    check_fighter(failed, 9, 4, "00")

    opened = render(core, rom, action_menu | {52: (A,), 97: (A,), 103: (UP,)})
    check_fighter(opened, 9, 3, "05")
    write_png(screenshot.with_name("fence-broken.png"), opened)

    retry = render(core, rom, action_menu | {55: (A,), 99: (A,), 102: (B,), 106: (A,), 110: (A,), 118: (A,), 163: (A,)})
    assert not intact(retry) and read_status(retry) == "MOVE 00/30 FT"

    out_of_reach = render(core, rom, {40: (LEFT,), 44: (LEFT,), 50: (A,)})
    assert intact(out_of_reach) and pixel(out_of_reach, 113, 118) == (31, 25, 11)
    check_fighter(out_of_reach, 7, 4, "10")
    assert render(core, rom, {40: (LEFT,), 44: (LEFT,), 50: (A,), 54: (SELECT,)}) == out_of_reach
    assert pixel(out_of_reach, 8, 16) != (13, 20, 29)
    assert pixel(out_of_reach, 120, 16) != (13, 20, 29)
    check_fighter(render(core, rom, {40: (LEFT,), 44: (LEFT,), 50: (A,), 54: (A,), 58: (A,)}),
                  7, 4, "10", budget=60)

    reset = render(core, rom, action_menu | {52: (A,), 97: (A,), 102: (L, START), 112: (START,)})
    assert intact(reset) and read_status(reset) == "MOVE 00/30 FT"
    print("Fence OK. Target brackets, command selection, cancel and Athletics results verified.")


def check_other_breakables(core, rom, screenshot):
    # A total of 12 clears the crate's DC 10 but fails the barrel's DC 14.
    crate = {40: (DOWN,), 46: (A,), 52: (A,), 61: (A,), 107: (A,)}
    barrel = {40: (DOWN,), 42: (RIGHT,), 46: (A,), 52: (A,), 61: (A,), 107: (A,)}
    crate_result = render(core, rom, crate | {114: (DOWN,)})
    barrel_result = render(core, rom, barrel | {114: (RIGHT,)})
    assert read_smash(render(core, rom, crate, frames=104)) == "D20 08 +04 = 12 DC10 BROKE"
    bridge = render(core, rom, barrel, frames=104)
    assert read_smash(bridge) == "D20 08 +04 = 12 DC14 FAIL"
    field = render(core, rom, crate, frames=104)
    def scenery(frame):
        return frame[3 * 101 * WIDTH:3 * 101 * WIDTH + 3 * 40]
    assert scenery(bridge) != scenery(field), "Bridge and road need distinct close-up scenery"
    assert read_status(bridge, 17, start=1, length=20) == "THE WOOD HOLDS"
    write_png(screenshot.with_name("smash-bridge.png"), bridge)
    check_fighter(crate_result, 9, 6, "10")
    check_fighter(barrel_result, 10, 5, "10")

    barrel_broken = render(core, rom, {40: (DOWN,), 42: (RIGHT,), 46: (A,),
                                       52: (A,), 66: (A,), 110: (A,), 115: (RIGHT,)})
    check_fighter(barrel_broken, 11, 5, "15")

    # Breaking one object leaves the others intact and available on later turns.
    two_objects = render(core, rom, crate | {114: (DOWN,), 118: (B,), 122: (UP,),
                                              126: (RIGHT,), 130: (A,), 134: (A,), 138: (A,)}, frames=180)
    assert re.fullmatch(r"D20 \d{2} \+04 = \d{2} DC14 (BROKE|FAIL)", read_smash(two_objects))
    targets = {40: (DOWN,), 42: (RIGHT,), 46: (A,)}
    barrel_selected = render(core, rom, targets)
    crate_selected = render(core, rom, targets | {50: (DOWN,)})
    def bracket(frame, x, y):
        offset = 3 * (y * WIDTH + x)
        return tuple(v >> 3 for v in frame[offset:offset + 3]) == (31, 30, 18)
    assert read_status(barrel_selected, 3, start=2, length=12) == "BARREL"
    assert read_status(crate_selected, 3, start=2, length=12) == "CRATE"
    assert bracket(barrel_selected, 176, 88)
    assert bracket(crate_selected, 144, 104)
    assert not bracket(crate_selected, 176, 88)
    assert render(core, rom, targets | {50: (DOWN,), 54: (UP,)}) == barrel_selected
    cycled_smash = render(core, rom, targets | {50: (DOWN,), 54: (A,), 61: (A,)}, frames=105)
    assert read_smash(cycled_smash) == "D20 08 +04 = 12 DC10 BROKE"
    print("Crate and barrel OK. Distinct DCs, independent state and collision verified.")


def check_dash(core, rom, screenshot):
    action_dash = {40: (A,), 44: (A,), 48: (RIGHT,), 52: (A,)}
    check_fighter(render(core, rom, action_dash), 9, 4, "00", budget=60)
    both = action_dash | {60: (A,), 64: (A,), 68: (A,)}
    check_fighter(render(core, rom, both), 9, 4, "00", budget=90)

    bonus = {40: (A,), 42: (RIGHT,), 44: (RIGHT,), 48: (A,)}
    selected = render(core, rom, bonus)
    assert read_status(selected, start=15, length=15) == "BONUS ACTION"
    assert read_status(selected, 3, start=2, length=12) == "FENCE"
    write_png(screenshot.with_name("dash-menu.png"), selected)
    assert read_status(render(core, rom, bonus | {52: (B,), 56: (B,)})) == "MOVE 00/30 FT"

    # The Action remains available after Bonus Dash.
    dashed = bonus | {52: (A,)}
    check_fighter(render(core, rom, dashed), 9, 4, "00", budget=60)
    smash = dashed | {60: (A,), 64: (A,), 68: (A,)}
    assert re.fullmatch(r"D20 \d{2} \+04 = \d{2} DC12 (BROKE|FAIL)",
                        read_smash(render(core, rom, smash, frames=110)))

    # Spend all base movement and use the Action on the barrel, then Bonus Dash.
    approach = {36: (RIGHT,), 40: (RIGHT,), 44: (RIGHT,), 50: (A,), 54: (A,), 58: (A,)}
    assert read_smash(render(core, rom, approach, frames=102)).endswith("DC14 BROKE")
    spent = approach | {104: (A,)}
    check_fighter(render(core, rom, spent), 12, 4, "30", True)
    resumed = spent | {106: (A,), 110: (A,), 114: (A,)}
    check_fighter(render(core, rom, resumed), 12, 4, "30", True, budget=60)
    farther = resumed | {frame: (RIGHT,) for frame in (120, 124, 128, 132)}
    check_fighter(render(core, rom, farther), 13, 4, "60", True, budget=60)
    denied = render(core, rom, farther | {140: (A,), 144: (RIGHT,), 148: (A,)})
    assert read_status(denied, start=15, length=15) == "BONUS SPENT"
    check_fighter(render(core, rom, farther | {144: (B,)}), 13, 4, "00", True)
    print("Bonus Dash OK. Separate resources, exhausted movement, terrain costs and turn reset verified.")


def check_movement_menu(core, rom, screenshot):
    root = {40: (A,)}
    movement = root | {44: (RIGHT,), 48: (A,)}
    opened = render(core, rom, movement)
    assert read_status(opened, start=15, length=15) == "30 FT LEFT"
    assert render(core, rom, movement | {52: (B,), 56: (B,)}) == render(core, rom)
    assert render(core, rom, movement | {52: (A,)}) == render(core, rom)
    step = render(core, rom, movement | {52: (A,), 60: (DOWN,)})
    check_fighter(step, 9, 5, "05")
    write_png(screenshot.with_name("movement-options.png"), opened)
    print("Movement menu OK. Remaining feet, Back and return to movement verified.")


def check_movement(core, rom, direct_rom, screenshot):
    enter = {30: (A,), 40: (START,)}
    starting_view = render(core, rom, enter)
    check_fighter(starting_view, 9, 4, "00")
    direct_view = render(core, direct_rom)
    check_fighter(direct_view, 9, 4, "00")
    check_terrain(starting_view, direct_view, (9, 4), (9, 5))
    write_png(screenshot.with_name("river-trial.png"), direct_view)

    # Each tuple is one press, with simultaneous directions for diagonals.
    crossing = [(RIGHT,)] * 4
    scenarios = (
        ("swim one square", [(RIGHT,)], (10, 4, "10", True)),
        ("ordinary swimmer stops at 30 feet", crossing, (12, 4, "30", True)),
        ("Swim Speed halves calm-water cost", [(SELECT,)] + crossing, (13, 4, "20", True)),
        ("swim onto the bridge", [(RIGHT,), (DOWN,)], (10, 5, "15")),
        ("swim back to shore", [(RIGHT,), (LEFT,)], (9, 4, "15")),
        ("walk onto the bridge", [(DOWN,), (RIGHT,)], (10, 5, "10")),
        ("new turn keeps position", crossing + [(B,)], (12, 4, "00", True)),
        ("new turn allows movement again", crossing + [(B,), (RIGHT,)], (13, 4, "10", True)),
        ("turn off Swim Speed", [(SELECT,), (RIGHT,), (SELECT,), (RIGHT,)],
         (11, 4, "10", True)),
        ("wall blocks entry", [(LEFT,), (LEFT,), (UP, LEFT)], (7, 4, "10")),
        ("wall blocks diagonal corner", [(LEFT,), (LEFT,), (UP,), (DOWN, LEFT)],
         (7, 3, "15")),
        ("pine blocks entry", [(DOWN, LEFT)] * 4, (6, 7, "15")),
        ("marsh costs 10 feet", [(LEFT,)] * 5 + [(UP,), (B,), (UP,), (UP,)],
         (4, 1, "15")),
    )
    for name, presses, expected in scenarios:
        inputs = {50 + 4 * index: buttons for index, buttons in enumerate(presses)}
        frame = render(core, rom, enter | inputs)
        try:
            check_fighter(frame, *expected)
        except AssertionError as error:
            raise AssertionError(f"Movement scenario failed: {name}, expected {expected}") from error
        if name == "swim one square":
            write_png(screenshot.with_name("fighter-swimming.png"), frame)

    back_to_browsing = render(core, rom, enter | {50: (RIGHT,), 60: (L, START)})
    assert back_to_browsing == render(core, rom, {30: (A,)})
    reentered = render(core, rom, enter | {50: (RIGHT,), 60: (L, START), 70: (START,)})
    check_fighter(reentered, 9, 4, "00")
    check_terrain(starting_view, reentered, (9, 4), (9, 5))
    check_animation(core, rom, enter)
    check_camera_animation(core, direct_rom)
    check_top_right(core, direct_rom, screenshot)
    check_bottom_edge(core, direct_rom)
    check_character_panel(core, direct_rom, screenshot)
    check_dash(core, direct_rom, screenshot)
    check_longbow(core, direct_rom, screenshot)
    check_attack_range(core, direct_rom, screenshot)
    check_repeated_weapon_swaps(core, direct_rom)
    check_new_breakables(core, direct_rom)
    check_equipment_menu(core, direct_rom, screenshot)
    check_turn_banner(core, direct_rom, screenshot)
    check_movement_menu(core, direct_rom, screenshot)
    check_fence(core, direct_rom, screenshot)
    check_other_breakables(core, direct_rom, screenshot)
    print(f"River Crossing OK. Idle, walking, swimming and camera animation verified. {len(scenarios)} movement scenarios, trial reset and direct boot verified.")


if __name__ == "__main__":
    if len(sys.argv) == 5 and sys.argv[1] == "--preview":
        core, rom, screenshot = map(Path, sys.argv[2:])
        check_equipment_menu(core, rom, screenshot)
        check_attack_range(core, rom, screenshot)
        check_repeated_weapon_swaps(core, rom)
        raise SystemExit(0)
    core, rom, direct_rom, screenshot = map(Path, sys.argv[1:])
    screenshot.unlink(missing_ok=True)
    start = render(core, rom)
    check_cursor(start, 3, 5)
    write_png(screenshot, start)

    # A short hold moves once. A longer hold starts auto-repeat.
    held = {frame: (RIGHT,) for frame in range(30, 45)}
    moved = render(core, rom, held | {52: (DOWN,)})
    check_cursor(moved, 4, 6)
    check_terrain(start, moved, (3, 5), (4, 6))

    repeated = render(core, rom, {frame: (RIGHT,) for frame in range(30, 55)})
    check_cursor(repeated, 6, 5)

    top_left = render(core, rom, {frame: (LEFT, UP) for frame in range(30, 60, 2)})
    check_cursor(top_left, 0, 0)
    check_terrain(start, top_left, (3, 5), (0, 0))

    # These two stops put the cursor at the same screen position, with camera
    # origins one square apart on both axes. Compare their overlapping terrain.
    diagonal = {frame: (RIGHT, DOWN) for frame in range(30, 74, 2)}
    scrolled = render(core, rom, diagonal)
    shifted = render(core, rom, diagonal | {74: (RIGHT, DOWN)})
    check_cursor(scrolled, 13, 8)
    check_cursor(shifted, 13, 8)
    for y in range(HEIGHT - 16):
        for x in range(WIDTH - 16):
            if (x // 16, y // 16) in ((12, 7), (13, 8)):
                continue
            old = 3 * ((y + 16) * WIDTH + x + 16)
            new = 3 * (y * WIDTH + x)
            assert scrolled[old:old + 3] == shifted[new:new + 3], "Camera did not scroll 16 pixels"
    assert scrolled != shifted, "Camera did not reveal new terrain"

    map_source = Path(__file__).resolve().parents[1] / "src/battlefield-maps.h"
    rows = re.findall(r'"([.=#~T,:+]{64})"', map_source.read_text())
    assert len(rows) == 3 * 64, "Expected three 64×64 terrain layouts"
    far = {frame: (RIGHT, DOWN) for frame in range(60, 200, 2)}
    river = render(core, rom, {30: (A,)})
    keep = render(core, rom, {30: (A,), 40: (A,)})
    for index, (name, frame, switch) in enumerate((
        ("battlefield", start, {}),
        ("river-crossing", river, {30: (A,)}),
        ("old-keep", keep, {30: (A,), 40: (A,)}),
    )):
        terrain = rows[index * 64:(index + 1) * 64]
        check_cursor(frame, 3, 5)
        check_view(frame, terrain, 0, 0)
        write_png(screenshot.with_name(f"{name}.png"), frame)
        bottom_right = render(core, rom, switch | far)
        check_cursor(bottom_right, 14, 9)
        check_view(bottom_right, terrain, 49, 54)
        write_png(screenshot.with_name(f"{name}-far.png"), bottom_right)

    reverse = {frame: (LEFT, UP) for frame in range(210, 350, 2)}
    returned = render(core, rom, far | reverse)
    check_cursor(returned, 0, 0)
    check_view(returned, rows[:64], 0, 0)
    assert returned == top_left, "Returning after cache wrap should restore the original view"

    assert len({start, river, keep}) == 3, "Expected three distinct battlefields"
    assert render(core, rom, {30: (A,), 40: (A,), 50: (A,)}) == start
    reset = render(core, rom, diagonal | {84: (A,)})
    assert reset == river, "Map switch should reset cursor and camera"

    check_movement(core, rom, direct_rom, screenshot)

    print(f"mGBA battlefield OK. Fighter movement and HUD verified. Cache wrap, held input and all three 64×64 maps verified. Screenshot at {screenshot}")
