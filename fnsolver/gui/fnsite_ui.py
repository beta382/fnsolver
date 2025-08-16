import re
import sys
from dataclasses import dataclass
from pathlib import Path
import xml.etree.ElementTree as ET


@dataclass
class Site:
    name: int
    x: int
    y: int


def main():
    if len(sys.argv) != 3:
        print(f"Extract site positions from map SVG.\nUsage: {sys.argv[0]} <map.svg> <out.cpp>")
        return 1

    # Load node map positions.
    svg_path = Path(sys.argv[1])
    svg = ET.parse(svg_path)
    nodes = svg.findall("./{http://www.w3.org/2000/svg}g[@id='nodeslayer']/{http://www.w3.org/2000/svg}image")
    sites: list[Site] = []
    for node in nodes:
        match = re.match(r"FN(\d+)", node.get("{http://www.inkscape.org/namespaces/inkscape}label"))
        if match is None:
            continue
        name = int(match.group(1), 10)
        x = int(node.get("x"))
        y = int(node.get("y"))
        width = int(node.get("width"))
        height = int(node.get("height"))
        # Origin is top-left.
        sites.append(Site(name=name, x=int(x + (width / 2)), y=int(y + (height / 2))))

    # Sort site by ID for writing.
    sites.sort(key=lambda site: site.name)

    # Write CPP definition file.
    out_path = Path(sys.argv[2])
    with out_path.open('w') as out:
        out.write(
            "/* To update this file, run fnsite_ui.py <path to map svg> <path to this cpp file> */\n"
            "\n"
            '#include "fnsite_ui.h"\n'
            "\n"
            "extern const std::unordered_map<FnSite::id_t, std::pair<int, int>> site_positions{\n"
        )
        for site in sites:
            out.write(f"  {{{site.name}, {{{site.x}, {site.y}}}}},\n")
        out.write("};\n")

    return 0


if __name__ == "__main__":
    main()
