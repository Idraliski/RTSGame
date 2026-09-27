"""Generates a 16-bit grayscale PNG heightmap for an RTS test landscape.

Run with any Python 3 (no extra packages), e.g. Unreal's bundled one:
  G:/Programs/UE_5.8/Engine/Binaries/ThirdParty/Python3/Win64/python.exe generate_rts_heightmap.py

Import it in Landscape mode > Manage > New > Import from File, using
Scale X/Y/Z = 100. With Z scale 100 one height unit is 100/128 cm, and
32768 is height 0, so height_cm = (value - 32768) * 100 / 128.
"""
import math
import random
import struct
import zlib

SIZE = 505             # 8x8 components of 63 quads = 505 verts per side
METERS_PER_VERT = 1.0  # landscape scale 100 cm
SEED = 1234

HILLS = [  # (x m, y m, height m, radius m)
    (120, 80, 8, 40),
    (-140, -60, 6, 50),
    (60, -150, 5, 35),
    (-90, 140, 7, 45),
    (170, -120, 4, 30),
    (-170, 60, 5, 30),
]


def smoothstep(e0, e1, x):
    t = max(0.0, min(1.0, (x - e0) / (e1 - e0)))
    return t * t * (3 - 2 * t)


class ValueNoise:
    def __init__(self, seed, grid=64):
        rng = random.Random(seed)
        self.grid = grid
        self.v = [[rng.random() * 2 - 1 for _ in range(grid)] for _ in range(grid)]

    def __call__(self, x, y):
        g = self.grid
        x0, y0 = math.floor(x), math.floor(y)
        tx, ty = x - x0, y - y0
        tx, ty = tx * tx * (3 - 2 * tx), ty * ty * (3 - 2 * ty)
        a = self.v[x0 % g][y0 % g]
        b = self.v[(x0 + 1) % g][y0 % g]
        c = self.v[x0 % g][(y0 + 1) % g]
        d = self.v[(x0 + 1) % g][(y0 + 1) % g]
        return (a + (b - a) * tx) * (1 - ty) + (c + (d - c) * tx) * ty


def height_m(x, y, n1, n2):
    r = math.hypot(x, y)
    # Rolling ground: two octaves of smooth noise.
    h = n1(x / 120, y / 120) * 2.0 + n2(x / 45, y / 45) * 0.8
    # Individual round hills.
    for hx, hy, hh, hr in HILLS:
        d2 = (x - hx) ** 2 + (y - hy) ** 2
        h += hh * math.exp(-d2 / (2 * (hr * 0.5) ** 2))
    # Keep the middle flat for spawning and testing selection/movement.
    h *= smoothstep(45, 100, r)
    # Raised rim so the map edge reads as a boundary from the RTS camera.
    half = SIZE * METERS_PER_VERT / 2
    edge = min(half - abs(x), half - abs(y))
    h += 15 * (1 - smoothstep(0, 40, edge))
    return h


def write_png16(path, rows):
    raw = b"".join(b"\x00" + struct.pack(">%dH" % SIZE, *row) for row in rows)

    def chunk(tag, data):
        return struct.pack(">I", len(data)) + tag + data + struct.pack(">I", zlib.crc32(tag + data))

    ihdr = struct.pack(">IIBBBBB", SIZE, SIZE, 16, 0, 0, 0, 0)  # 16-bit grayscale
    with open(path, "wb") as f:
        f.write(b"\x89PNG\r\n\x1a\n" + chunk(b"IHDR", ihdr)
                + chunk(b"IDAT", zlib.compress(raw, 9)) + chunk(b"IEND", b""))


def main():
    n1, n2 = ValueNoise(SEED), ValueNoise(SEED + 1)
    half = (SIZE - 1) / 2
    rows, lo, hi = [], 1e9, -1e9
    for j in range(SIZE):
        row = []
        for i in range(SIZE):
            h = height_m((i - half) * METERS_PER_VERT, (j - half) * METERS_PER_VERT, n1, n2)
            lo, hi = min(lo, h), max(hi, h)
            row.append(max(0, min(65535, round(32768 + h * 100 * 1.28))))
        rows.append(row)
    write_png16("RTSTerrain_Heightmap.png", rows)
    print(f"Wrote RTSTerrain_Heightmap.png {SIZE}x{SIZE}, heights {lo:.2f} m to {hi:.2f} m")


if __name__ == "__main__":
    main()
