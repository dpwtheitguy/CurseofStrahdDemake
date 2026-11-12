#include <stdint.h>

enum {
    MAP_WIDTH = 64,
    MAP_HEIGHT = 64,
    CACHE_SIZE = 32,
    VIEW_WIDTH = 15,
    VIEW_HEIGHT = 10,
    CELL_SIZE = 16,
    TILE_SIZE = 8,
    MAP_STRIDE = 32,
    SCREEN_BLOCK = 16,
    VISIBLE_LINES = 160,
    BG_SIZE_512 = 3 << 14,
    KEY_A = 1,
    KEY_B = 1 << 1,
    KEY_SELECT = 1 << 2,
    KEY_START = 1 << 3,
    BG1_ENABLE = 1 << 9,
    HUD_SCREEN_BLOCK = 20,
    SCENE_BLOCK = 21,
    SCENE_TILE = 160,
    SCENE_PALETTE = 5,
    SPARK_TILE = 40,
    SCENE_IMPACT = 24,
    SCENE_RESULT = 34,
    HUD_TILE_BASE = 32,
    RIVER_MAP = 1,
    TURN_FEET = 30,
    WALK_POSE_FRAMES = 8,
    SWIM_POSE_FRAMES = 8,
    STANDING_TILE = 4,
    SWIM_GLIDE_TILE = 8,
    WALKING_TILE = 12,
    SWIM_STROKE_TILE = 16,
    FENCE_TILE = 20,
    BROKEN_FENCE_TILE = 24,
    CRATE_TILE = 28,
    BARREL_TILE = 32,
    IDLE_BREATH_TILE = 36,
    IDLE_POSE_FRAMES = 32,
    PIXELS_PER_FRAME = 4,
    TARGET_OAM = 16,
    ATHLETICS_BONUS = 4,
    CARD_TILES = 4,
    CARD_SIZE = CARD_TILES * TILE_SIZE,
    MENU_TILE_BASE = 80,
    TARGET_CARD_X = 13,
    TARGET_CARD_Y = 11,
    CHOICE_CARD_Y = 14,
    SMASH_CARD_X = 9,
    BACK_CARD_X = 17,
    MENU_PALETTE = 3,
    ACTIVE_PALETTE = 4,
    REPEAT_DELAY = 18,
    REPEAT_PERIOD = 5,
    FORCE_BLANK = 1 << 7,
    BG0_ENABLE = 1 << 8,
    OBJ_ENABLE = 1 << 12,
    OBJ_1D = 1 << 6,
    OBJ_SIZE_16 = 1 << 14,
    OBJ_DISABLED = 1 << 9,
    KEY_RIGHT = 1 << 4,
    KEY_LEFT = 1 << 5,
    KEY_UP = 1 << 6,
    KEY_DOWN = 1 << 7,
    KEY_DPAD = KEY_RIGHT | KEY_LEFT | KEY_UP | KEY_DOWN
};

#define REG16(address) (*(volatile uint16_t *)(address))
#define BG_PALETTE ((volatile uint16_t *)0x05000000)
#define OBJ_PALETTE ((volatile uint16_t *)0x05000200)
#define BG_TILES ((volatile uint16_t *)0x06000000)
#define BG_MAP ((volatile uint16_t *)(0x06000000 + SCREEN_BLOCK * 2048))
#define HUD_MAP ((volatile uint16_t *)(0x06000000 + HUD_SCREEN_BLOCK * 2048))
#define SCENE_MAP ((volatile uint16_t *)(0x06000000 + SCENE_BLOCK * 2048))
#define OBJ_TILES ((volatile uint16_t *)0x06010000)
#define OAM ((volatile uint16_t *)0x07000000)
#define RGB5(r, g, b) ((r) | ((g) << 5) | ((b) << 10))

#include "battlefield-maps.h"
#include "battlefield-fighter.h"

typedef enum {
    FENCE_MENU_CLOSED,
    FENCE_MENU_SMASH,
    FENCE_MENU_CANCEL
} FenceMenu;

typedef struct {
    int x, y;
    unsigned tile, dc;
} Breakable;

static const Breakable breakables[] = {
    {9, 3, FENCE_TILE, 12},
    {9, 6, CRATE_TILE, 10},
    {11, 5, BARREL_TILE, 14},
};

enum { BREAKABLE_COUNT = sizeof breakables / sizeof breakables[0] };

enum { MAP_COUNT = sizeof maps / sizeof maps[0] };

static const uint16_t ground_colors[] = {
    0,
    RGB5(8, 13, 8), RGB5(6, 10, 7), RGB5(11, 16, 9),
    RGB5(17, 13, 8), RGB5(14, 10, 7), RGB5(20, 16, 11),
    RGB5(7, 8, 10), RGB5(16, 17, 18), RGB5(20, 21, 21),
    RGB5(11, 12, 14), RGB5(24, 24, 22),
    RGB5(4, 10, 15), RGB5(5, 12, 18), RGB5(8, 16, 21), 0,
    /* Bank 1. Four terrains share these twelve visible colors. */
    0,
    RGB5(3, 8, 6), RGB5(5, 12, 7), RGB5(10, 17, 8),
    RGB5(8, 11, 9), RGB5(12, 14, 8), RGB5(7, 13, 12),
    RGB5(10, 10, 12), RGB5(15, 15, 17), RGB5(19, 18, 20),
    RGB5(11, 7, 5), RGB5(21, 14, 8), RGB5(25, 18, 11)
};

typedef unsigned (*PixelColor)(unsigned x, unsigned y);

static unsigned fleck(unsigned x, unsigned y)
{
    unsigned n = x * 151u + y * 313u;
    return ((n ^ (n >> 3)) * 79u) & 31u;
}

