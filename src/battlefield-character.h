#include "../build/item-data.h"

enum { ITEM_NONE };

typedef struct {
    unsigned weapon;
    unsigned armor;
    unsigned shield;
} Loadout;

typedef struct {
    const char *name;
    const char *class_name;
    unsigned hp, max_hp, speed, athletics;
    int dexterity_modifier;
    unsigned can_bonus_dash;
    Loadout equipment;
} Character;

static const Loadout unequipped = {
    .weapon = ITEM_NONE,
    .armor = ITEM_NONE,
};

static const Character hero = {
    .name = "ALDEN",
    .class_name = "FIGHTER",
    .hp = 12,
    .max_hp = 12,
    .speed = 30,
    .athletics = 4,
    .dexterity_modifier = 1,
    .can_bonus_dash = 1,  /* Granted for the River Crossing trial. */
    .equipment = {
        .weapon = ITEM_WARHAMMER,
        .armor = ITEM_CHAIN_SHIRT,
        .shield = ITEM_SHIELD,
    },
};

static unsigned armor_class(const Character *character)
{
    int dexterity = character->dexterity_modifier;
    unsigned base = 10;
    if (character->equipment.armor == ITEM_CHAIN_SHIRT) {
        base = CHAIN_SHIRT_AC;
        if (dexterity > 2)
            dexterity = 2;
    }
    return (int)base + dexterity + SHIELD_AC * !!character->equipment.shield;
}
