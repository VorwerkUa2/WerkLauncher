import os
import sys

def check_icons():
    base_dir = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    themes_dir = os.path.join(base_dir, "launcher", "resources", "themes")
    
    packs = ["flat", "colored", "white"]
    icon_sets = {}

    print(f"--- Icon Pack Consistency Check ---")
    
    for pack in packs:
        pack_path = os.path.join(themes_dir, pack, "scalable")
        if not os.path.exists(pack_path):
            print(f"Error: Pack path {pack_path} does not exist.")
            continue
        
        icons = {f for f in os.listdir(pack_path) if f.endswith(".svg")}
        icon_sets[pack] = icons
        print(f"Pack '{pack}': {len(icons)} icons found.")

    all_icons = set()
    for icons in icon_sets.values():
        all_icons.update(icons)

    print(f"\nChecking for missing icons (total unique icons: {len(all_icons)})")
    
    has_errors = False
    for icon in sorted(all_icons):
        missing_in = [pack for pack in packs if icon not in icon_sets.get(pack, set())]
        if missing_in:
            print(f"  [!] {icon} is missing in: {', '.join(missing_in)}")
            has_errors = True

    if not has_errors:
        print("\nSuccess: All packs are in sync!")
    else:
        print("\nWarning: Some icons are missing in specific packs.")

if __name__ == "__main__":
    check_icons()
