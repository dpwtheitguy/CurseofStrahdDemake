#include <stdint.h>

enum {
    SCREEN_WIDTH = 240,
    SCREEN_HEIGHT = 160,
    MODE_3 = 3,
    BG2_ENABLE = 1 << 10,
    FORCE_BLANK = 1 << 7,
    GLYPH_WIDTH = 5,
    GLYPH_HEIGHT = 7,
    GLYPH_SCALE = 3
};

#define DISPLAY_CONTROL (*(volatile uint16_t *)0x04000000)
#define FRAMEBUFFER ((volatile uint16_t *)0x06000000)
#define RGB5(r, g, b) ((r) | ((g) << 5) | ((b) << 10))

/* Hand-drawn 5x7 glyph rows spelling HELLO WORLD. */
static const uint8_t greeting[][GLYPH_HEIGHT] = {
    {17, 17, 17, 31, 17, 17, 17}, /* H */
    {31, 16, 16, 30, 16, 16, 31}, /* E */
    {16, 16, 16, 16, 16, 16, 31}, /* L */
    {16, 16, 16, 16, 16, 16, 31}, /* L */
    {14, 17, 17, 17, 17, 17, 14}, /* O */
    { 0,  0,  0,  0,  0,  0,  0},
    {17, 17, 17, 21, 21, 27, 17}, /* W */
    {14, 17, 17, 17, 17, 17, 14}, /* O */
    {30, 17, 17, 30, 20, 18, 17}, /* R */
    {16, 16, 16, 16, 16, 16, 31}, /* L */
    {30, 17, 17, 17, 17, 17, 30}  /* D */
};

static void fill_rect(int x, int y, int width, int height, uint16_t color)
{
    for (int row = y; row < y + height; ++row) {
        for (int col = x; col < x + width; ++col) {
            FRAMEBUFFER[row * SCREEN_WIDTH + col] = color;
        }
    }
}

int main(void)
{
    const int count = sizeof greeting / sizeof greeting[0];
    const int advance = (GLYPH_WIDTH + 1) * GLYPH_SCALE;
    const int left = (SCREEN_WIDTH - (count * advance - GLYPH_SCALE)) / 2;
    const int top = (SCREEN_HEIGHT - GLYPH_HEIGHT * GLYPH_SCALE) / 2;

    DISPLAY_CONTROL = MODE_3 | BG2_ENABLE | FORCE_BLANK;
    fill_rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, RGB5(4, 5, 8));
    fill_rect(12, 53, 216, 54, RGB5(12, 19, 18));
    fill_rect(13, 54, 214, 52, RGB5(6, 9, 12));
    for (int i = 0; i < count; ++i) {
        for (int row = 0; row < GLYPH_HEIGHT; ++row) {
            for (int col = 0; col < GLYPH_WIDTH; ++col) {
                if (greeting[i][row] & (1 << (GLYPH_WIDTH - 1 - col))) {
                    fill_rect(left + i * advance + col * GLYPH_SCALE,
                              top + row * GLYPH_SCALE, GLYPH_SCALE,
                              GLYPH_SCALE, RGB5(31, 27, 19));
                }
            }
        }
    }
    DISPLAY_CONTROL = MODE_3 | BG2_ENABLE;

    for (;;) {}
}
