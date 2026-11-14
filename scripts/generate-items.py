"""Validate the authoring catalog and compile the item records used by the ROM."""
import json
import re
import sys
from pathlib import Path


def main():
    catalog = json.loads(Path(sys.argv[1]).read_text())
    if catalog["schema_version"] != 1:
        raise ValueError("Unsupported item schema version")
    items = {}
    for item in catalog["items"]:
        key = item["id"]
        if not re.fullmatch(r"[a-z0-9]+(?:-[a-z0-9]+)*", key) or key in items:
            raise ValueError(f"Invalid or duplicate item ID: {key}")
        items[key] = item
    for item in items.values():
        weapon = item.get("weapon", {})
        if "range_ft" in weapon:
            near, far = weapon["range_ft"]
            if not 0 < near <= far or near % 5 or far % 5:
                raise ValueError(f"Invalid weapon range: {item['id']}")
        if ammo := weapon.get("ammunition"):
            if items[ammo]["kind"] != "ammunition":
                raise ValueError(f"Invalid ammunition reference: {ammo}")

    print("/* Generated from the SRD item catalog. */")
    print("enum {")
    for index, key in enumerate(("warhammer", "chain-shirt", "shield", "longbow", "arrows"), 1):
        print(f"    ITEM_{key.upper().replace('-', '_')} = {index},")
        item = items[key]
        if key == "longbow":
            weapon = item["weapon"]
            if weapon["ammunition"] != "arrows" or "Two-Handed" not in weapon["properties"]:
                raise ValueError("The prototype requires a two-handed bow that uses arrows")
            count, sides = map(int, weapon["damage"].split("d"))
            if count != 1:
                raise ValueError("Prototype bow requires one damage die")
            print(f"    LONGBOW_DAMAGE_DIE = {sides},")
            print(f"    LONGBOW_NORMAL_FT = {weapon['range_ft'][0]},")
            print(f"    LONGBOW_LONG_FT = {weapon['range_ft'][1]},")
        if key in ("chain-shirt", "shield"):
            print(f"    {key.upper().replace('-', '_')}_AC = {item['armor']['base_ac']},")
        if key == "arrows":
            print(f"    ARROW_PACK = {item['pack_quantity']},")
    print("};")


if __name__ == "__main__":
    main()
