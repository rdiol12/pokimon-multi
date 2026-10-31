"""Color families of a Pokémon sprite palette, the same way the game splits
them (src/breed_fusion.c SplitFamilies): the darkest colors (outlines) are
left out, greys and whites form one family, and the other colors join the
family of a near hue (within about 37°). Families are ranked by how much of
the sprite they cover. Used by tools/breed_fusion/make_fusion_data.py."""

HUE_RANGE = 1536
HUE_NEAR = 160
DARK_LIMIT = 44
GREY_LIMIT = 40
MAX_FAMILIES = 4


def to_hsl(color):
    r, g, b = (color & 31) * 8, ((color >> 5) & 31) * 8, ((color >> 10) & 31) * 8
    mx, mn = max(r, g, b), min(r, g, b)
    d = mx - mn
    lum = (mx + mn) // 2
    if d == 0:
        return 0, 0, lum
    sat = int(d * 255 / (mx + mn)) if lum < 128 else int(d * 255 / (510 - mx - mn))
    if mx == r:
        hue = (int((g - b) * 256 / d) + HUE_RANGE) % HUE_RANGE
    elif mx == g:
        hue = int((b - r) * 256 / d) + 512
    else:
        hue = int((r - g) * 256 / d) + 1024
    return hue, sat, lum


def hue_distance(a, b):
    d = abs(a - b)
    return min(d, HUE_RANGE - d)


def families(palette, counts):
    """The color (RGB15) standing for each family, biggest family first."""
    hsl = [to_hsl(c) for c in palette]
    order = sorted((i for i in range(1, 16) if hsl[i][2] >= DARK_LIMIT), key=lambda i: -counts[i])
    reps, totals = [], []
    for c in order:
        grey = hsl[c][1] < GREY_LIMIT
        found, best = None, HUE_RANGE
        for k, rep in enumerate(reps):
            if (hsl[rep][1] < GREY_LIMIT) != grey:
                continue
            if grey:
                found, best = k, 0
                break
            d = hue_distance(hsl[rep][0], hsl[c][0])
            if d < best:
                found, best = k, d
        if found is not None and best >= HUE_NEAR:
            found = None
        if found is None and counts[c] and len(reps) < MAX_FAMILIES:
            reps.append(c)
            totals.append(0)
            found = len(reps) - 1
        if found is None and reps:
            found = min(range(len(reps)), key=lambda k: hue_distance(hsl[reps[k]][0], hsl[c][0])
                        + abs(hsl[reps[k]][1] - hsl[c][1]) * 4)
        if found is not None:
            totals[found] += counts[c]
    ranked = sorted(range(len(reps)), key=lambda k: -totals[k])
    return [palette[reps[k]] for k in ranked]