static unsigned grass(unsigned x, unsigned y)
{
    unsigned grain = fleck(x, y);
    if (grain == 0 || ((x + 5 * y) & 63u) == 19)
        return 3;
    return grain < 6 ? 2 : 1;
}

static unsigned road(unsigned x, unsigned y)
{
    unsigned grain = fleck(x + 9, y + 5);
    if (grain < 3)
        return 6;
    return grain < 8 ? 5 : 4;
}

static unsigned wall(unsigned x, unsigned y)
{
    if (y >= 14)
        return 7;
    if (y >= 10)
        return (y == 12 || ((x + 4) & 7u) == 0) ? 7 : 10;
    if (y == 0)
        return 11;
    if (y == 5 || ((x + (y < 5 ? 0 : 4)) & 7u) == 0)
        return 7;
    return (y == 1 || y == 6 || fleck(x, y) < 4) ? 9 : 8;
}

static unsigned water(unsigned x, unsigned y)
{
    unsigned ripple = (x + 3 * y) & 15u;
    if ((y & 3u) == 1 && ripple < 5)
        return 14;
    return ripple < 8 ? 12 : 13;
}

static unsigned forest(unsigned x, unsigned y)
{
    unsigned distance = x < 8 ? 7 - x : x - 8;
    unsigned width = y < 8 ? y / 2 : (y - 4) / 2;
    if (y > 13 || distance > width)
        return 1;
    return (distance + 1 >= width || fleck(x, y) < 7) ? 3 : 2;
}

static unsigned marsh(unsigned x, unsigned y)
{
    if (((x + 2 * y) & 7u) < 2 && (y & 3u) < 3)
        return 5;
    return fleck(x, y) < 13 ? 6 : 4;
}

static unsigned cobble(unsigned x, unsigned y)
{
    unsigned seam = (x + (y / 4 % 2) * 4) & 7u;
    if (y % 4 == 3 || seam == 0)
        return 7;
    return (y % 4 == 0 || fleck(x, y) < 5) ? 9 : 8;
}

static unsigned deck(unsigned x, unsigned y)
{
    if (y % 4 == 3 || (x + 4 * (y / 4 % 2)) % 8 == 0)
        return 10;
    return (y % 4 == 0 || fleck(x, y) < 4) ? 12 : 11;
}

static unsigned cursor(unsigned x, unsigned y)
{
    unsigned edge_x = x < 8 ? x : 15 - x;
    unsigned edge_y = y < 8 ? y : 15 - y;
    if ((edge_x == 0 && edge_y < 6) || (edge_y == 0 && edge_x < 6))
        return 1;
    if ((edge_x == 1 && edge_y < 5) || (edge_y == 1 && edge_x < 5))
        return 2;
    return 0;
}

/* Pack a 16×16 cell into four 4bpp hardware tiles in row-major order. */
static void write_cell(volatile uint16_t *destination, PixelColor pixel)
{
    for (unsigned tile_y = 0; tile_y < 2; ++tile_y) {
        for (unsigned tile_x = 0; tile_x < 2; ++tile_x) {
            for (unsigned row = 0; row < TILE_SIZE; ++row) {
                for (unsigned col = 0; col < TILE_SIZE; col += 4) {
                    uint16_t packed = 0;
                    for (unsigned i = 0; i < 4; ++i) {
                        packed |= pixel(tile_x * TILE_SIZE + col + i,
                                        tile_y * TILE_SIZE + row) << (4 * i);
                    }
                    *destination++ = packed;
                }
            }
        }
    }
}

static unsigned fighter_color(char pixel)
{
    if (pixel == '.')
        return 0;
    if (pixel >= 'a')
        return 10 + pixel - 'a';
    return pixel - '0';
}

static unsigned fighter(unsigned x, unsigned y)
{
    return fighter_color(fighter_pixels[y][x]);
}

static unsigned breathing_fighter(unsigned x, unsigned y)
{
    if (y == 0)
        return 0;
    /* Lower the helmet and shift a shoulder highlight while the boots stay planted. */
    if (y <= 4)
        return fighter_color(fighter_pixels[y - 1][x]);
    if (y == 7 && x == 8)
        return 6;
    return fighter_color(fighter_pixels[y][x]);
}

static unsigned swimming_fighter(unsigned x, unsigned y)
{
    return fighter_color(swimming_pixels[y][x]);
}

static unsigned walking_fighter(unsigned x, unsigned y)
{
    return fighter_color(walking_pixels[y][x]);
}

static unsigned stroking_fighter(unsigned x, unsigned y)
{
    return fighter_color(swim_stroke_pixels[y][x]);
}

static unsigned fence(unsigned x, unsigned y)
{
    if (x >= 2 && x <= 13 && ((y >= 5 && y <= 7) || (y >= 10 && y <= 12)))
        return y == 5 || y == 10 ? 3 : 2;
    if (((x >= 1 && x <= 3) || (x >= 12 && x <= 14)) && y >= 3 && y <= 14)
        return x == 2 || x == 13 ? 3 : 1;
    return 0;
}

static unsigned broken_fence(unsigned x, unsigned y)
{
    if ((y == 10 && x >= 7 && x <= 13) ||
        (y == 13 && x >= 3 && x <= 9) || (y == 11 && x >= 9 && x <= 12))
        return 3;
    if ((x == 2 || x == 13) && y >= 12 && y <= 14)
        return 2;
    return 0;
}

static unsigned crate(unsigned x, unsigned y)
{
    if (x < 2 || x > 13 || y < 4 || y > 14)
        return 0;
    if (x == 2 || x == 13 || y == 14)
        return 1;
    return y == 4 || y == 9 || x == y || x + y == 16 ? 3 : 2;
}

