#include <assert.h>
#include <stdio.h>
#include <string.h>

/* Exercise the ROM's pure rule functions without calling its hardware entry point. */
#define main gba_main
#include "../src/battlefield.c"
#undef main

static void equipment_rules(void)
{
    Loadout gear = hero.equipment;
    assert(!can_draw_and_shoot(&gear, 0, 0));
    gear.shield = ITEM_NONE;
    assert(can_draw_and_shoot(&gear, 0, 0));
    assert(!can_draw_and_shoot(&gear, 1, 0));
    assert(!can_draw_and_shoot(&gear, 0, 1));
    gear.shield = ITEM_SHIELD;
    unsigned interaction = 0, action = 0;
    GearChange change = plan_gear_change(EQUIP_MENU_WEAPON, &gear, interaction, action);
    assert(change.item == ITEM_WARHAMMER && !change.putting_on && change.cost == GEAR_FREE_INTERACTION);
    apply_gear_change(&gear, change, &interaction, &action);
    assert(gear.weapon == ITEM_NONE && gear.shield == ITEM_SHIELD && interaction && !action);

    change = plan_gear_change(EQUIP_MENU_BOW, &gear, interaction, action);
    assert(change.item == ITEM_SHIELD && change.cost == GEAR_UTILIZE);
    apply_gear_change(&gear, change, &interaction, &action);
    assert(gear.shield == ITEM_NONE && interaction && action);

    change = plan_gear_change(EQUIP_MENU_BOW, &gear, interaction, action);
    assert(change.blocked && strcmp(change.blocked, "END TURN FIRST") == 0);
    apply_gear_change(&gear, change, &interaction, &action);
    assert(gear.weapon == ITEM_NONE && interaction && action);

    interaction = 0;
    action = 0;
    change = plan_gear_change(EQUIP_MENU_BOW, &gear, interaction, action);
    assert(change.item == ITEM_LONGBOW && change.putting_on && change.cost == GEAR_FREE_INTERACTION);
    apply_gear_change(&gear, change, &interaction, &action);
    assert(gear.weapon == ITEM_LONGBOW && interaction && !action);
    assert(plan_gear_change(EQUIP_MENU_BOW, &gear, interaction, action).blocked);
    assert(plan_gear_change(EQUIP_MENU_SHIELD, &gear, interaction, action).blocked);

    change = plan_gear_change(EQUIP_MENU_WEAPON, &gear, interaction, action);
    assert(change.item == ITEM_LONGBOW && !change.putting_on && change.cost == GEAR_UTILIZE);
}

static void targeting_rules(void)
{
    assert(!can_target(BREAKABLE_COUNT, 9, 4, 0, 1));
    assert(!can_target(2, 9, 4, 0, 0)); /* Barrel beyond hand reach. */
    assert(can_target(2, 9, 4, 0, 1));
    assert(!can_target(2, 9, 4, 1u << 2, 1));
    assert(!can_target(0, 3, 3, 0, 1)); /* Ruined wall blocks the line. */
    assert(bow_distance(2, 9, 4) == 10);
    assert(square_in_attack_range(9, 4, 8, 3, 0));
    assert(!square_in_attack_range(9, 4, 9, 4, 0));
    assert(!square_in_attack_range(9, 4, 11, 5, 0));
    assert(square_in_attack_range(9, 4, 11, 5, 1));
    assert(!square_in_attack_range(3, 3, 9, 3, 1));

    assert(nearest_breakable(9, 4, 0, 1) == 0);
    assert(cycle_target(0, 1, 9, 4, 0, 1) == 1);
    assert(cycle_target(0, -1, 9, 4, 0, 1) == BREAKABLE_COUNT - 1);
    assert(cycle_target(0, 1, 9, 4, 1u << 1, 1) == 2);
    assert(cycle_target(0, 1, 9, 4, (1u << BREAKABLE_COUNT) - 1, 1) == BREAKABLE_COUNT);
}

int main(void)
{
    equipment_rules();
    targeting_rules();
    puts("Battlefield rule unit tests OK");
    return 0;
}
