import os
import re
import urllib.request

# Configuration
THEMES_BASE = "launcher/resources/themes"
BUILTIN_BASE = "launcher/resources/builtin/scalable"
ICONS_TO_PROCESS = {
    "telegram": {"brand_color": "#26A5E4", "source": "https://cdn.simpleicons.org/telegram"},
    "discord": {"brand_color": "#5865F2", "source": "https://cdn.simpleicons.org/discord"},
    "modrinth": {"brand_color": "#1BD768", "source": "https://cdn.simpleicons.org/modrinth"},
    "atlauncher": {"brand_color": "#D42027", "local": "atlauncher.svg"},
    "technic": {"brand_color": "#11A0E1", "local": "technic.svg"},
    "ftb_logo": {"brand_color": "#ff3300", "local": "ftb_logo.svg"},
    "microsoft": {"brand_color": "#00a4ef", "local": "microsoft.svg"},
    "reddit-alien": {"brand_color": "#FF4500", "source": "https://cdn.simpleicons.org/reddit"},
    "unknown_server": {"brand_color": "#4CAF50", "local": "unknown_server.svg"},
    "curseforge": {"brand_color": "#F15A24", "local": "curseforge.svg"},
}

FLAT_COLOR = "#757575"
WHITE_COLOR = "#ffffff"

def download_svg(url):
    try:
        req = urllib.request.Request(url, headers={'User-Agent': 'Mozilla/5.0'})
        with urllib.request.urlopen(req) as response:
            return response.read().decode('utf-8')
    except Exception as e:
        print(f"Failed to download {url}: {e}")
        return None

def get_svg(info):
    if "local" in info:
        path = os.path.join(BUILTIN_BASE, info["local"])
        if os.path.exists(path):
            with open(path, "r", encoding="utf-8") as f:
                return f.read()
    elif "source" in info:
        return download_svg(info["source"])
    return None

def apply_color(svg_content, color):
    # Remove existing fill attributes and add a global one to the svg tag
    # or just replace all fill attributes
    
    # Standardize: remove all fill="..." and fill:..." from elements
    svg_content = re.sub(r'fill="[^"]*"', '', svg_content)
    svg_content = re.sub(r'fill:[^;"]*', '', svg_content)
    
    # Add fill to the svg tag
    svg_content = svg_content.replace('<svg ', f'<svg fill="{color}" ')
    
    return svg_content

def main():
    for name, info in ICONS_TO_PROCESS.items():
        print(f"Processing {name}...")
        
        # 1. Get SVG content
        svg_content = get_svg(info)
        if not svg_content:
            print(f"  Skipping {name}, no content found.")
            continue
        
        # Standardize size/viewBox if needed (Simple Icons are usually 24x24)
        if 'viewBox' not in svg_content:
             svg_content = svg_content.replace('<svg ', '<svg viewBox="0 0 24 24" ')

        # 2. Save Colored version
        colored_path = os.path.join(THEMES_BASE, "colored", "scalable", f"{name}.svg")
        os.makedirs(os.path.dirname(colored_path), exist_ok=True)
        with open(colored_path, "w", encoding="utf-8") as f:
            if "local" in info:
                # Keep original colors for local files in Colored theme
                f.write(svg_content)
            else:
                f.write(apply_color(svg_content, info["brand_color"]))
        
        # 3. Save Flat version
        flat_path = os.path.join(THEMES_BASE, "flat", "scalable", f"{name}.svg")
        os.makedirs(os.path.dirname(flat_path), exist_ok=True)
        with open(flat_path, "w", encoding="utf-8") as f:
            f.write(apply_color(svg_content, FLAT_COLOR))
            
        # 4. Save White version
        white_path = os.path.join(THEMES_BASE, "white", "scalable", f"{name}.svg")
        os.makedirs(os.path.dirname(white_path), exist_ok=True)
        with open(white_path, "w", encoding="utf-8") as f:
            f.write(apply_color(svg_content, WHITE_COLOR))

    print("Done!")

if __name__ == "__main__":
    main()