static unsigned barrel(unsigned x, unsigned y)
{
    unsigned inset = y < 5 || y >= 13 ? 4 : 2;
    if (y < 3 || y > 14 || x < inset || x >= 16 - inset)
        return 0;
    if (y == 7 || y == 12 || y == 14 || x == inset || x == 15 - inset)
        return 1;
    return y <= 4 || x == 6 || x == 9 ? 3 : 2;
}

static void load_hud_font(void)
{
    BG_PALETTE[33] = RGB5(4, 5, 8);
    BG_PALETTE[34] = RGB5(31, 29, 22);


    volatile uint16_t *tile = BG_TILES + HUD_TILE_BASE * 16;
    for (unsigned glyph = 0; glyph < sizeof hud_glyphs / sizeof hud_glyphs[0]; ++glyph) {
        for (unsigned row = 0; row < 8; ++row) {
            unsigned ink = row < 7 ? hud_glyphs[glyph][row] : 0;
            uint32_t packed = 0x11111111;
            for (unsigned col = 0; col < 5; ++col) {
                if (ink & (1u << (4 - col)))
                    packed ^= 3u << (4 * (col + 1));
            }
            *tile++ = packed;
            *tile++ = packed >> 16;
        }
    }

    /* BG1 is transparent everywhere outside the HUD. */
    unsigned blank = HUD_TILE_BASE + sizeof hud_glyphs / sizeof hud_glyphs[0];
    for (unsigned halfword = 0; halfword < 16; ++halfword)
        *tile++ = 0;
    for (unsigned entry = 0; entry < MAP_STRIDE * MAP_STRIDE; ++entry)
        HUD_MAP[entry] = blank;
}

static unsigned hud_glyph(char letter)
{
    for (unsigned glyph = 0; hud_letters[glyph]; ++glyph)
        if (hud_letters[glyph] == letter)
            return glyph;
    return 0;
}

static void hud_letter(unsigned entry, char letter)
{
    HUD_MAP[entry] = (2 << 12) | (HUD_TILE_BASE + hud_glyph(letter));
}

static unsigned hammer_icon(unsigned x, unsigned y)
{
    if (y >= 2 && y <= 7 && x >= 2 && x <= 13) {
        if (x == 2 || x == 13 || y == 7)
            return 1;
        return y == 2 || x == 3 ? 3 : 2;
    }
    if (y >= 8 && y <= 14 && x >= 7 && x <= 9)
        return x == 7 ? 3 : x == 9 ? 1 : 2;
    return 0;
}

static unsigned back_icon(unsigned x, unsigned y)
{
    unsigned height = y < 8 ? 8 - y : y - 8;
    if (x >= 2 && x < 8 && height <= x - 2)
        return height == x - 2 ? 3 : 2;
    if ((y >= 7 && y <= 9 && x >= 7 && x <= 12) ||
        (x >= 11 && x <= 13 && y >= 8 && y <= 13))
        return y == 7 || x == 11 ? 3 : 2;
    return 0;
}

static unsigned card_pixel(unsigned x, unsigned y, PixelColor icon, const char *caption,
                           unsigned text_width)
{
    unsigned edge_x = x < CARD_SIZE / 2 ? x : CARD_SIZE - 1 - x;
    unsigned edge_y = y < CARD_SIZE / 2 ? y : CARD_SIZE - 1 - y;
    if (edge_x + edge_y < 3)
        return 0;
    if (edge_x == 0 || edge_y == 0)
        return 1;
    if (edge_x == 1 || edge_y == 1)
        return x < 2 || y < 2 ? 4 : 2;
    if (edge_x == 2 || edge_y == 2)
        return 2;

    if (x >= 8 && x < 24 && y >= 4 && y < 20) {
        unsigned color = icon(x - 8, y - 4);
        if (color)
            return 4 + color;
    }
    if (y == 21 && x >= 5 && x < CARD_SIZE - 5)
        return 2;

    unsigned left = (CARD_SIZE - text_width) / 2;
    if (y >= 22 && y < 29 && x >= left && x < left + text_width) {
        unsigned column = x - left;
        unsigned glyph_column = column;
        while (glyph_column >= 6) {
            ++caption;
            glyph_column -= 6;
        }
        if (glyph_column < 5 &&
            (hud_glyphs[hud_glyph(*caption)][y - 22] & (16 >> glyph_column)))
            return 7;
    }
    return y >= 22 ? 1 : 3;
}

static void write_card(unsigned first_tile, PixelColor icon, const char *caption)
{
    unsigned text_width = 0;
    for (const char *letter = caption; *letter; ++letter)
        text_width += 6;
    --text_width;  /* The last glyph has no trailing gap. */
    volatile uint16_t *out = BG_TILES + first_tile * 16;
    for (unsigned tile_y = 0; tile_y < CARD_TILES; ++tile_y) {
        for (unsigned tile_x = 0; tile_x < CARD_TILES; ++tile_x) {
            for (unsigned row = 0; row < TILE_SIZE; ++row) {
                for (unsigned col = 0; col < TILE_SIZE; col += 4) {
                    unsigned packed = 0;
                    for (unsigned i = 0; i < 4; ++i)
                        packed |= card_pixel(tile_x * TILE_SIZE + col + i,
                                             tile_y * TILE_SIZE + row,
                                             icon, caption, text_width) << (4 * i);
                    *out++ = packed;
                }
            }
        }
    }
}

