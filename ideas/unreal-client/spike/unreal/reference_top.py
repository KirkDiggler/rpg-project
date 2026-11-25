# Draws an atlas from its JSON dump with the web's formulas, independently of
# the Unreal scene, so the two can be compared:
#
#   python3 reference_top.py <castle-atlas.json> <out.png>
#
# ProtoJSON leaves out zero-valued fields, so every number is read tolerantly.
# Orientation: x right, the web's z down the screen (its own top view).
import json, math, sys
from PIL import Image, ImageDraw

atlas = json.load(open(sys.argv[1]))
num = lambda v: float(v) if v not in (None, "") else 0.0
SQRT3, CM = math.sqrt(3), 100.0           # one scene unit is one metre here
FEET = SQRT3 / 5.0 * CM                   # a hex is five feet flat to flat

def center(q, r):                         # pointy-top: x = sqrt(3) (q + r/2), z = 1.5 r
    return (SQRT3 * (q + r / 2) * CM, 1.5 * r * CM)

cells = [(int(num(c.get("x"))), int(num(c.get("y")))) for c in atlas.get("cells", [])]
pts = [center(q, r) for q, r in cells]
minx, maxx = min(p[0] for p in pts) - 150, max(p[0] for p in pts) + 150
miny, maxy = min(p[1] for p in pts) - 150, max(p[1] for p in pts) + 150
W = 1920
scale = W / (maxx - minx)
img = Image.new("RGB", (W, int((maxy - miny) * scale)), (70, 70, 76))
draw = ImageDraw.Draw(img)
px = lambda x, y: ((x - minx) * scale, (y - miny) * scale)

for q, r in cells:
    cx, cy = center(q, r)
    draw.polygon([px(cx + CM * math.cos(math.radians(30 + 60 * k)), cy + CM * math.sin(math.radians(30 + 60 * k))) for k in range(6)],
                 fill=(96, 105, 140))

for placed in atlas.get("placed", []):    # depth along the facing, width across it; both in feet
    p = placed.get("placement", {})
    ox, oy = num(p.get("origin", {}).get("x")), num(p.get("origin", {}).get("y"))
    w, depth, facing = num(p.get("width")), num(p.get("depth")), num(p.get("facing"))
    ux, uy = math.cos(math.radians(facing)), math.sin(math.radians(facing))
    vx, vy = -uy, ux
    draw.polygon([px((ox + ux * depth / 2 * a + vx * w / 2 * b) * FEET, (oy + uy * depth / 2 * a + vy * w / 2 * b) * FEET)
                  for a, b in ((-1, -1), (1, -1), (1, 1), (-1, 1))], fill=(235, 225, 205), outline=(60, 50, 40))

start = atlas.get("start", {}).get("at", {})
sx, sy = px(*center(int(num(start.get("x"))), int(num(start.get("y")))))
draw.ellipse([sx - 9, sy - 9, sx + 9, sy + 9], fill=(230, 60, 60))
img.save(sys.argv[2])
print("%s: %d hexes, %d placed boxes" % (sys.argv[2], len(cells), len(atlas.get("placed", []))))
