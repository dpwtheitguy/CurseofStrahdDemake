"""Generate original 4bpp scenery for the GBA action close-up."""
import random
import re
from pathlib import Path


WIDTH, HEIGHT = 240, 112
PALETTES = {
    "field": ("162743", "635d85", "9290a5", "d1a593", "4b506f", "343d57",
              "182d37", "234439", "315743", "45694b", "577956", "22372f",
              "594832", "7c6242", "a17e51"),
    "road": ("162743", "655c76", "938598", "caa591", "515066", "3c4258",
             "243939", "3c5240", "4c6546", "59704a", "6e8053", "56402e",
             "745438", "977047", "bc925f"),
    "forest": ("162743", "3c5365", "5c777f", "80938a", "374c55", "293d49",
               "142823", "23372b", "2c4735", "3a5640", "50704e", "1d2424",
               "3f382e", "624c38", "987450"),
    "wall": ("162743", "41465d", "606883", "8c8fa4", "4b5063", "363b50",
             "242a3a", "373746", "454656", "595864", "797784", "242635",
             "48434d", "696271", "95919d"),
    "bridge": ("162743", "566783", "8198aa", "bec4ba", "40546b", "2f4056",
               "1b3543", "254b58", "326679", "488492", "74a2a4", "332b2b",
               "584435", "795a40", "ae8258"),
    "water": ("162743", "536e8d", "8496af", "b3bec2", "4b5f77", "36465c",
              "1b3847", "254c5f", "30667d", "488499", "82adae", "193342",
              "275064", "3b7188", "7eafb2"),
    "snow": ("162743", "53678d", "8495b0", "ccd1dc", "59738f", "425a78",
             "324862", "72869d", "9aafc4", "bccdd8", "d6e2e6", "566d88",
             "8ba6bd", "adbecd", "e1eaf0"),
    "stone": ("162743", "4b5674", "7386a0", "adaec0", "555971", "3d4255",
              "293342", "435062", "566173", "696e7a", "85878a", "333342",
              "545563", "777987", "999ba5"),
    "marsh": ("162743", "566780", "87999c", "afbea7", "4a6265", "334c51",
              "233a3c", "355050", "4a6260", "61766a", "819185", "303734",
              "484b3b", "66634b", "8b8460"),
}