static void load_menu_cards(void)
{
    static const uint16_t bronze[] = {
        RGB5(8, 4, 3), RGB5(16, 9, 4), RGB5(12, 6, 3), RGB5(24, 15, 6),
        RGB5(16, 10, 5), RGB5(22, 14, 7), RGB5(31, 24, 13)
    };
    static const uint16_t gold[] = {
        RGB5(12, 4, 3), RGB5(24, 13, 4), RGB5(18, 8, 3), RGB5(31, 25, 11),
        RGB5(20, 12, 5), RGB5(28, 19, 8), RGB5(31, 28, 17)
    };
    for (unsigned i = 0; i < sizeof bronze / sizeof bronze[0]; ++i) {
        BG_PALETTE[16 * MENU_PALETTE + 1 + i] = bronze[i];
        BG_PALETTE[16 * ACTIVE_PALETTE + 1 + i] = gold[i];
    }
    write_card(MENU_TILE_BASE, fence, "DC 12");
    write_card(MENU_TILE_BASE + 16, crate, "DC 10");
    write_card(MENU_TILE_BASE + 32, barrel, "DC 14");
    write_card(MENU_TILE_BASE + 48, hammer_icon, "SMASH");
    write_card(MENU_TILE_BASE + 64, back_icon, "BACK");
}

static void place_card(unsigned x, unsigned y, unsigned tile, unsigned palette)
{
    for (unsigned row = 0; row < CARD_TILES; ++row)
        for (unsigned col = 0; col < CARD_TILES; ++col)
            HUD_MAP[(y + row) * MAP_STRIDE + x + col] =
                (palette << 12) | (tile + CARD_TILES * row + col);
}

static void draw_breakable_menu(FenceMenu menu, unsigned target)
{
    place_card(TARGET_CARD_X, TARGET_CARD_Y, MENU_TILE_BASE + 16 * target, MENU_PALETTE);
    place_card(SMASH_CARD_X, CHOICE_CARD_Y, MENU_TILE_BASE + 48,
               menu == FENCE_MENU_SMASH ? ACTIVE_PALETTE : MENU_PALETTE);
    place_card(BACK_CARD_X, CHOICE_CARD_Y, MENU_TILE_BASE + 64,
               menu == FENCE_MENU_CANCEL ? ACTIVE_PALETTE : MENU_PALETTE);
}

static void show_target(const Breakable *target, int camera_x, int camera_y)
{
    OAM[TARGET_OAM] = (target->y * CELL_SIZE - camera_y) & 255;
    OAM[TARGET_OAM + 1] = OBJ_SIZE_16 | ((target->x * CELL_SIZE - camera_x) & 511);
    OAM[TARGET_OAM + 2] = 0;  /* Reuse the gold bracket sprite. */
}

static void clear_card(unsigned x, unsigned y)
{
    unsigned blank = HUD_TILE_BASE + sizeof hud_glyphs / sizeof hud_glyphs[0];
    for (unsigned row = 0; row < CARD_TILES; ++row)
        for (unsigned col = 0; col < CARD_TILES; ++col)
            HUD_MAP[(y + row) * MAP_STRIDE + x + col] = blank;
}

static void clear_fence_menu(void)
{
    clear_card(TARGET_CARD_X, TARGET_CARD_Y);
    clear_card(SMASH_CARD_X, CHOICE_CARD_Y);
    clear_card(BACK_CARD_X, CHOICE_CARD_Y);
    OAM[TARGET_OAM] = OBJ_DISABLED;
}

static void hide_status(void)
{
    unsigned blank = HUD_TILE_BASE + sizeof hud_glyphs / sizeof hud_glyphs[0];
    for (unsigned x = 0; x < VIEW_WIDTH * 2; ++x)
        HUD_MAP[MAP_STRIDE + x] = blank;
}

static void hud_status(const char *text)
{
    for (unsigned x = 0; x < VIEW_WIDTH * 2; ++x)
        hud_letter(MAP_STRIDE + x, *text ? *text++ : ' ');
}

static void hud_number(unsigned entry, unsigned number)
{
    unsigned tens = 0;
    while (number >= 10) {
        ++tens;
        number -= 10;
    }
    hud_letter(entry, '0' + tens);
    hud_letter(entry + 1, '0' + number);
}

static void draw_hud(unsigned spent, unsigned swim_speed)
{
    static const char text[] = " MOVE 00/30 FT     SWIM OFF    ";
    unsigned tens = 0;
    while (spent >= 10) {
        ++tens;
        spent -= 10;
    }
    for (unsigned x = 0; x < VIEW_WIDTH * 2; ++x) {
        char letter = text[x];
        if (x == 6)
            letter = '0' + tens;
        if (x == 7)
            letter = '0' + spent;
        if (swim_speed && x == 25)
            letter = 'N';
        if (swim_speed && x == 26)
            letter = ' ';
        hud_letter(x, letter);
    }
}

static void load_tiles(void)
{
    for (unsigned i = 0; i < sizeof ground_colors / sizeof ground_colors[0]; ++i)
        BG_PALETTE[i] = ground_colors[i];

    write_cell(BG_TILES, grass);
    write_cell(BG_TILES + 64, road);
    write_cell(BG_TILES + 128, wall);

    write_cell(BG_TILES + 192, water);
    write_cell(BG_TILES + 256, forest);
    write_cell(BG_TILES + 320, marsh);
    write_cell(BG_TILES + 384, cobble);
    write_cell(BG_TILES + 448, deck);
}

static unsigned terrain_tile(char cell)
{
    switch (cell) {
    case '=': return 4;
    case '#': return 8;
    case '~': return 12;
    case 'T': return (1 << 12) | 16;
    case ',': return (1 << 12) | 20;
    case ':': return (1 << 12) | 24;
    case '+': return (1 << 12) | 28;
    default:  return 0;
    }
}

static void cache_square(unsigned map, unsigned x, unsigned y)
{
    unsigned tile = terrain_tile(maps[map][y][x]);

    /* Wrap world coordinates into the 32×32-square BG cache. Each
       quadrant of the cache is one 32×32-hardware-tile screen block. */
    unsigned cache_x = x % CACHE_SIZE;
    unsigned cache_y = y % CACHE_SIZE;
    unsigned block = (cache_y / 16) * 2 + cache_x / 16;
    unsigned entry = block * 1024
        + (2 * cache_y % 32) * MAP_STRIDE + 2 * cache_x % 32;
    BG_MAP[entry] = tile;
    BG_MAP[entry + 1] = tile + 1;
    BG_MAP[entry + MAP_STRIDE] = tile + 2;
    BG_MAP[entry + MAP_STRIDE + 1] = tile + 3;
}

