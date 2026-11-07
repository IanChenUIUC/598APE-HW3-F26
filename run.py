#!/usr/bin/env python

"""
Driver script: what is the biggest size (in jumps by factor of 2^{1/4})
that we can complete in t seconds (roughly, doubling the time per jump).
"""

from subprocess import check_output, TimeoutExpired
import argparse


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("-t", "--timeout", required=False, default=5)
    args = parser.parse_args()

    size = 65
    while True:
        cmd = ["./build/sandpile", "-n", str(size)]
        try:
            print(" ".join(cmd))
            check_output(cmd, timeout=args.timeout)
        except TimeoutExpired:
            print(f"Timed out at {size=} after {args.timeout} seconds")
            return
        size = int(size * (2**0.25))
        size += ~(size & 1)  # ensure it is odd


if __name__ == "__main__":
    main()
