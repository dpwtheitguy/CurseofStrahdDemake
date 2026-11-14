#include <stdint.h>

enum {
    MAP_WIDTH = 64,
    MAP_HEIGHT = 64,
    CACHE_SIZE = 32,
    VIEW_WIDTH = 15,
    VIEW_HEIGHT = 10,
    CELL_SIZE = 16,
    HUD_HEIGHT = 8,
    TRIAL_VIEW_HEIGHT = (VIEW_HEIGHT * CELL_SIZE - HUD_HEIGHT) / CELL_SIZE,
    TILE_SIZE = 8,
    MAP_STRIDE = 32,
    SCREEN_BLOCK = 16,
    VISIBLE_LINES = 160,
    BG_SIZE_512 = 3 << 14,
    KEY_A = 1,
    KEY_B = 1 << 1,
    KEY_SELECT = 1 << 2,
    KEY_START = 1 << 3,
    KEY_L = 1 << 9,
    BG1_ENABLE = 1 << 9,
    HUD_SCREEN_BLOCK = 20,
    SCENE_BLOCK = 21,
    RANGE_BLOCK = 22,
    BG2_ENABLE = 1 << 10,
    BRIGHTEN_BG2 = (2 << 6) | (1 << 2),
    SCENE_TILE = 320,
    SCENE_PALETTE = 5,
    SCENE_DIALOG = 12,
    SCENE_TITLE = 13,
    SPARK_TILE = 40,
    ARROW_TILE = 92,
    SHOOT_CARD_TILE = 96,
    CLOSEUP_READY_TILE = 44,
    CLOSEUP_BRACE_TILE = 60,
    CLOSEUP_STRIKE_TILE = 76,
    SCENE_ENTRANCE = 15,
    SCENE_IMPACT = 24,
    SCENE_RESULT = 34,
    HUD_TILE_BASE = 32,
    RIVER_MAP = 1,
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
    EXTRA_BREAKABLE_SLOT = 8,
    PALLET_TILE = 172,
    BASKET_TILE = 176,
    STOOL_TILE = 180,
    DUMMY_TILE = 184,
    TARGET_TILE = 188,
    CHARACTER_FRAME_TILE = 168,
    PORTRAIT_TILE = 177,
    CHARACTER_PALETTE = 6,
    NAME_PALETTE = 7,
    PORTRAIT_PALETTE = 8,
    CHARACTER_X = 1,
    CHARACTER_Y = 3,
    CHARACTER_WIDTH = 16,
    CHARACTER_HEIGHT = 9,
    ROLL_ROW = 24,
    CARD_TILES = 4,
    CARD_SIZE = CARD_TILES * TILE_SIZE,
    SMASH_CARD_TILE = 128,
    BACK_CARD_TILE = 144,
    DASH_CARD_TILE = 200,
    BONUS_CARD_TILE = 216,
    ACTION_CARD_TILE = 232,
    MOVE_CARD_TILE = 248,
    CANCEL_CARD_TILE = 264,
    TURN_HOLD_FRAMES = 12,
    TURN_CUE_FRAMES = 24,
    TURN_BANNER_TILE = 96,
    TURN_BANNER_SLOT = 5,
    TURN_BANNER_PALETTE = 4,
    GEAR_CUE_SLOT = 7,
    GEAR_CUE_TILE = 160,
    GEAR_CUE_PALETTE = 5,
    GEAR_CUE_FRAMES = 24,
    GEAR_MESSAGE_FRAMES = 84,
    GEAR_MESSAGE_ROW = 19,
    GEAR_MESSAGE_WIDTH = 30,
    ITEMS_X = 6,
    ITEMS_Y = 5,
    ITEMS_WIDTH = 18,
    ITEMS_HEIGHT = 8,
    ROOT_ACTION_X = 4,
    ROOT_MOVE_X = 9,
    ROOT_BONUS_X = 14,
    ROOT_ITEMS_X = 19,
    ROOT_CANCEL_X = 24,
    ITEMS_CARD_TILE = 80,
    DISABLED_PALETTE = 9,
    OBJECT_PALETTE = 10,
    OBJECT_TITLE_PALETTE = 11,
    OBJECT_WIDTH = 14,
    OBJECT_HEIGHT = 5,
    OBJECT_Y = 2,
    OBJECT_LEFT_X = 1,
    OBJECT_RIGHT_X = VIEW_WIDTH * 2 - OBJECT_WIDTH - 1,
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

#define REG32(address) (*(volatile uint32_t *)(address))
#define REG16(address) (*(volatile uint16_t *)(address))
#define BG_PALETTE ((volatile uint16_t *)0x05000000)
#define OBJ_PALETTE ((volatile uint16_t *)0x05000200)
#define BG_TILES ((volatile uint16_t *)0x06000000)
#define BG_MAP ((volatile uint16_t *)(0x06000000 + SCREEN_BLOCK * 2048))
#define RANGE_MAP ((volatile uint16_t *)(0x06000000 + RANGE_BLOCK * 2048))
#define HUD_MAP ((volatile uint16_t *)(0x06000000 + HUD_SCREEN_BLOCK * 2048))
#define SCENE_MAP ((volatile uint16_t *)(0x06000000 + SCENE_BLOCK * 2048))
#define OBJ_TILES ((volatile uint16_t *)0x06010000)
#define OAM ((volatile uint16_t *)0x07000000)
#define RGB5(r, g, b) ((r) | ((g) << 5) | ((b) << 10))

#include "battlefield-maps.h"
#include "battlefield-fighter.h"

#include "battlefield-character.h"

typedef enum {
    ACTION_MENU_CLOSED,
    ACTION_MENU_ACTION,
    ACTION_MENU_MOVEMENT,
    ACTION_MENU_BONUS,
    ACTION_MENU_ITEMS,
    ACTION_MENU_CANCEL,
    ACTION_MENU_SMASH,
    ACTION_MENU_DASH,
    ACTION_MENU_ACTION_CANCEL,
    ACTION_MENU_DRAW_SHOOT,
    EQUIP_MENU_WEAPON,
    EQUIP_MENU_BOW,
    EQUIP_MENU_SHIELD,
    EQUIP_MENU_BACK,
    ACTION_MENU_MOVE,
    ACTION_MENU_MOVE_CANCEL,
    ACTION_MENU_BONUS_DASH,
    ACTION_MENU_BONUS_CANCEL
} ActionMenu;

typedef struct {
    int x, y;
    unsigned tile, dc, ac, hp;
    const char *name, *description;
} Breakable;

static const Breakable breakables[] = {
    {9, 3, FENCE_TILE, 12, 15, 4, "FENCE", "WOODEN FENCE"},
    {9, 6, CRATE_TILE, 10, 15, 3, "CRATE", "WOODEN CRATE"},
    {11, 5, BARREL_TILE, 14, 15, 5, "BARREL", "WOODEN CASK"},
    {17, 5, PALLET_TILE, 9, 15, 3, "PALLET", "WOODEN PALLET"},
    {18, 6, BASKET_TILE, 8, 11, 2, "BASKET", "WOVEN BASKET"},
    {20, 5, STOOL_TILE, 10, 15, 3, "STOOL", "WOODEN STOOL"},
    {19, 5, DUMMY_TILE, 9, 15, 4, "DUMMY", "WOODEN DUMMY"},
    {19, 6, TARGET_TILE, 9, 11, 3, "TARGET", "ARCHERY TARGET"},
    {18, 5, TARGET_TILE, 9, 11, 3, "BULLSEYE", "ARCHERY TARGET"},
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

static void load_map_fighter(const Loadout *equipment)
{
    static const struct { unsigned tile; PixelColor pose; } poses[] = {
        {STANDING_TILE, fighter}, {SWIM_GLIDE_TILE, swimming_fighter},
        {WALKING_TILE, walking_fighter}, {SWIM_STROKE_TILE, stroking_fighter},
        {IDLE_BREATH_TILE, breathing_fighter},
    };
    for (unsigned i = 0; i < sizeof poses / sizeof poses[0]; ++i) {
        unsigned swim = poses[i].tile == SWIM_GLIDE_TILE || poses[i].tile == SWIM_STROKE_TILE;
        volatile uint16_t *out = OBJ_TILES + poses[i].tile * 16;
        for (unsigned top = 0; top < 16; top += 8)
            for (unsigned left = 0; left < 16; left += 8)
                for (unsigned y = top; y < top + 8; ++y)
                    for (unsigned x = left; x < left + 8; x += 4) {
                        unsigned packed = 0;
                        for (unsigned col = 0; col < 4; ++col) {
                            unsigned px = x + col;
                            unsigned pixel = poses[i].pose(px, y);
                            if (equipment->armor == ITEM_NONE && ((swim && y < 7) || (!swim && y < 5))) {
                                if (pixel == 2 || pixel == 3)
                                    pixel = y < (swim ? 5 : 3) ? 7 : 4;
                            }
                            if (!swim && equipment->weapon == ITEM_LONGBOW && px >= 10) {
                                static const uint8_t bow_limb[] = {10, 11, 12, 13, 14, 14, 14,
                                                                    14, 14, 13, 12, 11, 10};
                                if (y >= 2 && y < 15) {
                                    unsigned limb = bow_limb[y - 2];
                                    if (px == limb)
                                        pixel = 8;
                                    else if (px == limb + 1)
                                        pixel = 7;
                                    else if (px == 10)
                                        pixel = y >= 7 && y <= 9 ? 4 : 3;
                                    else if (px > 11)
                                        pixel = 0;
                                }
                            }
                            if (!swim && equipment->weapon == ITEM_WARHAMMER && px >= 12 && y >= 3)
                                pixel = y >= 4 && y <= 6 ? (y == 4 || px == 12 ? 3 : 2) :
                                        px == 13 && y >= 7 && y <= 11 ? 8 : 0;
                            if (!swim && equipment->weapon == ITEM_NONE && px >= 12 && y >= 6)
                                pixel = 0;
                            if (!swim && !equipment->shield && px <= 5 && y >= 8 && y < 12)
                                pixel = px >= 4 && pixel ? 4 : 0;
                            packed |= pixel << (4 * col);
                        }
                        *out++ = packed;
                    }
    }
}

static void load_swimming_bow(void)
{
    static const unsigned swim_tiles[] = {SWIM_GLIDE_TILE, SWIM_STROKE_TILE};
    for (unsigned pose = 0; pose < sizeof swim_tiles / sizeof swim_tiles[0]; ++pose) {
        volatile uint16_t *right = OBJ_TILES + (swim_tiles[pose] + 1) * 16;
        /* The upper bow limb rides above the wake at the right shoulder. */
        right[3] = (right[3] & 0xfff0) | 8;
        right[5] = (right[5] & 0xfff0) | 8;
        right[7] = (right[7] & 0xf0ff) | 0x0800;
        right[9] = (right[9] & 0xf0ff) | 0x0800;
        right[11] = (right[11] & 0x0fff) | 0x8000;
    }
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

static unsigned pallet(unsigned x, unsigned y)
{
    if (x < 2 || x > 13 || y < 7 || y > 13)
        return 0;
    if (y >= 12)
        return x <= 4 || x >= 11 ? 1 : 0;
    return y == 9 || x == 2 || x == 13 ? 1 : y == 7 || y == 10 ? 3 : 2;
}

static unsigned basket(unsigned x, unsigned y)
{
    if (y < 5 || y > 14 || x < (y > 11 ? 4u : 2u) || x > (y > 11 ? 11u : 13u))
        return 0;
    return y == 5 || y == 14 ? 1 : (x + y) & 1 ? 2 : 3;
}

static unsigned stool(unsigned x, unsigned y)
{
    if (y >= 5 && y <= 8 && x >= 2 && x <= 13)
        return y == 5 ? 3 : 2;
    return y > 8 && y <= 14 && (x == 4 || x == 11) ? 1 : 0;
}

static unsigned dummy(unsigned x, unsigned y)
{
    if (y >= 2 && y <= 5 && x >= 6 && x <= 9)
        return y == 2 ? 3 : 2;
    if (y >= 6 && y <= 8 && x >= 2 && x <= 13)
        return y == 6 ? 3 : 2;
    if (y >= 9 && y <= 11 && x >= 5 && x <= 10)
        return x == 5 || x == 10 ? 1 : 2;
    return y >= 12 && y <= 14 && (x == 7 || x == 8 || (y == 14 && x >= 4 && x <= 11)) ? 1 : 0;
}

static unsigned archery_target(unsigned x, unsigned y)
{
    int dx = (int)x - 7, dy = (int)y - 6;
    int radius = dx * dx + dy * dy;
    if (radius <= 30)
        return radius <= 3 || radius >= 22 ? 1 : radius <= 11 ? 3 : 2;
    return y >= 10 && y <= 14 && (x == 5 || x == 9) ? 2 : 0;
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

static unsigned dash_icon(unsigned x, unsigned y)
{
    if ((y == 4 && x >= 1 && x <= 5) ||
        (y == 7 && x >= 2 && x <= 6) ||
        (y == 10 && x >= 1 && x <= 4))
        return 3;
    if ((x >= 8 && x <= 12 && y >= 3 && y <= 10) ||
        (x >= 8 && x <= 15 && y >= 10 && y <= 13)) {
        if (y == 13 || x == 15)
            return 1;
        return x == 8 || y == 3 ? 3 : 2;
    }
    return 0;
}

static unsigned move_icon(unsigned x, unsigned y)
{
    unsigned dx = x < 7 ? 7 - x : x - 7;
    unsigned dy = y < 7 ? 7 - y : y - 7;
    if ((dx <= 1 && dy <= 6) || (dy <= 1 && dx <= 6))
        return dx == 0 || dy == 0 ? 3 : 2;
    if ((dy >= 4 && dy <= 6 && dx + dy <= 7) ||
        (dx >= 4 && dx <= 6 && dx + dy <= 7))
        return 3;
    return 0;
}

static unsigned bow_icon(unsigned x, unsigned y)
{
    unsigned arc = y < 8 ? y : 15 - y;
    if (x == 6 + arc / 2)
        return 3;
    if (x == 6 && y > 1 && y < 14)
        return 1;
    return y == 7 && x >= 2 && x <= 13 ? 2 : 0;
}

static unsigned card_pixel(unsigned x, unsigned y, PixelColor icon,
                           const uint32_t *caption_rows)
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

    if (y >= 22 && y < 29 && (caption_rows[y - 22] & (1u << x)))
        return 7;
    return y >= 22 ? 1 : 3;
}

static void write_card(unsigned first_tile, PixelColor icon, const char *caption)
{
    unsigned letters = 0;
    for (const char *letter = caption; *letter; ++letter)
        ++letters;
    unsigned glyph_step = letters > 5 ? 4 : 6;
    unsigned text_width = letters * glyph_step - 1;
    unsigned left = (CARD_SIZE - text_width) / 2;
    uint32_t caption_rows[7];
    for (unsigned row = 0; row < 7; ++row)
        caption_rows[row] = 0;
    for (unsigned letter = 0; letter < letters; ++letter) {
        static const unsigned narrow_columns[] = {24, 4, 3};
        const uint8_t *glyph = hud_glyphs[hud_glyph(caption[letter])];
        for (unsigned col = 0; col + 1 < glyph_step; ++col) {
            unsigned mask = glyph_step == 4 ? narrow_columns[col] : 16u >> col;
            for (unsigned row = 0; row < 7; ++row) {
                if (glyph[row] & mask)
                    caption_rows[row] |= 1u << (left + letter * glyph_step + col);
            }
        }
    }
    volatile uint16_t *out = BG_TILES + first_tile * 16;
    for (unsigned tile_y = 0; tile_y < CARD_TILES; ++tile_y) {
        for (unsigned tile_x = 0; tile_x < CARD_TILES; ++tile_x) {
            for (unsigned row = 0; row < TILE_SIZE; ++row) {
                for (unsigned col = 0; col < TILE_SIZE; col += 4) {
                    unsigned packed = 0;
                    for (unsigned i = 0; i < 4; ++i)
                        packed |= card_pixel(tile_x * TILE_SIZE + col + i,
                                             tile_y * TILE_SIZE + row,
                                             icon, caption_rows) << (4 * i);
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
        unsigned gray = (bronze[i] & 31) / 2;
        BG_PALETTE[16 * DISABLED_PALETTE + 1 + i] = RGB5(gray, gray, gray);
    }
    write_card(SMASH_CARD_TILE, hammer_icon, "SMASH");
    write_card(BACK_CARD_TILE, back_icon, "BACK");
    write_card(DASH_CARD_TILE, dash_icon, "DASH");
    write_card(BONUS_CARD_TILE, dash_icon, "BONUS");
    write_card(ACTION_CARD_TILE, hammer_icon, "ACTION");
    write_card(MOVE_CARD_TILE, move_icon, "MOVE");
    write_card(CANCEL_CARD_TILE, back_icon, "CANCEL");
}

static void place_card(unsigned x, unsigned y, unsigned tile, unsigned palette)
{
    for (unsigned row = 0; row < CARD_TILES; ++row)
        for (unsigned col = 0; col < CARD_TILES; ++col)
            HUD_MAP[(y + row) * MAP_STRIDE + x + col] =
                (palette << 12) | (tile + CARD_TILES * row + col);
}

static unsigned bonus_menu(ActionMenu menu)
{
    return menu == ACTION_MENU_BONUS_DASH || menu == ACTION_MENU_BONUS_CANCEL;
}

static unsigned equip_submenu(ActionMenu menu)
{
    return menu >= EQUIP_MENU_WEAPON && menu <= EQUIP_MENU_BACK;
}

static unsigned action_submenu(ActionMenu menu)
{
    return menu == ACTION_MENU_SMASH || menu == ACTION_MENU_DASH || menu == ACTION_MENU_ACTION_CANCEL || menu == ACTION_MENU_DRAW_SHOOT;
}

static unsigned move_submenu(ActionMenu menu)
{
    return menu == ACTION_MENU_MOVE || menu == ACTION_MENU_MOVE_CANCEL;
}

static unsigned card_palette(unsigned focused, unsigned available)
{
    if (focused)
        return ACTIVE_PALETTE;
    return available ? MENU_PALETTE : DISABLED_PALETTE;
}

static void draw_action_menu(ActionMenu menu, unsigned action_available, unsigned smash_available,
                             unsigned bonus_available, unsigned move_available, unsigned bow, unsigned draw_bow)
{
    if (action_submenu(menu)) {
        if (draw_bow)
            place_card(ROOT_ACTION_X, CHOICE_CARD_Y, SHOOT_CARD_TILE,
                       card_palette(menu == ACTION_MENU_DRAW_SHOOT, smash_available));
        place_card(ROOT_MOVE_X, CHOICE_CARD_Y, bow ? SHOOT_CARD_TILE : SMASH_CARD_TILE,
                   card_palette(menu == ACTION_MENU_SMASH, smash_available));
        place_card(ROOT_BONUS_X, CHOICE_CARD_Y, DASH_CARD_TILE,
                   card_palette(menu == ACTION_MENU_DASH, 1));
        place_card(ROOT_ITEMS_X, CHOICE_CARD_Y, BACK_CARD_TILE,
                   card_palette(menu == ACTION_MENU_ACTION_CANCEL, 1));
        return;
    }
    if (bonus_menu(menu) || move_submenu(menu)) {
        unsigned tile = bonus_menu(menu) ? DASH_CARD_TILE :
                        MOVE_CARD_TILE;
        unsigned primary = menu == ACTION_MENU_BONUS_DASH || menu == ACTION_MENU_MOVE;
        place_card(SMASH_CARD_X, CHOICE_CARD_Y, tile, card_palette(primary, 1));
        place_card(BACK_CARD_X, CHOICE_CARD_Y, BACK_CARD_TILE, card_palette(!primary, 1));
        return;
    }
    place_card(ROOT_ACTION_X, CHOICE_CARD_Y, ACTION_CARD_TILE,
               card_palette(menu == ACTION_MENU_ACTION, action_available));
    place_card(ROOT_MOVE_X, CHOICE_CARD_Y, MOVE_CARD_TILE,
               card_palette(menu == ACTION_MENU_MOVEMENT, move_available));
    place_card(ROOT_BONUS_X, CHOICE_CARD_Y, BONUS_CARD_TILE,
               card_palette(menu == ACTION_MENU_BONUS, bonus_available));
    place_card(ROOT_ITEMS_X, CHOICE_CARD_Y, ITEMS_CARD_TILE,
               card_palette(menu == ACTION_MENU_ITEMS, 1));
    place_card(ROOT_CANCEL_X, CHOICE_CARD_Y, CANCEL_CARD_TILE,
               card_palette(menu == ACTION_MENU_CANCEL, 1));
}

static ActionMenu next_command(ActionMenu menu, int direction, unsigned draw_bow)
{
    if (bonus_menu(menu))
        return menu == ACTION_MENU_BONUS_DASH ? ACTION_MENU_BONUS_CANCEL : ACTION_MENU_BONUS_DASH;
    if (move_submenu(menu))
        return menu == ACTION_MENU_MOVE ? ACTION_MENU_MOVE_CANCEL : ACTION_MENU_MOVE;
    if (equip_submenu(menu)) {
        if (menu == EQUIP_MENU_WEAPON && direction < 0)
            return EQUIP_MENU_BACK;
        if (menu == EQUIP_MENU_BACK && direction > 0)
            return EQUIP_MENU_WEAPON;
        return menu + direction;
    }
    if (action_submenu(menu)) {
        int next = menu + direction;
        ActionMenu last = draw_bow ? ACTION_MENU_DRAW_SHOOT : ACTION_MENU_ACTION_CANCEL;
        if (next < ACTION_MENU_SMASH)
            return last;
        if (next > (int)last)
            return ACTION_MENU_SMASH;
        return next;
    }
    int next = (int)menu + direction;
    if (next < ACTION_MENU_ACTION)
        return ACTION_MENU_CANCEL;
    if (next > ACTION_MENU_CANCEL)
        return ACTION_MENU_ACTION;
    return next;
}

static void show_target(const Breakable *target, int camera_x, int camera_y)
{
    int x = target->x * CELL_SIZE - camera_x;
    int y = target->y * CELL_SIZE - camera_y;
    if (x <= -CELL_SIZE || x >= VIEW_WIDTH * CELL_SIZE ||
        y <= -CELL_SIZE || y >= VIEW_HEIGHT * CELL_SIZE) {
        OAM[TARGET_OAM] = OBJ_DISABLED;
        return;
    }
    OAM[TARGET_OAM] = y & 255;
    OAM[TARGET_OAM + 1] = OBJ_SIZE_16 | (x & 511);
    OAM[TARGET_OAM + 2] = 0;  /* Reuse the gold bracket sprite. */
}

static void clear_card(unsigned x, unsigned y)
{
    unsigned blank = HUD_TILE_BASE + sizeof hud_glyphs / sizeof hud_glyphs[0];
    for (unsigned row = 0; row < CARD_TILES; ++row)
        for (unsigned col = 0; col < CARD_TILES; ++col)
            HUD_MAP[(y + row) * MAP_STRIDE + x + col] = blank;
}

static void clear_action_menu(void)
{
    clear_card(SMASH_CARD_X, CHOICE_CARD_Y);
    clear_card(BACK_CARD_X, CHOICE_CARD_Y);
    clear_card(ROOT_ACTION_X, CHOICE_CARD_Y);
    clear_card(ROOT_BONUS_X, CHOICE_CARD_Y);
    clear_card(ROOT_CANCEL_X, CHOICE_CARD_Y);
    clear_card(ROOT_ITEMS_X, CHOICE_CARD_Y);
}

static void turn_click(void)
{
    REG16(0x04000084) = 0x80;   /* Audio master. Leave Direct Sound alone. */
    REG16(0x04000080) = 0x8833; /* Noise on both sides at moderate volume. */
    REG16(0x04000078) = 0x323a; /* Short decay, about 23 ms. */
    REG16(0x0400007c) = 0xc033;
}

static void hud_notice(const char *text)
{
    for (unsigned x = 15; x < VIEW_WIDTH * 2; ++x)
        hud_letter(x, *text ? *text++ : ' ');
}

static void store_roll(const char *text)
{
    for (unsigned x = 0; x < VIEW_WIDTH * 2; ++x)
        hud_letter(ROLL_ROW * MAP_STRIDE + x, *text ? *text++ : ' ');
}

static void panel_number(unsigned entry, unsigned number, unsigned palette)
{
    unsigned tens = 0;
    while (number >= 10) {
        ++tens;
        number -= 10;
    }
    HUD_MAP[entry] = (palette << 12) | (HUD_TILE_BASE + hud_glyph('0' + tens));
    HUD_MAP[entry + 1] = (palette << 12) | (HUD_TILE_BASE + hud_glyph('0' + number));
}

static void hud_number(unsigned entry, unsigned number)
{
    panel_number(entry, number, 2);
}

static unsigned portrait_pixel(unsigned x, unsigned y)
{
    char pixel = portrait_pixels[y / 2][x / 2];
    if (pixel == '.') {
        int dx = (int)x - 15;
        int dy = (int)y - 15;
        return dx * dx + dy * dy < 213 ? 13 : 1;
    }
    return fighter_color(pixel) + 1;
}

static void load_panel_frame(void)
{
    volatile uint16_t *out = BG_TILES + CHARACTER_FRAME_TILE * 16;
    for (unsigned tile_y = 0; tile_y < 3; ++tile_y) {
        for (unsigned tile_x = 0; tile_x < 3; ++tile_x) {
            for (unsigned y = 0; y < 8; ++y) {
                for (unsigned x = 0; x < 8; x += 4) {
                    unsigned packed = 0;
                    for (unsigned i = 0; i < 4; ++i) {
                        unsigned col = x + i;
                        unsigned color = 1;
                        if ((tile_x == 0 && col == 0) || (tile_x == 2 && col == 7) ||
                            (tile_y == 0 && y == 0) || (tile_y == 2 && y == 7))
                            color = 4;
                        else if ((tile_x == 0 && col == 1) || (tile_x == 2 && col == 6) ||
                                 (tile_y == 0 && y == 1) || (tile_y == 2 && y == 6))
                            color = tile_x != 1 && tile_y != 1 ? 5 : 3;
                        packed |= color << (i * 4);
                    }
                    *out++ = packed;
                }
            }
        }
    }
    static const uint16_t blue[] = {
        RGB5(3, 7, 15), RGB5(23, 28, 31), RGB5(7, 13, 23),
        RGB5(13, 20, 29), RGB5(20, 26, 31)
    };
    for (unsigned i = 0; i < sizeof blue / sizeof blue[0]; ++i) {
        BG_PALETTE[OBJECT_PALETTE * 16 + 1 + i] = blue[i];
        BG_PALETTE[OBJECT_TITLE_PALETTE * 16 + 1 + i] = blue[i];
    }
    BG_PALETTE[OBJECT_TITLE_PALETTE * 16 + 2] = RGB5(31, 27, 16);
}

static void load_character_art(void)
{
    static const uint16_t portrait_colors[] = {
        RGB5(4, 7, 12), RGB5(3, 4, 7), RGB5(9, 15, 20), RGB5(17, 22, 25),
        RGB5(25, 29, 30), RGB5(22, 14, 10), RGB5(29, 21, 14), RGB5(13, 8, 7),
        RGB5(22, 8, 8), RGB5(29, 12, 9), RGB5(13, 18, 22), RGB5(18, 22, 24),
        RGB5(8, 12, 17)
    };
    BG_PALETTE[CHARACTER_PALETTE * 16 + 1] = portrait_colors[0];
    BG_PALETTE[CHARACTER_PALETTE * 16 + 2] = RGB5(24, 27, 27);
    BG_PALETTE[CHARACTER_PALETTE * 16 + 3] = RGB5(9, 14, 20);
    BG_PALETTE[CHARACTER_PALETTE * 16 + 4] = RGB5(15, 21, 25);
    BG_PALETTE[CHARACTER_PALETTE * 16 + 5] = RGB5(23, 18, 10);
    BG_PALETTE[NAME_PALETTE * 16 + 1] = portrait_colors[0];
    BG_PALETTE[NAME_PALETTE * 16 + 2] = RGB5(31, 26, 15);
    for (unsigned i = 0; i < sizeof portrait_colors / sizeof portrait_colors[0]; ++i)
        BG_PALETTE[PORTRAIT_PALETTE * 16 + 1 + i] = portrait_colors[i];

    volatile uint16_t *out = BG_TILES + PORTRAIT_TILE * 16;
    for (unsigned tile_y = 0; tile_y < 4; ++tile_y)
        for (unsigned tile_x = 0; tile_x < 4; ++tile_x)
            for (unsigned y = 0; y < 8; ++y)
                for (unsigned x = 0; x < 8; x += 4) {
                    unsigned packed = 0;
                    for (unsigned i = 0; i < 4; ++i)
                        packed |= portrait_pixel(tile_x * 8 + x + i, tile_y * 8 + y) << (i * 4);
                    *out++ = packed;
                }
}

static void panel_text(unsigned x, unsigned y, const char *text, unsigned palette)
{
    while (*text) {
        HUD_MAP[y * MAP_STRIDE + x] = (palette << 12) | (HUD_TILE_BASE + hud_glyph(*text++));
        ++x;
    }
}

static void draw_panel(unsigned x, unsigned y, unsigned width, unsigned height, unsigned palette)
{
    for (unsigned row = 0; row < height; ++row) {
        unsigned edge_y = row == 0 ? 0 : row == height - 1 ? 2 : 1;
        for (unsigned col = 0; col < width; ++col) {
            unsigned edge_x = col == 0 ? 0 : col == width - 1 ? 2 : 1;
            HUD_MAP[(y + row) * MAP_STRIDE + x + col] =
                (palette << 12) | (CHARACTER_FRAME_TILE + 3 * edge_y + edge_x);
        }
    }
}

static unsigned object_panel_x(const Breakable *object, int camera_x)
{
    return object->x - camera_x < VIEW_WIDTH / 2 ? OBJECT_RIGHT_X : OBJECT_LEFT_X;
}

static void show_object_panel(unsigned selected, int camera_x, unsigned details)
{
    if (selected >= BREAKABLE_COUNT)
        return;
    const Breakable *object = &breakables[selected];
    unsigned x = object_panel_x(object, camera_x);
    draw_panel(x, OBJECT_Y, OBJECT_WIDTH, OBJECT_HEIGHT, OBJECT_PALETTE);
    panel_text(x + 1, OBJECT_Y + 1, details ? "DETAILS    B" : object->name, OBJECT_TITLE_PALETTE);
    panel_text(x + 1, OBJECT_Y + 2, details ? object->description : "BREAKABLE", OBJECT_PALETTE);
    panel_text(x + 1, OBJECT_Y + 3, details ? "BLOCKS PATH" : "SEL DETAILS", OBJECT_PALETTE);
}

static void clear_object_panel(unsigned selected, int camera_x)
{
    if (selected < BREAKABLE_COUNT) {
        unsigned x = object_panel_x(&breakables[selected], camera_x);
        unsigned blank = HUD_TILE_BASE + sizeof hud_glyphs / sizeof hud_glyphs[0];
        for (unsigned row = 0; row < OBJECT_HEIGHT; ++row)
            for (unsigned col = 0; col < OBJECT_WIDTH; ++col)
                HUD_MAP[(OBJECT_Y + row) * MAP_STRIDE + x + col] = blank;
    }
    OAM[TARGET_OAM] = OBJ_DISABLED;
}

static void show_character(const Character *character, unsigned swim_speed)
{
    draw_panel(CHARACTER_X, CHARACTER_Y, CHARACTER_WIDTH, CHARACTER_HEIGHT, CHARACTER_PALETTE);
    place_card(2, 5, PORTRAIT_TILE, PORTRAIT_PALETTE);
    panel_text(7, 4, character->name, NAME_PALETTE);
    panel_text(7, 5, character->class_name, CHARACTER_PALETTE);
    panel_text(7, 6, "HP  00/00", CHARACTER_PALETTE);
    panel_text(7, 7, "AC   00", CHARACTER_PALETTE);
    panel_text(7, 8, "SPD 00 FT", CHARACTER_PALETTE);
    panel_text(7, 9, "ATH  +00", CHARACTER_PALETTE);
    panel_text(7, 10, swim_speed ? "SWIM ON" : "SWIM OFF", CHARACTER_PALETTE);
    panel_number(6 * MAP_STRIDE + 11, character->hp, CHARACTER_PALETTE);
    panel_number(6 * MAP_STRIDE + 14, character->max_hp, CHARACTER_PALETTE);
    panel_number(7 * MAP_STRIDE + 12, armor_class(character), CHARACTER_PALETTE);
    panel_number(8 * MAP_STRIDE + 11, character->speed, CHARACTER_PALETTE);
    panel_number(9 * MAP_STRIDE + 13, character->athletics, CHARACTER_PALETTE);
}

static void clear_character(void)
{
    unsigned blank = HUD_TILE_BASE + sizeof hud_glyphs / sizeof hud_glyphs[0];
    for (unsigned y = 0; y < CHARACTER_HEIGHT; ++y)
        for (unsigned x = 0; x < CHARACTER_WIDTH; ++x)
            HUD_MAP[(CHARACTER_Y + y) * MAP_STRIDE + CHARACTER_X + x] = blank;
}

static void draw_hud(unsigned spent, unsigned move_limit)
{
    static const char text[] = " MOVE 00/00 FT";
    for (unsigned x = 0; text[x]; ++x)
        hud_letter(x, text[x]);
    hud_number(6, spent);
    hud_number(9, move_limit);
    hud_letter(14, ' ');
    hud_notice("");
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

#include "../build/closeup-art.h"

enum { TERRAIN_BACKDROP_BITS = 8, TERRAIN_PRIORITY_SHIFT = 24 };

static unsigned terrain_art(char cell)
{
    switch (cell) {
#define TERRAIN(symbol, tile, scene, priority) \
    case symbol: return (tile) | (SCENE_##scene << 16) | ((priority) << TERRAIN_PRIORITY_SHIFT);
#include "battlefield-terrain.def"
#undef TERRAIN
    default: return 0;  /* The art generator rejects undeclared map symbols. */
    }
}

static void cache_square(unsigned map, unsigned x, unsigned y)
{
    unsigned tile = terrain_art(maps[map][y][x]) & 0xffff;

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

static void load_starting_view(unsigned map, int view_height)
{
    for (int y = 0; y < view_height; ++y)
        for (unsigned x = 0; x < VIEW_WIDTH; ++x)
            cache_square(map, x, y);
}

static void stream_edges(unsigned map, int old_x, int old_y, int x, int y, int view_height)
{
    if (x != old_x) {
        int column = x > old_x ? x + VIEW_WIDTH - 1 : x;
        for (int row = y; row < y + view_height; ++row)
            cache_square(map, column, row);
    }
    if (y != old_y) {
        int row = y > old_y ? y + view_height - 1 : y;
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
    /* Negative scroll puts world row zero below the opaque HUD. */
    REG16(0x04000012) = (unsigned)camera_y & (CACHE_SIZE * CELL_SIZE - 1);
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

static unsigned breakable_slot(unsigned index)
{
    return index < 3 ? index + 1 : EXTRA_BREAKABLE_SLOT + index - 3;
}

static void show_breakables(int camera_x, int camera_y, unsigned moving, unsigned broken)
{
    for (unsigned i = 0; i < BREAKABLE_COUNT; ++i) {
        const Breakable *object = &breakables[i];
        volatile uint16_t *sprite = OAM + 4 * breakable_slot(i);
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

/* First bow trial uses tile-center sight lines and 5-foot diagonal squares. */
static unsigned square_distance(int x, int y, int to_x, int to_y)
{
    unsigned dx = x < to_x ? to_x - x : x - to_x;
    unsigned dy = y < to_y ? to_y - y : y - to_y;
    return 5 * (dx > dy ? dx : dy);
}

static unsigned bow_distance(unsigned index, int x, int y)
{
    return square_distance(x, y, breakables[index].x, breakables[index].y);
}

static unsigned clear_bow_line(int x, int y, int tx, int ty)
{
    int dx = tx > x ? tx - x : x - tx, dy = ty > y ? ty - y : y - ty;
    int error = dx - dy;
    while (x != tx || y != ty) {
        int twice = 2 * error;
        if (twice > -dy) { error -= dy; x += x < tx ? 1 : -1; }
        if (twice < dx) { error += dx; y += y < ty ? 1 : -1; }
        char terrain = maps[RIVER_MAP][y][x];
        if (terrain == '#' || terrain == 'T')
            return 0;
    }
    return 1;
}

static unsigned can_target(unsigned index, int x, int y, unsigned broken, unsigned bow)
{
    if (index >= BREAKABLE_COUNT)
        return 0;
    if (!bow)
        return target_in_reach(index, x, y, broken);
    return !(broken & (1u << index)) && bow_distance(index, x, y) <= LONGBOW_LONG_FT &&
           clear_bow_line(x, y, breakables[index].x, breakables[index].y);
}

static unsigned square_in_attack_range(int x, int y, int tx, int ty, unsigned bow)
{
    unsigned distance = square_distance(x, y, tx, ty);
    if (!distance || maps[RIVER_MAP][ty][tx] == '#' || maps[RIVER_MAP][ty][tx] == 'T')
        return 0;
    if (!bow)
        return distance == 5;
    return distance <= LONGBOW_LONG_FT && clear_bow_line(x, y, tx, ty);
}

static void draw_range(int x, int y, int scroll_x, int scroll_y, unsigned bow)
{
    int left = scroll_x / CELL_SIZE;
    int top = scroll_y / CELL_SIZE;
    unsigned blank = HUD_TILE_BASE + sizeof hud_glyphs / sizeof hud_glyphs[0];
    for (unsigned row = 0; row < 32; ++row)
        for (unsigned col = 0; col < 32; ++col)
            RANGE_MAP[row * MAP_STRIDE + col] = blank;
    for (int row = 0; row < 11 && top + row < MAP_HEIGHT; ++row)
        for (int col = 0; col < 16 && left + col < MAP_WIDTH; ++col) {
            int tx = left + col;
            int ty = top + row;
            if (!square_in_attack_range(x, y, tx, ty, bow))
                continue;
            unsigned entry = 2 * row * MAP_STRIDE + 2 * col;
            unsigned tile = terrain_art(maps[RIVER_MAP][ty][tx]) & 0xffff;
            RANGE_MAP[entry] = tile;
            RANGE_MAP[entry + 1] = tile + 1;
            RANGE_MAP[entry + MAP_STRIDE] = tile + 2;
            RANGE_MAP[entry + MAP_STRIDE + 1] = tile + 3;
        }
    REG16(0x04000018) = (scroll_x - left * CELL_SIZE) & 511;
    REG16(0x0400001a) = (scroll_y - top * CELL_SIZE) & 511;
}

static unsigned cycle_target(unsigned selected, int direction, int x, int y, unsigned broken, unsigned bow)
{
    for (unsigned count = 0; count < BREAKABLE_COUNT; ++count) {
        int next = (int)selected + direction;
        if (next < 0)
            next = BREAKABLE_COUNT - 1;
        if (next == BREAKABLE_COUNT)
            next = 0;
        selected = next;
        if (can_target(selected, x, y, broken, bow))
            return selected;
    }
    return BREAKABLE_COUNT;
}

static unsigned nearest_breakable(int x, int y, unsigned broken, unsigned bow)
{
    unsigned target = BREAKABLE_COUNT;
    int best = MAP_WIDTH + MAP_HEIGHT;
    for (unsigned i = 0; i < BREAKABLE_COUNT; ++i) {
        int dx = x - breakables[i].x;
        int dy = y - breakables[i].y;
        if (!can_target(i, x, y, broken, bow))
            continue;
        int distance = (dx < 0 ? -dx : dx) + (dy < 0 ? -dy : dy);
        if (distance < best) {
            best = distance;
            target = i;
        }
    }
    return target;
}

static void load_turn_banner(void)
{
    static const uint16_t colors[] = {
        RGB5(2, 3, 8), RGB5(24, 15, 6), RGB5(31, 24, 13),
        RGB5(5, 10, 23), RGB5(3, 7, 18), RGB5(31, 30, 24), RGB5(31, 26, 14),
    };
    for (unsigned i = 0; i < sizeof colors / sizeof colors[0]; ++i)
        OBJ_PALETTE[16 * TURN_BANNER_PALETTE + 1 + i] = colors[i];
    /* DMA3 copies the generated tiles during startup forced blank. */
    REG32(0x040000d4) = (uintptr_t)turn_banner_tiles;
    REG32(0x040000d8) = (uintptr_t)(OBJ_TILES + TURN_BANNER_TILE * 16);
    REG32(0x040000dc) = (0x8000u << 16) | (sizeof turn_banner_tiles / sizeof(uint16_t));
}

static void show_turn_banner(unsigned remaining)
{
    unsigned age = TURN_CUE_FRAMES - remaining;
    unsigned slide = age > TURN_HOLD_FRAMES ? age - TURN_HOLD_FRAMES : 0;
    for (unsigned half = 0; half < 2; ++half) {
        volatile uint16_t *sprite = OAM + (TURN_BANNER_SLOT + half) * 4;
        unsigned x = 56 + 64 * half + 2 * slide * slide;
        if (!remaining || x >= VIEW_WIDTH * CELL_SIZE) {
            sprite[0] = OBJ_DISABLED;
            continue;
        }
        sprite[0] = (1 << 14) | (64 - slide); /* Wide sprite. */
        sprite[1] = (3 << 14) | x;            /* 64 by 32. */
        sprite[2] = (TURN_BANNER_PALETTE << 12) | (TURN_BANNER_TILE + 32 * half);
    }
}

static unsigned roll_die(uint32_t frame, unsigned sides)
{
    /* Input timing supplies the seed. Reject the incomplete final group. */
    uint32_t seed = frame * 2654435761u;
    unsigned value;
    unsigned ceiling = 256;
    while (ceiling >= sides)
        ceiling -= sides;
    ceiling = 256 - ceiling;
    do {
        seed = seed * 1664525u + 1013904223u;
        value = seed >> 24;
    } while (value >= ceiling);
    while (value >= sides)
        value -= sides;
    return value + 1;
}

static unsigned arrow_pixel(unsigned x, unsigned y)
{
    if (y == 7)
        return x < 3 ? 1 : 2;
    return (y == 6 || y == 8) && (x == 2 || x >= 13) ? 1 : 0;
}

static unsigned scene_spark(unsigned x, unsigned y)
{
    if ((x == 7 && y >= 2 && y <= 13) || (y == 7 && x >= 2 && x <= 13))
        return 1;
    if ((x == y || x + y == 14) && x >= 4 && x <= 10)
        return 1;
    return 0;
}

#ifdef LOCAL_BATTLE_AUDIO
/* DMA1 reads internal memory. Stage the selected cue in EWRAM before playback. */
__asm__(".pushsection .rodata\n.balign 4\n"
        ".global battle_pcm, battle_pcm_end\n"
        "battle_pcm: .incbin \"build/preview-battle.pcm\"\n"
        "battle_pcm_end:\n.popsection\n");
extern const uint32_t battle_pcm[], battle_pcm_end[];

#define BATTLE_PCM_EWRAM ((volatile uint32_t *)0x02000000)
#define PREVIEW_AUDIO_FRAMES 1792
#define AUDIO_FADE_FRAMES 14

#ifdef LOCAL_MAP_AUDIO
__asm__(".pushsection .rodata\n.balign 4\n"
        ".global map_pcm, map_pcm_end\n"
        "map_pcm: .incbin \"build/preview-map.pcm\"\n"
        "map_pcm_end:\n.popsection\n");
extern const uint32_t map_pcm[], map_pcm_end[];
#endif

static void stop_battle_audio(void)
{
    REG16(0x04000102) = 0;      /* Timer 0. */
    REG16(0x04000106) = 0;      /* Timer 1 sample counter. */
    REG16(0x040000c6) = 0;      /* DMA 1. */
    REG16(0x04000082) = (1 << 11) | 1; /* Clear FIFO A, keep PSG at half volume. */
}

static void play_staged_audio(void)
{
    stop_battle_audio();
    REG16(0x04000084) = 0x80;
    REG16(0x04000082) = 0x0b05; /* FIFO A at full volume, PSG at half volume. */
    REG32(0x040000bc) = (uintptr_t)BATTLE_PCM_EWRAM;
    REG32(0x040000c0) = 0x040000a0;
    REG16(0x040000c6) = 0xb640; /* FIFO request, 32-bit repeat, fixed destination. */
    REG16(0x04000100) = -2048;  /* 8,192 Hz. */
    REG16(0x04000104) = 0;
    REG16(0x04000106) = 0x84;  /* Count Timer 0 overflows. */
    REG16(0x04000102) = 0x80;
}
static void stage_audio(const uint32_t *start, const uint32_t *end)
{
    stop_battle_audio();
    REG32(0x040000d4) = (uintptr_t)start;
    REG32(0x040000d8) = (uintptr_t)BATTLE_PCM_EWRAM;
    unsigned words = ((uintptr_t)end - (uintptr_t)start) / sizeof *start;
    REG32(0x040000dc) = (0x8400u << 16) | words; /* DMA3, immediate 32-bit copy. */
    play_staged_audio();
}

/* Leave a lead beyond queued samples and taper the remaining PCM in place. */
static void fade_audio(unsigned elapsed, unsigned duration)
{
    enum { FADE_SAMPLES = 1536, FIFO_LEAD = 192 };
    unsigned estimate = elapsed * 137;
    unsigned cursor = (estimate & ~65535u) | REG16(0x04000104);
    if (cursor + 32768u < estimate)
        cursor += 65536u;
    else if (cursor > estimate + 32768u)
        cursor -= 65536u;

    unsigned limit = duration * 137;
    volatile int8_t *pcm = (volatile int8_t *)BATTLE_PCM_EWRAM;
    for (unsigned i = 0; i < AUDIO_FADE_FRAMES * 137 && cursor + FIFO_LEAD + i < limit; ++i) {
        unsigned sample = cursor + FIFO_LEAD + i;
        pcm[sample] = i < FADE_SAMPLES
            ? (int)pcm[sample] * (int)(FADE_SAMPLES - i) / FADE_SAMPLES : 0;
    }
}

#endif

static unsigned closeup_terrain(char actor, char target)
{
    unsigned near = terrain_art(actor);
    unsigned far = terrain_art(target);
    unsigned scene = (far >> TERRAIN_PRIORITY_SHIFT) > (near >> TERRAIN_PRIORITY_SHIFT) ? far : near;
    return (scene >> 16) & ((1u << TERRAIN_BACKDROP_BITS) - 1);
}

static void scene_text(unsigned x, unsigned y, const char *text, unsigned palette)
{
    while (*text)
        SCENE_MAP[y * MAP_STRIDE + x++] = (palette << 12) | (HUD_TILE_BASE + hud_glyph(*text++));
}

static void prepare_closeup(unsigned terrain, const Breakable *target, const Character *character)
{
    static const uint16_t frame[] = {
        RGB5(2, 5, 15), RGB5(31, 30, 23), RGB5(13, 7, 3),
        RGB5(23, 15, 6), RGB5(31, 24, 12)
    };
    for (unsigned i = 0; i < sizeof frame / sizeof frame[0]; ++i) {
        BG_PALETTE[16 * SCENE_DIALOG + 1 + i] = frame[i];
        BG_PALETTE[16 * SCENE_TITLE + 1 + i] = frame[i];
    }
    BG_PALETTE[16 * SCENE_TITLE + 2] = RGB5(31, 25, 13);
    for (unsigned i = 0; i < 16; ++i)
        BG_PALETTE[16 * SCENE_PALETTE + i] = closeup_art[terrain].palette[i];

    volatile uint16_t *tiles = BG_TILES + SCENE_TILE * 16;
    for (unsigned tile = 0; tile < 420; ++tile)
        for (unsigned word = 0; word < 16; ++word)
            *tiles++ = closeup_art[terrain].tiles[tile][word];

    for (unsigned y = 0; y < 20; ++y) {
        for (unsigned x = 0; x < 30; ++x) {
            unsigned tile = HUD_TILE_BASE;
            unsigned palette = SCENE_DIALOG;
            if (y >= 2 && y < 16) {
                tile = SCENE_TILE + (y - 2) * 30 + x;
                palette = SCENE_PALETTE;
            } else if (y >= 16) {
                unsigned row = y == 16 ? 0 : y == 19 ? 2 : 1;
                unsigned col = x == 0 ? 0 : x == 29 ? 2 : 1;
                tile = CHARACTER_FRAME_TILE + 3 * row + col;
            }
            SCENE_MAP[y * MAP_STRIDE + x] = (palette << 12) | tile;
        }
    }
    scene_text(1, 0, character->name, SCENE_TITLE);
    unsigned bow = character->equipment.weapon == ITEM_LONGBOW;
    scene_text(1, 1, bow ? "LONGBOW ATTACK" : "STRENGTH ATHLETICS", SCENE_DIALOG);
    scene_text(1, 17, bow ? "ALDEN DRAWS" : "ALDEN BRACES", SCENE_TITLE);
    scene_text(1, 18, bow ? "SHOOT" : "SMASH", SCENE_DIALOG);
    scene_text(8, 18, target->name, SCENE_TITLE);
    write_cell(OBJ_TILES + SPARK_TILE * 16, scene_spark);
    write_cell(OBJ_TILES + ARROW_TILE * 16, arrow_pixel);
    OBJ_PALETTE[3 * 16 + 1] = RGB5(31, 22, 10);
    const Loadout *equipment = &character->equipment;
    const uint16_t (*poses)[16][16] =
        closeup_fighter_tiles[bow ? 2 : equipment->weapon == ITEM_WARHAMMER]
                             [equipment->armor == ITEM_CHAIN_SHIRT][!!equipment->shield];
    for (unsigned pose = 0; pose < 3; ++pose)
        for (unsigned tile = 0; tile < 16; ++tile)
            for (unsigned word = 0; word < 16; ++word)
                OBJ_TILES[(CLOSEUP_READY_TILE + pose * 16 + tile) * 16 + word] =
                    poses[pose][tile][word];
    for (unsigned i = 0; i <= TARGET_OAM / 4; ++i)
        OAM[4 * i] = OBJ_DISABLED;

    for (unsigned i = 3; i < BREAKABLE_COUNT; ++i)
        OAM[4 * breakable_slot(i)] = OBJ_DISABLED;

    /* Matrix 0 enlarges Alden, while matrix 1 keeps the far target smaller. */
    OAM[3] = 181;
    OAM[7] = 0;
    OAM[11] = 0;
    OAM[15] = 181;
    OAM[19] = 139;
    OAM[23] = 0;
    OAM[27] = 0;
    OAM[31] = 139;
}

static void scene_sprite(unsigned slot, int x, int y, unsigned tile, unsigned palette,
                         unsigned matrix, unsigned large)
{
    volatile uint16_t *sprite = OAM + slot * 4;
    sprite[0] = (y & 255) | (3 << 8);
    sprite[1] = (large ? (2 << 14) : OBJ_SIZE_16) | (matrix << 9) | (x & 511);
    sprite[2] = (palette << 12) | tile;
}

static void animate_closeup(unsigned age, const Breakable *target, unsigned success, unsigned hit, unsigned bow)
{
    int hero_x = 174;
    int hero_y = 92;
    int target_x = 66;
    unsigned pose = CLOSEUP_READY_TILE;
    if (age < SCENE_IMPACT) {
        int step = age > SCENE_ENTRANCE ? age - SCENE_ENTRANCE : 0;
        hero_x -= 8 * step;
        hero_y -= 3 * step;
        pose = step < 3 ? CLOSEUP_BRACE_TILE : CLOSEUP_STRIKE_TILE;
    } else if (age < SCENE_RESULT) {
        int recoil = age - SCENE_IMPACT;
        hero_x = 106 + 7 * recoil;
        hero_y = 68 + 2 * recoil;
        if (!success && recoil >= 2 && recoil <= 7) {
            hero_x += 10;
            hero_y -= 3;
        }
        pose = success ? CLOSEUP_STRIKE_TILE : CLOSEUP_BRACE_TILE;
        if (age < SCENE_IMPACT + 5)
            target_x -= (age & 1) ? 3 : 0;
    } else {
        pose = success ? CLOSEUP_READY_TILE : CLOSEUP_BRACE_TILE;
    }

    if (bow) {
        hero_x = 174;
        hero_y = 92;
        pose = age < 16 ? CLOSEUP_BRACE_TILE :
               age < SCENE_IMPACT + 4 ? CLOSEUP_STRIKE_TILE : CLOSEUP_READY_TILE;
        if (!hit)
            target_x = 66;
    }
    if (age < SCENE_ENTRANCE) {
        int remaining = SCENE_ENTRANCE - age;
        hero_x += remaining * remaining / 2;
        target_x -= remaining * remaining / 2;
    }
    scene_sprite(0, hero_x - 32, hero_y - 32, pose, 1, 0, 1);
    scene_sprite(1, target_x - 16, 69 - 16,
                 success && age >= SCENE_IMPACT ? BROKEN_FENCE_TILE : target->tile, 2, 1, 0);
    if (bow && age >= 16 && age < SCENE_IMPACT) {
        int step = age - 16;
        scene_sprite(2, 137 - 10 * step, 74 - 4 * step, ARROW_TILE, 0, 1, 0);
    } else if ((!bow || hit) && age >= SCENE_IMPACT && age < SCENE_IMPACT + (success ? 7 : 4)) {
        unsigned spread = age - SCENE_IMPACT;
        scene_sprite(2, 77 - 16 + spread, 64 - 16 - spread, SPARK_TILE, success ? 3 : 0, 1, 0);
    } else {
        OAM[8] = OBJ_DISABLED;
    }

    if (age == SCENE_IMPACT || age == SCENE_RESULT) {
        scene_text(1, 17, success ? "WOOD SPLINTERS" : bow ? (hit ? "ARROW STRIKES" : "ARROW MISSES") : "THE WOOD HOLDS", SCENE_TITLE);
    }
    if (age == SCENE_RESULT) {
        for (unsigned x = 1; x < 29; ++x)
            SCENE_MAP[18 * MAP_STRIDE + x] =
                (SCENE_DIALOG << 12) | (HUD_MAP[ROLL_ROW * MAP_STRIDE + x] & 1023);
        scene_text(22, 17, "X NEXT", SCENE_DIALOG);
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

typedef enum {
    GEAR_FREE_INTERACTION,
    GEAR_UTILIZE,
    GEAR_SELECTION
} GearCost;

typedef struct {
    unsigned item;
    unsigned putting_on;
    GearCost cost;
    const char *blocked;
    unsigned followup_weapon;
    unsigned uses_free_interaction;
} GearChange;

static GearChange plan_gear_change(ActionMenu command, const Loadout *equipment,
                                   unsigned interact_spent, unsigned action_spent)
{
    if (command == EQUIP_MENU_BOW) {
        if (equipment->weapon == ITEM_LONGBOW)
            return (GearChange){0, 0, GEAR_FREE_INTERACTION, "BOW EQUIPPED", 0, 0};
        if (equipment->shield && !action_spent && !interact_spent)
            return (GearChange){
                .item = ITEM_SHIELD, .cost = GEAR_UTILIZE,
                .followup_weapon = equipment->weapon ? ITEM_NONE : ITEM_LONGBOW,
                .uses_free_interaction = 1,
            };
        if (equipment->shield)
            command = EQUIP_MENU_SHIELD;
    }
    if (command == EQUIP_MENU_SHIELD) {
        if (action_spent || equipment->weapon == ITEM_LONGBOW)
            return (GearChange){0, 0, GEAR_FREE_INTERACTION, action_spent ? "ACTION SPENT" : "STOW BOW FIRST", 0, 0};
        return (GearChange){ITEM_SHIELD, !equipment->shield, GEAR_UTILIZE, 0, 0, 0};
    }
    if (command == EQUIP_MENU_WEAPON && equipment->weapon == ITEM_LONGBOW &&
        !interact_spent && !action_spent)
        return (GearChange){ITEM_LONGBOW, 0, GEAR_UTILIZE, 0, ITEM_WARHAMMER, 1};
    if (interact_spent && action_spent)
        return (GearChange){0, 0, GEAR_FREE_INTERACTION, "END TURN FIRST", 0, 0};
    unsigned item = equipment->weapon ? equipment->weapon :
                    command == EQUIP_MENU_BOW ? ITEM_LONGBOW : ITEM_WARHAMMER;
    return (GearChange){item, !equipment->weapon,
                        interact_spent ? GEAR_UTILIZE : GEAR_FREE_INTERACTION, 0, 0, 0};
}

/* A free stow can accompany Attack, whose weapon interaction draws the bow. */
static unsigned can_draw_and_shoot(const Loadout *gear, unsigned interaction, unsigned action)
{
    return !gear->shield && !action && gear->weapon != ITEM_LONGBOW &&
           (!gear->weapon || !interaction);
}

static void apply_gear_change(Loadout *equipment, GearChange change,
                              unsigned *interact_spent, unsigned *action_spent)
{
    if (change.blocked)
        return;
    unsigned equipped = change.putting_on ? change.item : ITEM_NONE;
    if (change.item == ITEM_SHIELD)
        equipment->shield = equipped;
    else
        equipment->weapon = equipped;
    if (change.cost == GEAR_UTILIZE)
        *action_spent = 1;
    else
        *interact_spent = 1;
    if (change.uses_free_interaction) {
        equipment->weapon = change.followup_weapon;
        *interact_spent = 1;
    }
}

typedef struct {
    unsigned frames, item;
    unsigned putting_on;
    GearCost cost;
} GearCue;

static unsigned gear_icon_pixel(unsigned item, unsigned x, unsigned y)
{
    if (item == ITEM_SHIELD) {
        unsigned edge = y < 9 ? 3 : (y - 9) / 2 + 3;
        if (y < 3 || y > 13 || x < edge || x >= 16 - edge)
            return 0;
        return x == edge || x == 15 - edge || y == 3 || y == 13 ? 2 :
               x == 7 || x == 8 || y == 7 ? 3 : 1;
    }
    if (item == ITEM_LONGBOW) {
        unsigned arc = y < 8 ? y : 15 - y;
        if (y >= 1 && y <= 14 && x == 5 + arc)
            return 2;
        return x == 6 && y > 1 && y < 14 ? 3 : 0;
    }
    if (y >= 3 && y <= 6 && x >= 3 && x <= 12)
        return y == 3 || x == 3 ? 3 : 2;
    return x >= 7 && x <= 8 && y >= 7 && y <= 13 ? 2 : 0;
}

static unsigned hammer_cue_pixel(unsigned x, unsigned y)
{
    return gear_icon_pixel(ITEM_WARHAMMER, x, y);
}

static unsigned bow_cue_pixel(unsigned x, unsigned y)
{
    return gear_icon_pixel(ITEM_LONGBOW, x, y);
}

static unsigned shield_cue_pixel(unsigned x, unsigned y)
{
    return gear_icon_pixel(ITEM_SHIELD, x, y);
}

static void show_gear_cue(GearCue *cue, int x, int y)
{
    volatile uint16_t *sprite = OAM + GEAR_CUE_SLOT * 4;
    if (!cue->frames || --cue->frames == 0) {
        sprite[0] = OBJ_DISABLED;
        return;
    }
    unsigned elapsed = GEAR_CUE_FRAMES - cue->frames;
    int lift = cue->putting_on ? (int)(GEAR_CUE_FRAMES - elapsed) / 2 : (int)elapsed / 2;
    int icon_y = y - 12 - lift;
    if (icon_y < HUD_HEIGHT)
        icon_y = y + 9 + lift / 2;
    if (x <= -16 || x >= VIEW_WIDTH * CELL_SIZE || icon_y < HUD_HEIGHT || icon_y >= VISIBLE_LINES) {
        sprite[0] = OBJ_DISABLED;
        return;
    }
    unsigned tile = cue->item == ITEM_SHIELD ? 8 : cue->item == ITEM_LONGBOW ? 4 : 0;
    sprite[0] = icon_y;
    sprite[1] = OBJ_SIZE_16 | (x & 511);
    sprite[2] = (GEAR_CUE_PALETTE << 12) | (GEAR_CUE_TILE + tile);
}

static const char *gear_confirmation(const GearCue *cue)
{
    if (cue->item == ITEM_SHIELD)
        return cue->putting_on ? "SHIELD ON" : "SHIELD OFF";
    if (cue->item == ITEM_LONGBOW)
        return cue->putting_on ? "BOW DRAWN" : "BOW STOWED";
    return cue->putting_on ? "HAMMER DRAWN" : "HAMMER STOWED";
}

static void draw_gear_message(const GearCue *cue)
{
    if (cue->cost == GEAR_SELECTION) {
        for (unsigned x = 0; x < GEAR_MESSAGE_WIDTH; ++x)
            hud_letter(GEAR_MESSAGE_ROW * MAP_STRIDE + x,
                       x < sizeof "LONGBOW SELECTED" - 1 ? "LONGBOW SELECTED"[x] : ' ');
        return;
    }
    const char *cost = cue->cost == GEAR_FREE_INTERACTION ? "FREE INTERACTION " : "UTILIZE ";
    const char *result = gear_confirmation(cue);
    unsigned entry = GEAR_MESSAGE_ROW * MAP_STRIDE;
    for (unsigned x = 0; x < GEAR_MESSAGE_WIDTH; ++x) {
        char letter = *cost ? *cost++ : *result ? *result++ : ' ';
        hud_letter(entry + x, letter);
    }
}

static void clear_gear_message(unsigned *frames)
{
    if (*frames) {
        *frames = 0;
        unsigned blank = HUD_TILE_BASE + sizeof hud_glyphs / sizeof hud_glyphs[0];
        for (unsigned x = 0; x < GEAR_MESSAGE_WIDTH; ++x)
            HUD_MAP[GEAR_MESSAGE_ROW * MAP_STRIDE + x] = blank;
    }
}

static void refresh_equipment(const Loadout *equipment, unsigned *shoot_art_loaded,
                              uint16_t display)
{
    REG16(0x04000000) = FORCE_BLANK;
    if (equipment->weapon == ITEM_LONGBOW && !*shoot_art_loaded) {
        write_card(SHOOT_CARD_TILE, bow_icon, "SHOOT");
        *shoot_art_loaded = 1;
    }
    OBJ_PALETTE[16 * GEAR_CUE_PALETTE + 1] = RGB5(13, 23, 30);
    OBJ_PALETTE[16 * GEAR_CUE_PALETTE + 2] = RGB5(31, 23, 10);
    OBJ_PALETTE[16 * GEAR_CUE_PALETTE + 3] = RGB5(31, 30, 23);
    write_cell(OBJ_TILES + GEAR_CUE_TILE * 16, hammer_cue_pixel);
    write_cell(OBJ_TILES + (GEAR_CUE_TILE + 4) * 16, bow_cue_pixel);
    write_cell(OBJ_TILES + (GEAR_CUE_TILE + 8) * 16, shield_cue_pixel);
    load_map_fighter(equipment);
    if (equipment->weapon == ITEM_LONGBOW)
        load_swimming_bow();
    wait_vblank();
    REG16(0x04000000) = display | BG1_ENABLE;
}

static void draw_items(ActionMenu menu, const Loadout *equipment, unsigned used, unsigned action_spent)
{
    static const char *const rows[] = {"HAMMER", "LONGBOW", "SHIELD", "BACK"};
    const char *hint = "";
    if (menu != EQUIP_MENU_BACK) {
        GearChange change = plan_gear_change(menu, equipment, used, action_spent);
        hint = change.blocked ? change.blocked :
               change.item == ITEM_SHIELD ?
                   (change.uses_free_interaction ?
                        (change.followup_weapon ? "DOFF AND DRAW" : "STOW AND DOFF") :
                    change.putting_on ? "UTILIZE DON" : "UTILIZE DOFF") :
               change.followup_weapon == ITEM_WARHAMMER ? "SWAP USES ACTION" :
               change.cost == GEAR_UTILIZE ? (change.putting_on ? "UTILIZE DRAW" : "UTILIZE STOW") :
               !change.putting_on ? "FREE STOW" :
               change.item == ITEM_LONGBOW ? "FREE DRAW BOW" : "DRAW HAMMER";
    }

    draw_panel(ITEMS_X, ITEMS_Y, ITEMS_WIDTH, ITEMS_HEIGHT, OBJECT_PALETTE);
    panel_text(ITEMS_X + 2, ITEMS_Y + 1, "ITEMS", OBJECT_TITLE_PALETTE);
    for (unsigned i = 0; i < sizeof rows / sizeof rows[0]; ++i) {
        unsigned focused = menu == EQUIP_MENU_WEAPON + i;
        unsigned palette = focused ? OBJECT_TITLE_PALETTE : OBJECT_PALETTE;
        if (focused)
            panel_text(ITEMS_X + 1, ITEMS_Y + 2 + i, ">", palette);
        panel_text(ITEMS_X + 2, ITEMS_Y + 2 + i, rows[i], palette);
        if (i < 3) {
            unsigned equipped = i == 0 ? equipment->weapon == ITEM_WARHAMMER :
                                i == 1 ? equipment->weapon == ITEM_LONGBOW : equipment->shield != ITEM_NONE;
            panel_text(ITEMS_X + 10, ITEMS_Y + 2 + i, equipped ? "ON" : "STOWED", OBJECT_PALETTE);
        }
    }
    panel_text(ITEMS_X + 1, ITEMS_Y + 6, hint, OBJECT_PALETTE);
}

static void draw_bow_confirmation(const Loadout *gear, GearChange change)
{
    draw_panel(ITEMS_X, ITEMS_Y, ITEMS_WIDTH, ITEMS_HEIGHT, OBJECT_PALETTE);
    panel_text(ITEMS_X + 2, ITEMS_Y + 1, "SWITCH TO BOW", OBJECT_TITLE_PALETTE);
    panel_text(ITEMS_X + 1, ITEMS_Y + 2,
               change.uses_free_interaction && gear->weapon ? "STOW HAMMER FREE" :
               change.item == ITEM_WARHAMMER ? "STOW HAMMER" : "REMOVE SHIELD", OBJECT_PALETTE);
    if (change.item == ITEM_SHIELD && change.uses_free_interaction && gear->weapon)
        panel_text(ITEMS_X + 1, ITEMS_Y + 3, "REMOVE SHIELD", OBJECT_PALETTE);
    panel_text(ITEMS_X + 1, ITEMS_Y + 4,
               change.cost == GEAR_UTILIZE ? "USES ACTION" : "FREE INTERACTION", OBJECT_PALETTE);
    panel_text(ITEMS_X + 1, ITEMS_Y + 5,
               change.cost == GEAR_UTILIZE ? "SHOOT NEXT TURN" : "DRAW WITH ATTACK", OBJECT_PALETTE);
    panel_text(ITEMS_X + 1, ITEMS_Y + 6, "X CONFIRM Z BACK", OBJECT_TITLE_PALETTE);
}

static void clear_items(void)
{
    unsigned blank = HUD_TILE_BASE + sizeof hud_glyphs / sizeof hud_glyphs[0];
    for (unsigned y = ITEMS_Y; y < ITEMS_Y + ITEMS_HEIGHT; ++y)
        for (unsigned x = ITEMS_X; x < ITEMS_X + ITEMS_WIDTH; ++x)
            HUD_MAP[y * MAP_STRIDE + x] = blank;
}

int main(void)
{
    Character character = hero;
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
    int scroll_y = moving ? -HUD_HEIGHT : 0;
    unsigned spent = 0;
    unsigned move_limit = character.speed;
    unsigned animation_frame = 0;
    unsigned step_frames = 0;
    unsigned swim_speed = 0;
    unsigned previous = 0;
    unsigned broken_objects = 0;
    ActionMenu menu = ACTION_MENU_CLOSED;
    unsigned selected = BREAKABLE_COUNT;
    unsigned object_details = 0;
    unsigned scene_age = 0;
#ifdef LOCAL_BATTLE_AUDIO
    unsigned audio_frames = 0;
    unsigned map_fade = 0;
    unsigned battle_fade = 0;

#endif
    unsigned character_open = 0;
    uint16_t saved_status[VIEW_WIDTH * 2];
    unsigned character_art_loaded = 0;
    unsigned scene_success = 0;
    unsigned scene_hit = 0;
    unsigned bow = 0;
    unsigned bow_selected = 0;
    unsigned range_visible = 0;
    unsigned range_ranged = 0;
    unsigned range_phase = 0;
    unsigned confirm_bow = 0;
    unsigned shoot_art_loaded = 0;
    unsigned arrows = ARROW_PACK;
    unsigned object_hp[BREAKABLE_COUNT];
    for (unsigned i = 0; i < BREAKABLE_COUNT; ++i)
        object_hp[i] = breakables[i].hp;
    unsigned interact_spent = 0;
    GearCue gear_cue;
    gear_cue.frames = 0;
    gear_cue.item = ITEM_NONE;
    gear_cue.putting_on = 0;
    unsigned gear_message_frames = 0;
    unsigned turn_cue = 0;
    unsigned action_spent = 0;
    unsigned bonus_spent = 0;
    unsigned repeat_timer = REPEAT_DELAY;
    const uint16_t display = BG0_ENABLE | OBJ_ENABLE | OBJ_1D;

    REG16(0x04000000) = FORCE_BLANK;
    REG16(0x04000008) = BG_SIZE_512 | (SCREEN_BLOCK << 8) | 2;
    REG16(0x0400000a) = HUD_SCREEN_BLOCK << 8;
    REG16(0x0400000c) = (RANGE_BLOCK << 8) | 1;
    REG16(0x04000050) = 0;
    REG16(0x04000014) = 0;
    REG16(0x04000016) = 0;
    load_tiles();
    load_hud_font();
    load_menu_cards();
    load_panel_frame();
    load_starting_view(map, VIEW_HEIGHT);

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
    load_map_fighter(&character.equipment);
    load_turn_banner();
    REG32(0x040000d4) = (uintptr_t)items_card_tiles;
    REG32(0x040000d8) = (uintptr_t)(BG_TILES + ITEMS_CARD_TILE * 16);
    REG32(0x040000dc) = (0x8000u << 16) | (sizeof items_card_tiles / sizeof(uint16_t));
    OBJ_PALETTE[33] = RGB5(8, 4, 3);
    OBJ_PALETTE[34] = RGB5(21, 12, 6);
    OBJ_PALETTE[35] = RGB5(27, 18, 9);
    write_cell(OBJ_TILES + FENCE_TILE * 16, fence);
    write_cell(OBJ_TILES + BROKEN_FENCE_TILE * 16, broken_fence);
    write_cell(OBJ_TILES + CRATE_TILE * 16, crate);
    write_cell(OBJ_TILES + BARREL_TILE * 16, barrel);
    write_cell(OBJ_TILES + PALLET_TILE * 16, pallet);
    write_cell(OBJ_TILES + BASKET_TILE * 16, basket);
    write_cell(OBJ_TILES + STOOL_TILE * 16, stool);
    write_cell(OBJ_TILES + DUMMY_TILE * 16, dummy);
    write_cell(OBJ_TILES + TARGET_TILE * 16, archery_target);
    show_view(sprite_x, sprite_y, scroll_x, scroll_y);
    show_breakables(scroll_x, scroll_y, moving, broken_objects);
    show_pose(x, y, moving, animation_frame, step_frames);
    if (moving)
        draw_hud(spent, move_limit);
    wait_vblank();
    REG16(0x04000000) = display | (moving ? BG1_ENABLE : 0);

#ifdef LOCAL_MAP_AUDIO
    stage_audio(map_pcm, map_pcm_end);
    audio_frames = PREVIEW_AUDIO_FRAMES;
#endif

    for (;;) {
        wait_vblank();
#ifdef LOCAL_BATTLE_AUDIO
        if (map_fade && --map_fade == 0) {
#ifdef LOCAL_MAP_AUDIO
            stage_audio(map_pcm, map_pcm_end);
            audio_frames = PREVIEW_AUDIO_FRAMES;
#else
            stop_battle_audio();
            audio_frames = 0;
#endif
        } else if (battle_fade && --battle_fade == 0) {
            stage_audio(battle_pcm, battle_pcm_end);
            audio_frames = PREVIEW_AUDIO_FRAMES;
        } else if (!map_fade && !battle_fade && audio_frames && --audio_frames == 0) {
#ifdef LOCAL_MAP_AUDIO
            if (!scene_age) {
                play_staged_audio();
                audio_frames = PREVIEW_AUDIO_FRAMES;
            } else
#endif
                stop_battle_audio();
        }
#endif
        if (turn_cue)
            show_turn_banner(--turn_cue);
        if (gear_message_frames == 1)
            clear_gear_message(&gear_message_frames);
        else if (gear_message_frames)
            --gear_message_frames;
        ++animation_frame;
        if (step_frames)
            --step_frames;
        if (moving && !scene_age) {
            sprite_x = approach(sprite_x, x * CELL_SIZE);
            sprite_y = approach(sprite_y, y * CELL_SIZE);
            scroll_x = approach(scroll_x, camera_x * CELL_SIZE);
            int target_scroll_y = camera_y * CELL_SIZE - HUD_HEIGHT;
            if (camera_y == MAP_HEIGHT - TRIAL_VIEW_HEIGHT)
                target_scroll_y = (MAP_HEIGHT - VIEW_HEIGHT) * CELL_SIZE;
            scroll_y = approach(scroll_y, target_scroll_y);
            show_view(sprite_x, sprite_y, scroll_x, scroll_y);
            show_breakables(scroll_x, scroll_y, moving, broken_objects);
            show_pose((sprite_x + 8) / CELL_SIZE, (sprite_y + 8) / CELL_SIZE,
                      moving, animation_frame, step_frames);
            if (menu != ACTION_MENU_CLOSED && !equip_submenu(menu) && selected < BREAKABLE_COUNT)
                show_target(&breakables[selected], scroll_x, scroll_y);
        }
        if (gear_cue.frames && moving && !scene_age && !character_open && menu == ACTION_MENU_CLOSED) {
            show_gear_cue(&gear_cue, sprite_x - scroll_x, sprite_y - scroll_y);
            if (!step_frames && gear_cue.frames >= GEAR_CUE_FRAMES - 7 &&
                maps[map][y][x] != '~')
                OAM[2] = (1 << 12) | (1 << 10) | IDLE_BREATH_TILE;
            if (!gear_cue.frames)
                hud_notice(bow_selected && !bow ? (action_spent ? "ACTION SPENT" : character.equipment.shield ? "SHIELD EQUIPPED" : "DRAW AND SHOOT") : "");
        }
        unsigned hammer_preview = menu == ACTION_MENU_CLOSED && !step_frames &&
                                  gear_message_frames && gear_cue.item == ITEM_WARHAMMER &&
                                  gear_cue.putting_on && character.equipment.weapon == ITEM_WARHAMMER;
        unsigned show_range = moving && !scene_age && !character_open &&
                              (hammer_preview ||
                               (action_submenu(menu) && (character.equipment.weapon || bow_selected))) &&
                              scroll_x == camera_x * CELL_SIZE &&
                              scroll_y == (camera_y == MAP_HEIGHT - TRIAL_VIEW_HEIGHT
                                  ? (MAP_HEIGHT - VIEW_HEIGHT) * CELL_SIZE
                                  : camera_y * CELL_SIZE - HUD_HEIGHT);
        unsigned ranged_overlay = !hammer_preview && (bow || bow_selected || menu == ACTION_MENU_DRAW_SHOOT);
        if (show_range != range_visible ||
            (show_range && (ranged_overlay != range_ranged || !(REG16(0x04000000) & BG2_ENABLE)))) {
            if (show_range) {
                REG16(0x04000000) = FORCE_BLANK;
                draw_range(x, y, scroll_x, scroll_y, ranged_overlay);
                range_phase = 0;
                REG16(0x04000050) = BRIGHTEN_BG2;
                REG16(0x04000000) = display | BG1_ENABLE | BG2_ENABLE;
            } else {
                REG16(0x04000000) &= ~BG2_ENABLE;
                REG16(0x04000050) = 0;
            }
            range_visible = show_range;
            range_ranged = ranged_overlay;
        }
        if (show_range) {
            static const uint8_t brightness[] = {1, 1, 2, 3, 3, 2, 1, 1};
            REG16(0x04000054) = brightness[range_phase / 6];
            if (++range_phase == 6 * sizeof brightness)
                range_phase = 0;
        }
        /* KEYINPUT is active low. Keep only buttons newly pressed this frame. */
        unsigned held = ~REG16(0x04000130) & (KEY_DPAD | KEY_A | KEY_B | KEY_SELECT | KEY_START | KEY_L);
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
            if (scene_age < SCENE_RESULT && (pressed & KEY_A)) {
                scene_age = SCENE_RESULT;
                animate_closeup(scene_age, &breakables[selected], scene_success, scene_hit, bow);
                continue;
            }
            if (scene_age <= SCENE_RESULT)
                ++scene_age;
            if (scene_age > SCENE_RESULT && (pressed & (KEY_A | KEY_B))) {
                scene_age = 0;
#ifdef LOCAL_BATTLE_AUDIO
                if (audio_frames > AUDIO_FADE_FRAMES) {
                    fade_audio(PREVIEW_AUDIO_FRAMES - audio_frames, PREVIEW_AUDIO_FRAMES);
                    map_fade = AUDIO_FADE_FRAMES;
                } else {
#ifdef LOCAL_MAP_AUDIO
                    stage_audio(map_pcm, map_pcm_end);
                    audio_frames = PREVIEW_AUDIO_FRAMES;
#else
                    stop_battle_audio();
                    audio_frames = 0;
#endif
                }
#endif
                REG16(0x04000000) = FORCE_BLANK;
                REG16(0x0400000a) = HUD_SCREEN_BLOCK << 8;
                OAM[8] = OBJ_DISABLED;
                show_view(sprite_x, sprite_y, scroll_x, scroll_y);
                show_breakables(scroll_x, scroll_y, moving, broken_objects);
                show_pose(x, y, moving, animation_frame, 0);
                REG16(0x04000000) = display | BG1_ENABLE;
            } else {
                animate_closeup(scene_age, &breakables[selected], scene_success, scene_hit, bow);
            }
            continue;
        }

        if (character_open) {
            if (pressed & (KEY_SELECT | KEY_RIGHT)) {
                if (pressed & KEY_RIGHT) {
                    character.equipment = hero.equipment;
                    character.equipment.weapon = ITEM_LONGBOW;
                    character.equipment.shield = ITEM_NONE;
                } else
                    character.equipment = character.equipment.armor == ITEM_NONE
                    ? hero.equipment : unequipped;
                bow = character.equipment.weapon == ITEM_LONGBOW;
                bow_selected = bow;
                hud_notice(bow ? "BOW 00 ARROWS" : "SEL GEAR");
                if (bow)
                    hud_number(19, arrows);
                REG16(0x04000000) = FORCE_BLANK;
                if (bow && !shoot_art_loaded) {
                    write_card(SHOOT_CARD_TILE, bow_icon, "SHOOT");
                    shoot_art_loaded = 1;
                }
                load_map_fighter(&character.equipment);
                if (bow)
                    load_swimming_bow();
                show_character(&character, swim_speed);
                wait_vblank();
                REG16(0x04000000) = display | BG1_ENABLE;
            }
            if (pressed & (KEY_START | KEY_B)) {
                clear_character();
                for (unsigned col = 0; col < VIEW_WIDTH * 2; ++col)
                    HUD_MAP[col] = saved_status[col];
                character_open = 0;
            }
            continue;
        }
        unsigned exit_trial = moving && (held & KEY_L) && (pressed & KEY_START);
        if (moving && menu == ACTION_MENU_CLOSED && (pressed & KEY_START) && !exit_trial) {
            clear_gear_message(&gear_message_frames);
            if (gear_cue.frames) {
                gear_cue.frames = 0;
                OAM[4 * GEAR_CUE_SLOT] = OBJ_DISABLED;
                hud_notice("");
            }
            if (turn_cue)
                show_turn_banner(turn_cue = 0);
            for (unsigned col = 0; col < VIEW_WIDTH * 2; ++col)
                saved_status[col] = HUD_MAP[col];
            hud_notice(bow ? "BOW 00 ARROWS" : "SEL GEAR");
            if (bow)
                hud_number(19, arrows);
            if (!character_art_loaded) {
                REG16(0x04000000) = FORCE_BLANK;
                load_character_art();
                character_art_loaded = 1;
                show_character(&character, swim_speed);
                wait_vblank();
                REG16(0x04000000) = display | BG1_ENABLE;
            } else {
                show_character(&character, swim_speed);
            }
            character_open = 1;
            continue;
        }

        if ((!moving && (pressed & KEY_A)) ||
            (!moving && map == RIVER_MAP && (pressed & KEY_START)) ||
            exit_trial) {
            if (!moving && (pressed & KEY_A)) {
                map = (map + 1 == MAP_COUNT) ? 0 : map + 1;
                moving = 0;
            } else {
                moving = !moving;
            }
            spent = 0;
            step_frames = 0;
            broken_objects = 0;
            clear_gear_message(&gear_message_frames);
            gear_cue.frames = 0;
            OAM[4 * GEAR_CUE_SLOT] = OBJ_DISABLED;
            interact_spent = 0;
            if (turn_cue)
                show_turn_banner(turn_cue = 0);
            arrows = ARROW_PACK;
            for (unsigned i = 0; i < BREAKABLE_COUNT; ++i)
                object_hp[i] = breakables[i].hp;
            action_spent = 0;
            bonus_spent = 0;
            move_limit = character.speed;
            swim_speed = 0;
            x = moving ? 9 : 3;
            y = moving ? 4 : 5;
            confirm_bow = 0;
            if (menu != ACTION_MENU_CLOSED) {
                clear_object_panel(selected, camera_x);
                if (equip_submenu(menu))
                    clear_items();
                clear_action_menu();
                menu = ACTION_MENU_CLOSED;
                object_details = 0;
            }
            camera_x = 0;
            camera_y = 0;
            sprite_x = x * CELL_SIZE;
            sprite_y = y * CELL_SIZE;
            scroll_x = 0;
            scroll_y = moving ? -HUD_HEIGHT : 0;
            REG16(0x04000000) = FORCE_BLANK;
            load_starting_view(map, VIEW_HEIGHT);
            show_view(sprite_x, sprite_y, scroll_x, scroll_y);
            show_breakables(scroll_x, scroll_y, moving, broken_objects);
            show_pose(x, y, moving, animation_frame, step_frames);
            if (moving)
                draw_hud(spent, move_limit);
            wait_vblank();
            REG16(0x04000000) = display | (moving ? BG1_ENABLE : 0);
        } else if (menu != ACTION_MENU_CLOSED) {
            unsigned bonus_available = character.can_bonus_dash && !bonus_spent;
            unsigned draw_bow = bow_selected && can_draw_and_shoot(&character.equipment, interact_spent, action_spent);
            unsigned ranged = bow || draw_bow;
            unsigned action_available = !action_spent && selected < BREAKABLE_COUNT && (!ranged || arrows);
            unsigned move_available = spent < move_limit;
            if (confirm_bow && (pressed & KEY_B)) {
                confirm_bow = 0;
                draw_items(menu, &character.equipment, interact_spent, action_spent);
                continue;
            }
            if (confirm_bow && !(pressed & KEY_A))
                continue;
            if (object_details) {
                if (pressed & (KEY_A | KEY_B | KEY_SELECT)) {
                    object_details = 0;
                    show_object_panel(selected, camera_x, object_details);
                }
                continue;
            }
            if (!equip_submenu(menu) && selected < BREAKABLE_COUNT && (pressed & KEY_SELECT)) {
                object_details = 1;
                show_object_panel(selected, camera_x, object_details);
                continue;
            }
            if ((pressed & KEY_B) ||
                ((pressed & KEY_A) && (menu == ACTION_MENU_CANCEL ||
                                       menu == ACTION_MENU_BONUS_CANCEL ||
                                       menu == ACTION_MENU_ACTION_CANCEL ||
                                       menu == ACTION_MENU_MOVE_CANCEL ||
                                       menu == EQUIP_MENU_BACK))) {
                unsigned from_items = equip_submenu(menu);
                ActionMenu parent = from_items ? ACTION_MENU_ITEMS :
                                    bonus_menu(menu) ? ACTION_MENU_BONUS :
                                    move_submenu(menu) ? ACTION_MENU_MOVEMENT :
                                    action_submenu(menu) ? ACTION_MENU_ACTION : ACTION_MENU_CLOSED;
                clear_action_menu();
                hud_notice("");
                menu = parent;
                if (from_items) {
                    clear_items();
                    show_object_panel(selected, camera_x, 0);
                }
                if (menu != ACTION_MENU_CLOSED)
                    draw_action_menu(menu, !action_spent, action_available, bonus_available, move_available, bow, draw_bow);
                else
                    clear_object_panel(selected, camera_x);
                continue;
            }
            if (!(pressed & KEY_A)) {
                if (equip_submenu(menu)) {
                    if (pressed & (KEY_UP | KEY_DOWN | KEY_LEFT | KEY_RIGHT)) {
                        menu = next_command(menu, pressed & (KEY_UP | KEY_LEFT) ? -1 : 1, draw_bow);
                        draw_items(menu, &character.equipment, interact_spent, action_spent);
                    }
                    continue;
                }
                if (pressed & (KEY_LEFT | KEY_RIGHT))
                    menu = next_command(menu, (pressed & KEY_LEFT) ? -1 : 1, draw_bow);
                if (selected < BREAKABLE_COUNT && (pressed & (KEY_UP | KEY_DOWN))) {
                    unsigned next = cycle_target(selected, (pressed & KEY_DOWN) ? 1 : -1,
                                                 x, y, broken_objects, ranged);
                    if (next != selected) {
                        clear_object_panel(selected, camera_x);
                        selected = next;
                        show_object_panel(selected, camera_x, object_details);
                        show_target(&breakables[selected], scroll_x, scroll_y);
                    }
                }
                if (pressed & KEY_DPAD) {
                    if (action_submenu(menu) && draw_bow)
                        hud_notice(menu == ACTION_MENU_DRAW_SHOOT ? "DRAW AND SHOOT" : "UP DOWN TARGET");
                    if (!bonus_menu(menu) && !move_submenu(menu) && !action_submenu(menu) && !equip_submenu(menu))
                        hud_notice(menu == ACTION_MENU_ITEMS ? "GEAR AND ITEMS" :
                                   ranged && selected < BREAKABLE_COUNT && arrows && !action_spent
                                   ? "UP DOWN TARGET" : "");
                    draw_action_menu(menu, !action_spent, action_available, bonus_available, move_available, bow, draw_bow);
                }
                continue;
            }
            ActionMenu command = menu;
            if ((command == ACTION_MENU_ACTION && action_spent) ||
                (command == ACTION_MENU_DRAW_SHOOT && (!draw_bow || !action_available)) ||
                (command == ACTION_MENU_SMASH && (bow ? !action_available :
                    action_spent || selected >= BREAKABLE_COUNT ||
                    !target_in_reach(selected, x, y, broken_objects))) ||
                (command == ACTION_MENU_DASH && action_spent)) {
                hud_notice(action_spent ? "ACTION SPENT" : ranged ? (!arrows ? "NO ARROWS" : "NO CLEAR SHOT") : "NO TARGET");
                continue;
            }
            if (equip_submenu(command)) {
                GearChange change = plan_gear_change(command, &character.equipment, interact_spent, action_spent);
                if (change.blocked && (command != EQUIP_MENU_BOW || bow)) {
                    hud_notice(change.blocked);
                    continue;
                }
                if (command == EQUIP_MENU_BOW && !change.blocked && !change.putting_on && !confirm_bow) {
                    confirm_bow = 1;
                    draw_bow_confirmation(&character.equipment, change);
                    continue;
                }
                confirm_bow = 0;
                clear_items();
                menu = ACTION_MENU_CLOSED;
                apply_gear_change(&character.equipment, change, &interact_spent, &action_spent);
                bow = character.equipment.weapon == ITEM_LONGBOW;
                if (command == EQUIP_MENU_BOW)
                    bow_selected = 1;
                else if (command == EQUIP_MENU_WEAPON)
                    bow_selected = 0;

                /* The map shows the player's selection. Attack and AC use actual gear. */
                Loadout map_gear = character.equipment;
                if (bow_selected)
                    map_gear.weapon = ITEM_LONGBOW;
                refresh_equipment(&map_gear, &shoot_art_loaded, display);
                gear_cue = bow_selected && !bow
                    ? (GearCue){GEAR_CUE_FRAMES, ITEM_LONGBOW, 1, GEAR_SELECTION}
                    : change.followup_weapon
                    ? (GearCue){GEAR_CUE_FRAMES, change.followup_weapon, 1,
                                change.followup_weapon == ITEM_WARHAMMER ? GEAR_UTILIZE : GEAR_FREE_INTERACTION}
                    : (GearCue){GEAR_CUE_FRAMES, change.item, change.putting_on, change.cost};
                gear_message_frames = GEAR_MESSAGE_FRAMES;
                draw_gear_message(&gear_cue);
                hud_notice(gear_cue.cost == GEAR_SELECTION ? (action_spent ? "ACTION SPENT" : character.equipment.shield ? "SHIELD EQUIPPED" : "DRAW AND SHOOT") : gear_confirmation(&gear_cue));
                continue;
            }
            if (command == ACTION_MENU_BONUS && !bonus_available) {
                hud_notice("BONUS SPENT");
                continue;
            }
            clear_action_menu();
            if (command == ACTION_MENU_MOVEMENT) {
                menu = ACTION_MENU_MOVE;
                hud_notice("00 FT LEFT");
                hud_number(15, move_limit - spent);
                draw_action_menu(menu, !action_spent, action_available, bonus_available, move_available, bow, draw_bow);
                continue;
            }
            if (command == ACTION_MENU_ACTION) {
                menu = action_available && draw_bow
                    ? ACTION_MENU_DRAW_SHOOT :
                    action_available ? ACTION_MENU_SMASH : ACTION_MENU_DASH;
                if (draw_bow && !shoot_art_loaded) {
                    REG16(0x04000000) = FORCE_BLANK;
                    write_card(SHOOT_CARD_TILE, bow_icon, "SHOOT");
                    shoot_art_loaded = 1;
                    wait_vblank();
                    REG16(0x04000000) = display | BG1_ENABLE;
                }
                hud_notice(!ranged ? "ACTION" : !arrows ? "NO ARROWS" :
                           selected >= BREAKABLE_COUNT ? "NO CLEAR SHOT" :
                           menu == ACTION_MENU_DRAW_SHOOT ? "DRAW AND SHOOT" : "UP DOWN TARGET");
                draw_action_menu(menu, !action_spent, action_available, bonus_available, move_available, bow, draw_bow);
                continue;
            }
            if (command == ACTION_MENU_ITEMS) {
                clear_object_panel(selected, camera_x);
                menu = EQUIP_MENU_WEAPON;
                draw_items(menu, &character.equipment, interact_spent, action_spent);
                continue;
            }
            if (command == ACTION_MENU_BONUS) {
                menu = ACTION_MENU_BONUS_DASH;
                hud_notice("BONUS ACTION");
                draw_action_menu(menu, !action_spent, action_available, bonus_available, move_available, bow, draw_bow);
                continue;
            }
            menu = ACTION_MENU_CLOSED;
            clear_object_panel(selected, camera_x);
            hud_notice("");
            if (command == ACTION_MENU_MOVE)
                continue;
            if (command == ACTION_MENU_DASH || command == ACTION_MENU_BONUS_DASH) {
                if (command == ACTION_MENU_DASH)
                    action_spent = 1;
                else
                    bonus_spent = 1;
                move_limit += character.speed;
                draw_hud(spent, move_limit);
                continue;
            }
            if (command == ACTION_MENU_DRAW_SHOOT) {
                if (character.equipment.weapon)
                    interact_spent = 1;
                character.equipment.weapon = ITEM_LONGBOW;
                bow = 1;
                refresh_equipment(&character.equipment, &shoot_art_loaded, display);
            }
            unsigned roll = roll_die(animation_frame, 20);
            const Breakable *target = &breakables[selected];
            unsigned modifier = character.athletics;
            unsigned defense = target->dc;
            unsigned hit;
            unsigned destroyed;
            if (bow) {
                --arrows;
                modifier = 2 + character.dexterity_modifier;
                defense = target->ac;
                if (bow_distance(selected, x, y) > LONGBOW_NORMAL_FT || maps[map][y][x] == '~') {
                    unsigned second = roll_die(animation_frame + 97, 20);
                    if (second < roll)
                        roll = second;
                }
                hit = roll == 20 || (roll != 1 && roll + modifier >= defense);
                unsigned damage = roll_die(animation_frame + 17, LONGBOW_DAMAGE_DIE)
                                  + character.dexterity_modifier;
                if (roll == 20)
                    damage += roll_die(animation_frame + 41, LONGBOW_DAMAGE_DIE);
                destroyed = hit && damage >= object_hp[selected];
                if (hit)
                    object_hp[selected] = destroyed ? 0 : object_hp[selected] - damage;
            } else {
                hit = roll + modifier >= defense;
                destroyed = hit;
            }
            if (destroyed)
                broken_objects |= 1u << selected;
            action_spent = 1;
            if (bow)
                store_roll(destroyed ? " D20 00 +00 = 00 AC00 BROKE" :
                           hit ? " D20 00 +00 = 00 AC00 HIT" : " D20 00 +00 = 00 AC00 MISS");
            else
                store_roll(hit ? " D20 00 +00 = 00 DC00 BROKE"
                               : " D20 00 +00 = 00 DC00 FAIL");
            hud_number(ROLL_ROW * MAP_STRIDE + 19, defense);
            hud_number(ROLL_ROW * MAP_STRIDE + 5, roll);
            hud_number(ROLL_ROW * MAP_STRIDE + 9, modifier);
            hud_number(ROLL_ROW * MAP_STRIDE + 14, roll + modifier);
            scene_success = destroyed;
            scene_hit = hit;
            scene_age = 1;
            REG16(0x04000000) = FORCE_BLANK;
            prepare_closeup(closeup_terrain(maps[map][y][x], maps[map][target->y][target->x]), target, &character);
#ifdef LOCAL_BATTLE_AUDIO
#ifdef LOCAL_MAP_AUDIO
            if (!map_fade && audio_frames > AUDIO_FADE_FRAMES) {
                fade_audio(PREVIEW_AUDIO_FRAMES - audio_frames, PREVIEW_AUDIO_FRAMES);
                battle_fade = AUDIO_FADE_FRAMES;
            } else
#endif
            {
                map_fade = 0;
                stage_audio(battle_pcm, battle_pcm_end);
                audio_frames = PREVIEW_AUDIO_FRAMES;
            }
#endif
            REG16(0x0400000a) = SCENE_BLOCK << 8;
            animate_closeup(scene_age, &breakables[selected], scene_success, scene_hit, bow);
            wait_vblank();
            REG16(0x04000000) = BG1_ENABLE | OBJ_ENABLE | OBJ_1D;
        } else if (moving && (pressed & KEY_A)) {
            clear_gear_message(&gear_message_frames);
            gear_cue.frames = 0;
            OAM[4 * GEAR_CUE_SLOT] = OBJ_DISABLED;
            unsigned bonus_available = character.can_bonus_dash && !bonus_spent;
            unsigned draw_bow = bow_selected && can_draw_and_shoot(&character.equipment, interact_spent, action_spent);
            unsigned ranged = bow || draw_bow;
            selected = nearest_breakable(x, y, broken_objects, ranged);
            unsigned action_available = !action_spent && selected < BREAKABLE_COUNT && (!ranged || arrows);
            unsigned move_available = spent < move_limit;
            menu = action_available ? ACTION_MENU_ACTION :
                   bonus_available ? ACTION_MENU_BONUS : ACTION_MENU_MOVEMENT;
            if (turn_cue)
                show_turn_banner(turn_cue = 0);
            hud_notice(bow && action_spent ? "ACTION SPENT" :
                       ranged && !arrows ? "NO ARROWS" :
                       ranged && selected >= BREAKABLE_COUNT ? "NO CLEAR SHOT" :
                       ranged ? "UP DOWN TARGET" : "");
            draw_action_menu(menu, !action_spent, action_available, bonus_available, move_available, bow, draw_bow);
            show_object_panel(selected, camera_x, object_details);
            if (selected < BREAKABLE_COUNT)
                show_target(&breakables[selected], scroll_x, scroll_y);
        } else if (moving && (pressed & (KEY_B | KEY_SELECT))) {
            if (pressed & KEY_SELECT)
                swim_speed = !swim_speed;
            if (pressed & KEY_B) {
                interact_spent = 0;
                clear_gear_message(&gear_message_frames);
                gear_cue.frames = 0;
                OAM[4 * GEAR_CUE_SLOT] = OBJ_DISABLED;
                turn_cue = TURN_CUE_FRAMES;
                show_turn_banner(turn_cue);
                turn_click();
                action_spent = 0;
                bonus_spent = 0;
                move_limit = character.speed;
            }
            spent = 0;
            draw_hud(spent, move_limit);
            if (bow_selected && !bow)
                hud_notice((action_spent ? "ACTION SPENT" : character.equipment.shield ? "SHIELD EQUIPPED" : "DRAW AND SHOOT"));
        } else if (pressed & KEY_DPAD) {
            int dx = !!(pressed & KEY_RIGHT) - !!(pressed & KEY_LEFT);
            int dy = !!(pressed & KEY_DOWN) - !!(pressed & KEY_UP);
            int destination_x = clamp(x + dx, MAP_WIDTH);
            int destination_y = clamp(y + dy, MAP_HEIGHT);
            if (moving) {
                unsigned cost = step_cost(x, y, destination_x, destination_y, swim_speed, broken_objects);
                if (cost == 0 || spent + cost > move_limit)
                    continue;
                spent += cost;
                step_frames = WALK_POSE_FRAMES;
                draw_hud(spent, move_limit);
                if (bow_selected && !bow)
                    hud_notice((action_spent ? "ACTION SPENT" : character.equipment.shield ? "SHIELD EQUIPPED" : "DRAW AND SHOOT"));
                else if (gear_cue.frames)
                    hud_notice(gear_confirmation(&gear_cue));
            }
            x = destination_x;
            y = destination_y;
            int next_x = follow(x, camera_x, VIEW_WIDTH, MAP_WIDTH);
            int view_height = moving ? TRIAL_VIEW_HEIGHT : VIEW_HEIGHT;
            int next_y = follow(y, camera_y, view_height, MAP_HEIGHT);
            /* A partial tenth row is visible except at the southern edge,
               where the viewport aligns to the last ten world rows. */
            int old_top = clamp(camera_y, MAP_HEIGHT - VIEW_HEIGHT + 1);
            int next_top = clamp(next_y, MAP_HEIGHT - VIEW_HEIGHT + 1);
            stream_edges(map, camera_x, old_top, next_x, next_top, VIEW_HEIGHT);
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