static void load_starting_view(unsigned map)
{
    for (unsigned y = 0; y < VIEW_HEIGHT; ++y)
        for (unsigned x = 0; x < VIEW_WIDTH; ++x)
            cache_square(map, x, y);
}

static void stream_edges(unsigned map, int old_x, int old_y, int x, int y)
{
    if (x != old_x) {
        int column = x > old_x ? x + VIEW_WIDTH - 1 : x;
        for (int row = y; row < y + VIEW_HEIGHT; ++row)
            cache_square(map, column, row);
    }
    if (y != old_y) {
        int row = y > old_y ? y + VIEW_HEIGHT - 1 : y;
        for (int column = x; column < x + VIEW_WIDTH; ++column)
            cache_square(map, column, row);
    }
}

static void place_piece(int x, int y)
{
    OAM[0] = y & 255;
    OAM[1] = OBJ_SIZE_16 | (x & 511);
}

static void wait_vblank(void)
{
    while (REG16(0x04000006) >= VISIBLE_LINES) {}
    while (REG16(0x04000006) < VISIBLE_LINES) {}
}

static int clamp(int coordinate, int limit)
{
    if (coordinate < 0)
        return 0;
    if (coordinate >= limit)
        return limit - 1;
    return coordinate;
}

/* Keep the cursor inside a small margin before scrolling a whole square. */
static int follow(int cursor_position, int camera, int view_size, int map_size)
{
    if (cursor_position < camera + 1)
        return clamp(cursor_position - 1, map_size - view_size + 1);
    if (cursor_position >= camera + view_size - 1)
        return clamp(cursor_position - view_size + 2, map_size - view_size + 1);
    return camera;
}

static void show_view(int x, int y, int camera_x, int camera_y)
{
    REG16(0x04000010) = camera_x % (CACHE_SIZE * CELL_SIZE);
    REG16(0x04000012) = camera_y % (CACHE_SIZE * CELL_SIZE);
    place_piece(x - camera_x, y - camera_y);
}

static void show_pose(int x, int y, unsigned moving, unsigned frame, unsigned step_frames)
{
    if (!moving) {
        OAM[2] = 0;  /* Gold cursor in map browsing mode. */
        return;
    }

    unsigned tile = (frame & IDLE_POSE_FRAMES) ? IDLE_BREATH_TILE : STANDING_TILE;
    if (step_frames)
        tile = WALKING_TILE;
    if (maps[RIVER_MAP][y][x] == '~')
        tile = (frame & SWIM_POSE_FRAMES) ? SWIM_STROKE_TILE : SWIM_GLIDE_TILE;
    OAM[2] = (1 << 12) | (1 << 10) | tile;
}

static void show_breakables(int camera_x, int camera_y, unsigned moving, unsigned broken)
{
    for (unsigned i = 0; i < BREAKABLE_COUNT; ++i) {
        const Breakable *object = &breakables[i];
        volatile uint16_t *sprite = OAM + 4 * (i + 1);
        int x = object->x * CELL_SIZE - camera_x;
        int y = object->y * CELL_SIZE - camera_y;
        if (!moving || x <= -CELL_SIZE || x >= VIEW_WIDTH * CELL_SIZE ||
            y <= -CELL_SIZE || y >= VIEW_HEIGHT * CELL_SIZE) {
            sprite[0] = OBJ_DISABLED;
            continue;
        }
        unsigned tile = (broken & (1u << i)) ? BROKEN_FENCE_TILE : object->tile;
        sprite[0] = y & 255;
        sprite[1] = OBJ_SIZE_16 | (x & 511);
        sprite[2] = (2 << 12) | (1 << 10) | tile;
    }
}

static unsigned target_in_reach(unsigned index, int x, int y, unsigned broken)
{
    int dx = x - breakables[index].x;
    int dy = y - breakables[index].y;
    return !(broken & (1u << index)) && dx >= -1 && dx <= 1 && dy >= -1 && dy <= 1;
}

static unsigned cycle_target(unsigned selected, int direction, int x, int y, unsigned broken)
{
    for (unsigned count = 0; count < BREAKABLE_COUNT; ++count) {
        int next = (int)selected + direction;
        if (next < 0)
            next = BREAKABLE_COUNT - 1;
        if (next == BREAKABLE_COUNT)
            next = 0;
        selected = next;
        if (target_in_reach(selected, x, y, broken))
            return selected;
    }
    return BREAKABLE_COUNT;
}

static unsigned nearest_breakable(int x, int y, unsigned broken)
{
    unsigned target = BREAKABLE_COUNT;
    int best = 3;
    for (unsigned i = 0; i < BREAKABLE_COUNT; ++i) {
        int dx = x - breakables[i].x;
        int dy = y - breakables[i].y;
        if (!target_in_reach(i, x, y, broken))
            continue;
        int distance = (dx < 0 ? -dx : dx) + (dy < 0 ? -dy : dy);
        if (distance < best) {
            best = distance;
            target = i;
        }
    }
    return target;
}

static unsigned roll_d20(uint32_t frame)
{
    /* Input timing supplies the seed. Reject the incomplete group of 20. */
    uint32_t seed = frame * 2654435761u;
    unsigned value;
    do {
        seed = seed * 1664525u + 1013904223u;
        value = seed >> 24;
    } while (value >= 240);
    while (value >= 20)
        value -= 20;
    return value + 1;
}

