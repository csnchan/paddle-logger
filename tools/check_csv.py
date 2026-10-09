import sys
path = sys.argv[1] if len(sys.argv) > 1 else "paddle.csv"
rows = 0
forces = []
for line in open(path):
    line = line.strip()
    if not line or line.startswith("#") or line.startswith("t_ms"):
        print(line)
        continue
    parts = line.split(",")
    rows += 1
    forces.append(float(parts[2]))
print(f"rows {rows}")
if forces:
    print(f"force min {min(forces):.2f}  max {max(forces):.2f}")
