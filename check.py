#!/usr/bin/env python

"""
Compare two NxN sandpile renders, only on the cells inside the pacman region
(cells outside it are not specified by the reference).
"""

import argparse
import sys

import numpy as np
from PIL import Image


def load(path, n):
    img = np.asarray(Image.open(path).convert("RGB"))
    if img.shape[:2] != (n, n):
        sys.exit(f"{path}: expected {n}x{n}, got {img.shape[1]}x{img.shape[0]}")
    return img


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("n", type=int)
    parser.add_argument("img1")
    parser.add_argument("img2")
    args = parser.parse_args()
    n = args.n

    img1 = load(args.img1, n)
    img2 = load(args.img2, n)

    c = n // 2
    i, j = np.mgrid[0:n, 0:n]
    region = ((i - c) ** 2 + (j - c) ** 2 <= c * c) & (j <= np.maximum(i, n - 1 - i))

    diff = np.argwhere((img1 != img2).any(axis=2) & region)
    if len(diff) == 0:
        print(f"N={n}: all {region.sum()} region cells match")
    else:
        print(f"N={n}: {len(diff)} of {region.sum()} region cells differ")


if __name__ == "__main__":
    main()