static unsigned scene_spark(unsigned x, unsigned y)
{
    if ((x == 7 && y >= 2 && y <= 13) || (y == 7 && x >= 2 && x <= 13))
        return 1;
    if ((x == y || x + y == 14) && x >= 4 && x <= 10)
        return 1;
    return 0;
}

static void scene_text(unsigned row, const char *text)
{
    for (unsigned x = 0; x < VIEW_WIDTH * 2; ++x)
        SCENE_MAP[row * MAP_STRIDE + x] =
            (2 << 12) | (HUD_TILE_BASE + hud_glyph(*text ? *text++ : ' '));
}

static void prepare_closeup(void)
{
    static const uint16_t colors[] = {
        RGB5(4, 6, 11), RGB5(6, 9, 14), RGB5(9, 12, 16),
        RGB5(10, 12, 9), RGB5(15, 14, 9), RGB5(18, 16, 11)
    };
    for (unsigned i = 0; i < sizeof colors / sizeof colors[0]; ++i)
        BG_PALETTE[16 * SCENE_PALETTE + 1 + i] = colors[i];
    volatile uint16_t *tiles = BG_TILES + SCENE_TILE * 16;
    for (unsigned tile = 0; tile < 6; ++tile) {
        for (unsigned y = 0; y < 8; ++y) {
            for (unsigned x = 0; x < 8; x += 4) {
                unsigned packed = 0;
                for (unsigned i = 0; i < 4; ++i) {
                    unsigned shade = tile + 1;
                    if (tile == 4 && fleck(x + i, y) < 5)
                        shade = 6;
                    if (tile == 2 && y + ((x + i) & 3) < 4)
                        shade = 2;
                    packed |= shade << (4 * i);
                }
                *tiles++ = packed;
            }
        }
    }
    for (unsigned y = 0; y < 20; ++y) {
        for (unsigned x = 0; x < 30; ++x) {
            unsigned horizon = 8 + ((x / 5) & 1);
            unsigned tile = y < 5 ? 0 : y < horizon ? 1 : y == horizon ? 2 :
                            y < 12 ? 3 : y == 12 ? 5 : 4;
            SCENE_MAP[y * MAP_STRIDE + x] = (SCENE_PALETTE << 12) | (SCENE_TILE + tile);
        }
    }
    scene_text(1, " STRENGTH  ATHLETICS");
    scene_text(18, " SMASH");
    scene_text(19, "");
    write_cell(OBJ_TILES + SPARK_TILE * 16, scene_spark);
    for (unsigned i = 0; i <= TARGET_OAM / 4; ++i)
        OAM[4 * i] = OBJ_DISABLED;
    /* 8.8 affine matrix with a half-size source step gives 2x sprites. */
    OAM[3] = 128;
    OAM[7] = 0;
    OAM[11] = 0;
    OAM[15] = 128;
}

static void scene_sprite(unsigned slot, int x, int y, unsigned tile, unsigned palette)
{
    volatile uint16_t *sprite = OAM + slot * 4;
    sprite[0] = (y & 255) | (3 << 8);  /* Affine, double-size canvas. */
    sprite[1] = OBJ_SIZE_16 | (x & 511);
    sprite[2] = (palette << 12) | tile;
}

static void animate_closeup(unsigned age, const Breakable *target, unsigned success)
{
    int x = 57;
    int target_x = 147;
    if (age < 12)
        x -= age / 2;
    else if (age < SCENE_IMPACT)
        x = 51 + 6 * (age - 12);
    else if (age < SCENE_RESULT) {
        x = 117 - 4 * (age - SCENE_IMPACT);
        target_x += (age & 1) ? 3 : 0;
    } else
        x = 77;

    scene_sprite(0, x, 79, age >= 12 && age < SCENE_RESULT ? WALKING_TILE : STANDING_TILE, 1);
    scene_sprite(1, target_x, 79, success && age >= SCENE_IMPACT ? BROKEN_FENCE_TILE : target->tile, 2);
    if (age >= SCENE_IMPACT && age < SCENE_RESULT)
        scene_sprite(2, 132, 69, SPARK_TILE, 0);
    else
        OAM[8] = OBJ_DISABLED;
    if (age == SCENE_RESULT) {
        for (unsigned x = 0; x < VIEW_WIDTH * 2; ++x)
            SCENE_MAP[18 * MAP_STRIDE + x] = HUD_MAP[MAP_STRIDE + x];
        scene_text(19, " X NEXT");
    }
}

static unsigned solid(int x, int y, unsigned broken_objects)
{
    for (unsigned i = 0; i < BREAKABLE_COUNT; ++i)
        if (!(broken_objects & (1u << i)) && x == breakables[i].x && y == breakables[i].y)
            return 1;
    char terrain = maps[RIVER_MAP][y][x];
    return terrain == '#' || terrain == 'T';
}

static unsigned step_cost(int x, int y, int next_x, int next_y, unsigned swim_speed, unsigned broken_objects)
{
    const char (*river)[MAP_WIDTH + 1] = maps[RIVER_MAP];
    char destination = river[next_y][next_x];
    if ((x == next_x && y == next_y) || solid(next_x, next_y, broken_objects))
        return 0;
    if (x != next_x && y != next_y &&
        (solid(next_x, y, broken_objects) || solid(x, next_y, broken_objects)))
        return 0;
    if (destination == '~')
        return swim_speed ? 5 : 10;
    return destination == ',' ? 10 : 5;
}

static int approach(int pixel, int target)
{
    if (target > pixel + PIXELS_PER_FRAME)
        return pixel + PIXELS_PER_FRAME;
    if (target < pixel - PIXELS_PER_FRAME)
        return pixel - PIXELS_PER_FRAME;
    return target;
}

