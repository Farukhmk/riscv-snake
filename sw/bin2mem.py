#!/usr/bin/env python3
"""words <in.bin> <out.hex>   : one little-endian 32-bit word per line (imem)
   banks <in.bin> <prefix>   : <prefix>0..3.hex, one byte per line per lane (dmem)"""
import sys

def load(p):
    b = open(p, "rb").read()
    return b + b"\x00" * (-len(b) % 4)

def main():
    if len(sys.argv) != 4 or sys.argv[1] not in ("words", "banks"):
        sys.exit(__doc__)
    mode, src, dst = sys.argv[1:]
    b = load(src)
    if mode == "words":
        with open(dst, "w") as f:
            for i in range(0, len(b), 4):
                f.write("%08x\n" % int.from_bytes(b[i:i+4], "little"))
    else:
        for lane in range(4):
            with open(f"{dst}{lane}.hex", "w") as f:
                for i in range(lane, len(b), 4):
                    f.write("%02x\n" % b[i])
                if len(b) == 0:
                    f.write("00\n")

main()