def landscape(kind):
    rng = random.Random(73)
    pixels = [[1] * WIDTH for _ in range(HEIGHT)]

    def span(y, left, right, color):
        for x in range(max(0, left), min(right, WIDTH)):
            pixels[y][x] = color

    for y in range(38):
        span(y, 0, WIDTH, 1 if y < 12 else 2 if y < 27 else 3)
    for left, y, length in [(12, 11, 38), (63, 7, 44), (132, 16, 47), (196, 9, 32)]:
        for row in range(3):
            span(y + row, left + row * 4, left + length - row * 3, 3)
    for peak, top, width in [(18, 18, 33), (60, 25, 34), (109, 17, 43),
                             (151, 23, 32), (205, 17, 46), (239, 28, 23)]:
        for y in range(top, 41):
            radius = (y - top) * width // (41 - top)
            span(y, peak - radius, peak + radius + 1, 4)
            span(y, peak, peak + radius + 1, 5)
            if y < top + 7:
                span(y, peak - radius, peak + radius // 3 + 1, 3)
    for x in range(WIDTH):
        top = 39 - rng.randrange(5)
        for y in range(top, 44):
            pixels[y][x] = 6
    for y in range(44, HEIGHT):
        shade = 7 if y < 55 else 8 if y < 78 else 9
        span(y, 0, WIDTH, shade)
        for _ in range(8 + y // 4):
            x = rng.randrange(WIDTH)
            span(y, x, x + rng.randrange(1, 3 + y // 12), shade + (-1 if rng.randrange(3) else 1))

    if kind == "bridge":
        for y in range(47, HEIGHT):
            left = 65 - (y - 47) * 2
            right = 103 + (y - 47) * 5 // 2
            span(y, left, right, 12 if (y - 47) % (5 if y < 75 else 8) else 11)
            if y % 4 == 0:
                for _ in range(10):
                    x = rng.randrange(max(0, left), right)
                    span(y, x, min(right, x + 6), 13)
            span(y, left - 2, left + 1, 14)
            span(y, right - 1, right + 2, 14)
    elif kind in {"road", "stone"}:
        for y in range(44, HEIGHT):
            depth = y - 44
            center = 57 + depth
            width = 22 + depth * 2
            left, right = max(0, center - width), min(WIDTH, center + width)
            span(y, left, right, 12)
            if kind == "stone":
                size = 4 if depth < 20 else 7 if depth < 40 else 10
                for x in range(left, right):
                    joint = (x + (depth // size % 2) * size) % (size * 2)
                    if depth % size == 0 or joint == 0:
                        pixels[y][x] = 11
                    else:
                        pixels[y][x] = 14 if depth % size == 1 else 13
            else:
                for _ in range(14):
                    x = rng.randrange(left, right)
                    span(y, x, min(right, x + 1 + depth // 18), rng.choice((11, 13, 14)))
    elif kind == "wall":
        # A broken rampart behind both actors, with staggered courses and a stone footing.
        for x in range(WIDTH):
            top = 23 + (x // 26 % 2) * 8 + (8 if 102 <= x < 142 else 0)
            for y in range(top, 57):
                joint = (x + (y // 7 % 2) * 9) % 18
                pixels[y][x] = 11 if y % 7 == 0 or joint == 0 else 14 if y % 7 == 1 else 13
        for y in range(57, HEIGHT):
            for _ in range(9):
                x = rng.randrange(WIDTH)
                span(y, x, x + rng.randrange(2, 8), rng.choice((11, 12, 13, 14)))
    elif kind == "forest":
        for center, top, height, width in [(8, 1, 58, 27), (39, 8, 44, 19),
                                            (213, 7, 49, 22), (237, 0, 63, 27),
                                            (111, 19, 28, 13), (163, 16, 33, 16)]:
            for y in range(top, top + height):
                if y > top + height * 4 // 5:
                    span(y, center - 2, center + 3, 12)
                else:
                    spread = (y - top) % (height // 4) + (y - top) * width // height
                    span(y, center - spread, center + spread + 1, 6)
                    span(y, center - spread + 2, center, 8)
        for x in range(2, WIDTH, 5):
            y = 88 + rng.randrange(20)
            span(y, x, x + 4, 11)
            span(y - 1, x + 1, x + 3, 10)
    elif kind in {"marsh", "water"}:
        for y in range(49, HEIGHT):
            for _ in range(10):
                x = rng.randrange(WIDTH)
                span(y, x, x + rng.randrange(3, 15), rng.choice((7, 10, 14)))
        if kind == "marsh":
            for x, y in [(14, 79), (39, 100), (103, 66), (129, 94), (208, 81), (230, 109)]:
                for stem in range(5):
                    height = rng.randrange(5, 16)
                    for dy in range(height):
                        pos = x + stem * 2 + (stem - 2) * dy // 7
                        span(y - dy, pos, pos + 1, 11 if dy < height - 3 else 15)
    if kind in {"field", "forest", "wall", "marsh", "snow"}:
        # Perspective footing for the near actor and a smaller far landing.
        for cx, cy, rx, ry in [(66, 65, 26, 6), (174, 99, 54, 11)]:
            for y in range(cy - ry, cy + ry + 1):
                for x in range(cx - rx, cx + rx):
                    if (x - cx)**2 * ry**2 + (y - cy)**2 * rx**2 < rx**2 * ry**2:
                        pixels[y][x] = rng.choice((12, 13, 13, 13, 14))
    if kind == "snow":
        for _ in range(95):
            y = rng.randrange(4, HEIGHT)
            x = rng.randrange(WIDTH)
            pixels[y][x] = 15
    return pixels


def fighter_pose(pose, weapon, armor, shield):
    pixels = [[0] * 32 for _ in range(32)]

    def polygon(color, *points):
        for y in range(32):
            for x in range(32):
                inside = False
                for (ax, ay), (bx, by) in zip(points, points[1:] + points[:1]):
                    if (ay > y) != (by > y) and x < ax + (bx - ax) * (y - ay) / (by - ay):
                        inside = not inside
                if inside:
                    pixels[y][x] = color

    lean = (-1, 1, -3)[pose]
    # Near-facing cape folds, boots, helmet rim and the shield's metal boss.
    polygon(1, (11, 11), (22, 9), (27, 20), (31, 24), (25, 27), (21, 25),
            (21, 30), (15, 30), (15, 24), (12, 29), (6, 29), (9, 24))
    polygon(5, (14, 11), (21, 11), (25, 20), (29, 24), (24, 25), (19, 22), (9, 24))
    polygon(6, (18, 12), (21, 13), (23, 21), (26, 24), (22, 22), (19, 19))
    polygon(9, (13, 23), (15, 23), (12, 27), (8, 28), (10, 25))
    polygon(9, (16, 22), (19, 23), (19, 27), (20, 29), (16, 29))
    if armor:
        polygon(1, (10 + lean, 4), (14 + lean, 1), (20 + lean, 2),
                (24 + lean, 6), (23 + lean, 11), (19 + lean, 13),
                (12 + lean, 12), (9 + lean, 8))
        polygon(2, (10 + lean, 5), (14 + lean, 2), (19 + lean, 3),
                (22 + lean, 6), (22 + lean, 9), (12 + lean, 9))
        polygon(3, (11 + lean, 5), (15 + lean, 3), (19 + lean, 4), (16 + lean, 5), (13 + lean, 7))
        polygon(4, (11 + lean, 9), (16 + lean, 10), (18 + lean, 12), (13 + lean, 11))
    else:
        polygon(1, (10 + lean, 6), (13 + lean, 3), (20 + lean, 3),
                (23 + lean, 7), (20 + lean, 13), (13 + lean, 13))
        polygon(4, (11 + lean, 8), (17 + lean, 6), (21 + lean, 8), (19 + lean, 12),
                (13 + lean, 12))
        polygon(7, (10 + lean, 7), (13 + lean, 4), (20 + lean, 4),
                (22 + lean, 7), (16 + lean, 6), (12 + lean, 9))
    if weapon == 2:
        polygon(1, (4, 3), (8, 7), (9, 13), (8, 21), (4, 26),
                (2, 25), (5, 20), (6, 13), (5, 8), (2, 4))
        polygon(7, (3, 4), (7, 9), (8, 14), (6, 22), (3, 25),
                (6, 19), (7, 13), (6, 8))
        draw = 12 if pose == 1 else 4
        for y in range(5, 25):
            pixels[y][3 + (draw - 3) * (10 - abs(y - 15)) // 10] = 3
        polygon(4, (6, 14), (13, 14), (15, 16), (10, 17), (6, 16))
    elif pose == 2:
        polygon(1, (13, 11), (15, 15), (7, 17), (1, 15), (0, 11), (3, 9), (7, 13))
        polygon(3 if weapon else 4, (1, 11), (3, 10), (6, 14), (4, 15), (2, 14))
        polygon(4, (6, 14), (10, 13), (14, 13), (8, 16))
    elif weapon:
        polygon(1, (16, 17), (19, 15), (26, 9), (25, 6), (30, 4), (32, 8), (29, 13), (20, 20))
        polygon(2, (26, 7), (29, 5), (31, 8), (28, 11))
        polygon(3, (27, 7), (29, 6), (30, 8), (28, 9))
        polygon(7, (20, 16), (28, 10), (27, 13), (21, 19))
    if shield:
        polygon(1, (5, 13), (10, 11), (15, 13), (16, 18), (13, 22), (8, 23), (4, 20), (3, 16))
        polygon(7, (5, 14), (10, 12), (14, 14), (15, 18), (12, 21), (8, 22), (5, 19))
        polygon(8, (7, 15), (10, 14), (13, 16), (12, 20), (8, 21), (6, 18))
        polygon(3, (9, 16), (11, 16), (12, 18), (10, 19), (8, 18))
    else:
        polygon(1, (9, 12), (14, 14), (13, 19), (8, 22), (5, 20), (6, 16))
        polygon(4, (9, 14), (12, 15), (11, 19), (8, 21), (6, 19))
    return pixels


def tile_words(pixels, left, top):
    return [sum(pixels[y][x + i] << (i * 4) for i in range(4))
            for y in range(top, top + 8) for x in (left, left + 4)]


def emit_tile(pixels, left, top):
    print("            {" + ", ".join(f"0x{n:04x}" for n in tile_words(pixels, left, top)) + "},")


def validate_map_coverage():
    source = Path(__file__).resolve().parents[1] / "src"
    definitions = re.findall(r"^TERRAIN\('(.)', [^,]+, (\w+), \d+\)$",
                             (source / "battlefield-terrain.def").read_text(), re.MULTILINE)
    by_symbol = dict(definitions)
    if len(by_symbol) != len(definitions):
        raise ValueError("Duplicate terrain symbol")
    rows = re.findall(r'^\s*"([^"]+)"', (source / "battlefield-maps.h").read_text(), re.MULTILINE)
    if not rows:
        raise ValueError("No battlefield map rows found")
    missing = set("".join(rows)) - by_symbol.keys()
    if missing:
        raise ValueError(f"Map symbols missing terrain and backdrop entries: {sorted(missing)}")
    missing_art = set(by_symbol.values()) - PALETTES.keys()
    if missing_art:
        raise ValueError(f"Terrain backdrops missing art: {sorted(missing_art)}")
    if len(set(by_symbol.values())) != len(by_symbol):
        raise ValueError("Each terrain needs its own battle backdrop")



def emit_command_ui():
    source = Path(__file__).resolve().parents[1] / "src/battlefield-fighter.h"
    glyphs = {letter: [int(n) for n in values.split(",")]
              for values, letter in re.findall(r"\{([\d, ]+)\},\s*/\* (.) \*/", source.read_text())}
    pixels = [[0] * 128 for _ in range(32)]
    for y in range(3, 29):
        for x in range(128):
            edge = min(x, 127 - x)
            vertical = min(y - 3, 28 - y)
            if edge + vertical < 5:
                continue
            pixels[y][x] = (1 if edge == 0 or vertical == 0 else
                            2 if edge <= 2 or vertical <= 2 else
                            3 if edge == 3 or vertical == 3 else
                            4 if y < 15 else 5)
    for index, letter in enumerate("NEW TURN"):
        if letter == " ":
            continue
        for y, bits in enumerate(glyphs[letter]):
            for x in range(5):
                if bits & (16 >> x):
                    for dy in range(2):
                        for dx in range(2):
                            px, py = 17 + 12 * index + 2 * x + dx, 9 + 2 * y + dy
                            pixels[py + 2][px + 1] = 1
                            pixels[py][px] = 6 if y < 3 else 7
    print("static const uint16_t turn_banner_tiles[64][16] = {")
    for half in (0, 64):
        for top in range(0, 32, 8):
            for left in range(half, half + 64, 8):
                emit_tile(pixels, left, top)
    print("};")

    card = [[0] * 32 for _ in range(32)]
    for y in range(32):
        for x in range(32):
            edge = min(x, 31 - x)
            vertical = min(y, 31 - y)
            if edge + vertical >= 4:
                card[y][x] = 1 if min(edge, vertical) == 0 or y >= 22 else 4 if min(edge, vertical) == 1 else 3
            if 8 <= x < 24 and 6 <= y < 19:
                card[y][x] = 6 if x in (8, 23) or y in (6, 18) else 5 if y < 11 else 2
            if 14 <= x < 18 and 11 <= y < 15:
                card[y][x] = 7
    for index, letter in enumerate("ITEMS"):
        for y, bits in enumerate(glyphs[letter]):
            for x in range(5):
                if bits & (16 >> x):
                    card[23 + y][2 + 6 * index + x] = 7
    print("static const uint16_t items_card_tiles[16][16] = {")
    for top in range(0, 32, 8):
        for left in range(0, 32, 8):
            emit_tile(card, left, top)
    print("};")


def main():
    validate_map_coverage()
    print("/* Generated by scripts/generate-closeup.py. */")
    print("enum { " + ", ".join(f"SCENE_{kind}" for kind in PALETTES) + " };")
    print("static const struct { uint16_t palette[16]; uint16_t tiles[420][16]; } closeup_art[] = {")
    for kind, palette in PALETTES.items():
        pixels = landscape(kind)
        print(f"    {{ /* {kind} */")
        colors = [tuple(int(rgb[i:i + 2], 16) >> 3 for i in (0, 2, 4)) for rgb in palette]
        print("        {0, " + ", ".join(f"RGB5({r}, {g}, {b})" for r, g, b in colors) + "}, {")
        for top in range(0, HEIGHT, 8):
            for left in range(0, WIDTH, 8):
                emit_tile(pixels, left, top)
        print("        }},")
    print("};")
    print("static const uint16_t closeup_fighter_tiles[3][2][2][3][16][16] = {")
    for weapon in range(3):
        print("    {")
        for armor in (False, True):
            print("    {")
            for shield in (False, True):
                print("    {")
                for pose in range(3):
                    print("    {")
                    pixels = fighter_pose(pose, weapon, armor, shield)
                    for top in range(0, 32, 8):
                        for left in range(0, 32, 8):
                            emit_tile(pixels, left, top)
                    print("    },")
                print("    },")
            print("    },")
        print("    },")
    print("};")

    emit_command_ui()


if __name__ == "__main__":
    main()