int main(void)
{
#ifdef START_RIVER_TRIAL
    unsigned map = RIVER_MAP;
    unsigned moving = 1;
#else
    unsigned map = 0;
    unsigned moving = 0;
#endif
    int x = moving ? 9 : 3;
    int y = moving ? 4 : 5;
    int camera_x = 0;
    int camera_y = 0;
    int sprite_x = x * CELL_SIZE;
    int sprite_y = y * CELL_SIZE;
    int scroll_x = 0;
    int scroll_y = 0;
    unsigned spent = 0;
    unsigned animation_frame = 0;
    unsigned step_frames = 0;
    unsigned swim_speed = 0;
    unsigned previous = 0;
    unsigned broken_objects = 0;
    FenceMenu menu = FENCE_MENU_CLOSED;
    unsigned selected = BREAKABLE_COUNT;
    unsigned scene_age = 0;
    unsigned scene_success = 0;
    unsigned action_spent = 0;
    unsigned repeat_timer = REPEAT_DELAY;
    const uint16_t display = BG0_ENABLE | OBJ_ENABLE | OBJ_1D;

    REG16(0x04000000) = FORCE_BLANK;
    REG16(0x04000008) = BG_SIZE_512 | (SCREEN_BLOCK << 8) | 2;
    REG16(0x0400000a) = HUD_SCREEN_BLOCK << 8;
    REG16(0x04000014) = 0;
    REG16(0x04000016) = 0;
    load_tiles();
    load_hud_font();
    load_menu_cards();
    load_starting_view(map);

    for (unsigned i = 0; i < 128; ++i)
        OAM[4 * i] = OBJ_DISABLED;
    OBJ_PALETTE[1] = RGB5(31, 30, 18);
    OBJ_PALETTE[2] = RGB5(9, 6, 5);
    write_cell(OBJ_TILES, cursor);
    static const uint16_t fighter_colors[] = {
        RGB5(3, 4, 7), RGB5(16, 20, 23), RGB5(27, 29, 28),
        RGB5(28, 19, 13), RGB5(19, 5, 6), RGB5(27, 9, 9),
        RGB5(13, 9, 5), RGB5(28, 20, 9), RGB5(7, 8, 11),
        RGB5(15, 23, 26), RGB5(26, 30, 30)
    };
    for (unsigned i = 0; i < sizeof fighter_colors / sizeof fighter_colors[0]; ++i)
        OBJ_PALETTE[17 + i] = fighter_colors[i];
    write_cell(OBJ_TILES + STANDING_TILE * 16, fighter);
    write_cell(OBJ_TILES + SWIM_GLIDE_TILE * 16, swimming_fighter);
    write_cell(OBJ_TILES + WALKING_TILE * 16, walking_fighter);
    write_cell(OBJ_TILES + SWIM_STROKE_TILE * 16, stroking_fighter);
    OBJ_PALETTE[33] = RGB5(8, 4, 3);
    OBJ_PALETTE[34] = RGB5(21, 12, 6);
    OBJ_PALETTE[35] = RGB5(27, 18, 9);
    write_cell(OBJ_TILES + FENCE_TILE * 16, fence);
    write_cell(OBJ_TILES + BROKEN_FENCE_TILE * 16, broken_fence);
    write_cell(OBJ_TILES + CRATE_TILE * 16, crate);
    write_cell(OBJ_TILES + BARREL_TILE * 16, barrel);
    write_cell(OBJ_TILES + IDLE_BREATH_TILE * 16, breathing_fighter);
    show_view(sprite_x, sprite_y, scroll_x, scroll_y);
    show_breakables(scroll_x, scroll_y, moving, broken_objects);
    show_pose(x, y, moving, animation_frame, step_frames);
    if (moving) {
        draw_hud(spent, swim_speed);
        hud_status(" X SELECT OBJECT");
    }
    wait_vblank();
    REG16(0x04000000) = display | (moving ? BG1_ENABLE : 0);

    for (;;) {
        wait_vblank();
        ++animation_frame;
        if (step_frames)
            --step_frames;
        if (moving && !scene_age) {
            sprite_x = approach(sprite_x, x * CELL_SIZE);
            sprite_y = approach(sprite_y, y * CELL_SIZE);
            scroll_x = approach(scroll_x, camera_x * CELL_SIZE);
            scroll_y = approach(scroll_y, camera_y * CELL_SIZE);
            show_view(sprite_x, sprite_y, scroll_x, scroll_y);
            show_breakables(scroll_x, scroll_y, moving, broken_objects);
            show_pose((sprite_x + 8) / CELL_SIZE, (sprite_y + 8) / CELL_SIZE,
                      moving, animation_frame, step_frames);
            if (menu != FENCE_MENU_CLOSED)
                show_target(&breakables[selected], scroll_x, scroll_y);
        }
        /* KEYINPUT is active low. Keep only buttons newly pressed this frame. */
        unsigned held = ~REG16(0x04000130) & (KEY_DPAD | KEY_A | KEY_B | KEY_SELECT | KEY_START);
        unsigned pressed = held & ~previous;
        unsigned direction = held & KEY_DPAD;
        if (direction == 0 || direction != (previous & KEY_DPAD)) {
            repeat_timer = REPEAT_DELAY;
        } else if (--repeat_timer == 0) {
            pressed |= direction;
            repeat_timer = REPEAT_PERIOD;
        }
        previous = held;

        if (scene_age) {
            if (scene_age <= SCENE_RESULT)
                ++scene_age;
            if (scene_age > SCENE_RESULT && (pressed & (KEY_A | KEY_B))) {
                scene_age = 0;
                REG16(0x04000000) = FORCE_BLANK;
                REG16(0x0400000a) = HUD_SCREEN_BLOCK << 8;
                show_view(sprite_x, sprite_y, scroll_x, scroll_y);
                show_breakables(scroll_x, scroll_y, moving, broken_objects);
                show_pose(x, y, moving, animation_frame, 0);
                REG16(0x04000000) = display | BG1_ENABLE;
            } else {
                animate_closeup(scene_age, &breakables[selected], scene_success);
            }
            continue;
        }

        if ((!moving && (pressed & KEY_A)) || ((pressed & KEY_START) && map == RIVER_MAP)) {
            if (!moving && (pressed & KEY_A)) {
                map = (map + 1 == MAP_COUNT) ? 0 : map + 1;
                moving = 0;
            } else {
                moving = !moving;
            }
            spent = 0;
            step_frames = 0;
            broken_objects = 0;
            action_spent = 0;
            swim_speed = 0;
            x = moving ? 9 : 3;
            y = moving ? 4 : 5;
            camera_x = 0;
            camera_y = 0;
            sprite_x = x * CELL_SIZE;
            sprite_y = y * CELL_SIZE;
            scroll_x = 0;
            scroll_y = 0;
            REG16(0x04000000) = FORCE_BLANK;
            if (menu != FENCE_MENU_CLOSED) {
                clear_fence_menu();
                menu = FENCE_MENU_CLOSED;
            }
            load_starting_view(map);
            show_view(sprite_x, sprite_y, scroll_x, scroll_y);
            show_breakables(scroll_x, scroll_y, moving, broken_objects);
            show_pose(x, y, moving, animation_frame, step_frames);
            if (moving) {
                draw_hud(spent, swim_speed);
                hud_status(" X SELECT OBJECT");
            }
            wait_vblank();
            REG16(0x04000000) = display | (moving ? BG1_ENABLE : 0);
        } else if (menu != FENCE_MENU_CLOSED) {
            if ((pressed & KEY_B) || ((pressed & KEY_A) && menu == FENCE_MENU_CANCEL)) {
                clear_fence_menu();
                menu = FENCE_MENU_CLOSED;
                hud_status(" X SELECT OBJECT");
                continue;
            }
            if (!(pressed & KEY_A)) {
                if (pressed & KEY_LEFT)
                    menu = FENCE_MENU_SMASH;
                else if (pressed & KEY_RIGHT)
                    menu = FENCE_MENU_CANCEL;
                if (pressed & (KEY_UP | KEY_DOWN))
                    selected = cycle_target(selected, (pressed & KEY_DOWN) ? 1 : -1,
                                            x, y, broken_objects);
                if (pressed & KEY_DPAD) {
                    draw_breakable_menu(menu, selected);
                    show_target(&breakables[selected], scroll_x, scroll_y);
                }
                continue;
            }
            clear_fence_menu();
            menu = FENCE_MENU_CLOSED;
            unsigned roll = roll_d20(animation_frame);
            unsigned total = roll + ATHLETICS_BONUS;
            unsigned dc = breakables[selected].dc;
            unsigned success = total >= dc;
            if (success)
                broken_objects |= 1u << selected;
            action_spent = 1;
            hud_status(success ? " D20 00 +4 = 00 DC00 BROKE"
                               : " D20 00 +4 = 00 DC00 FAIL");
            hud_number(MAP_STRIDE + 18, dc);
            hud_number(MAP_STRIDE + 5, roll);
            hud_number(MAP_STRIDE + 13, total);
            scene_success = success;
            scene_age = 1;
            REG16(0x04000000) = FORCE_BLANK;
            prepare_closeup();
            REG16(0x0400000a) = SCENE_BLOCK << 8;
            animate_closeup(scene_age, &breakables[selected], scene_success);
            wait_vblank();
            REG16(0x04000000) = BG1_ENABLE | OBJ_ENABLE | OBJ_1D;
        } else if (moving && (pressed & KEY_A)) {
            if (action_spent)
                continue;
            selected = nearest_breakable(x, y, broken_objects);
            if (selected == BREAKABLE_COUNT) {
                hud_status(" MOVE NEXT TO OBJECT");
                continue;
            }
            menu = FENCE_MENU_SMASH;
            hide_status();
            draw_breakable_menu(menu, selected);
            show_target(&breakables[selected], scroll_x, scroll_y);
        } else if (moving && (pressed & (KEY_B | KEY_SELECT))) {
            if (pressed & KEY_SELECT)
                swim_speed = !swim_speed;
            if (pressed & KEY_B) {
                action_spent = 0;
                hud_status(" X SELECT OBJECT");
            }
            spent = 0;
            draw_hud(spent, swim_speed);
        } else if (pressed & KEY_DPAD) {
            int dx = !!(pressed & KEY_RIGHT) - !!(pressed & KEY_LEFT);
            int dy = !!(pressed & KEY_DOWN) - !!(pressed & KEY_UP);
            int destination_x = clamp(x + dx, MAP_WIDTH);
            int destination_y = clamp(y + dy, MAP_HEIGHT);
            if (moving) {
                unsigned cost = step_cost(x, y, destination_x, destination_y, swim_speed, broken_objects);
                if (cost == 0 || spent + cost > TURN_FEET)
                    continue;
                spent += cost;
                step_frames = WALK_POSE_FRAMES;
                draw_hud(spent, swim_speed);
            }
            x = destination_x;
            y = destination_y;
            int next_x = follow(x, camera_x, VIEW_WIDTH, MAP_WIDTH);
            int next_y = follow(y, camera_y, VIEW_HEIGHT, MAP_HEIGHT);
            stream_edges(map, camera_x, camera_y, next_x, next_y);
            camera_x = next_x;
            camera_y = next_y;
            if (!moving) {
                sprite_x = x * CELL_SIZE;
                sprite_y = y * CELL_SIZE;
                scroll_x = camera_x * CELL_SIZE;
                scroll_y = camera_y * CELL_SIZE;
                show_view(sprite_x, sprite_y, scroll_x, scroll_y);
            }
        }
    }
}
