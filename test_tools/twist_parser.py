#!/usr/bin/env python3
import sys

for l in sys.stdin:
    if l[0] == '#':
        continue

    ts, nid, x = l.split()
    ts = float(ts)
    ch = []
    for i in range(8, len(x)//2):
        c = chr(int(x[i*2:i*2+2], 16))
        if c == "\n":
            break
        ch.append(c)

    trace = "".join(ch)
    sys.stdout.write(f"{ts:.6f} {nid} {trace}\n")
