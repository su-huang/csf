import pandas as pd
import matplotlib.pyplot as plt
import matplotlib.ticker as ticker

df = pd.read_csv("results.csv")

# rows 0-4:   block size gcc
# rows 5-9:   block size swim
# rows 10-15: associativity gcc
# rows 16-21: associativity swim
# rows 22-26: cache size gcc
# rows 27-31: cache size swim
# rows 32-34: write policy gcc
# rows 35-37: write policy swim
# rows 38-39: eviction gcc
# rows 40-41: eviction swim

block_gcc  = df.iloc[0:5]
block_swim = df.iloc[5:10]
assoc_gcc  = df.iloc[10:16]
assoc_swim = df.iloc[16:22]
size_gcc   = df.iloc[22:27]
size_swim  = df.iloc[27:32]
policy_gcc  = df.iloc[32:35]
policy_swim = df.iloc[35:38]
evict_gcc  = df.iloc[38:40]
evict_swim = df.iloc[40:42]

BLUE  = "#378ADD"
GREEN = "#1D9E75"

def fmt_cycles(val, _):
    return f"{val/1e6:.1f}M"

fig, axes = plt.subplots(2, 3, figsize=(18, 10))
fig.suptitle("Cache Simulation Results", fontsize=14, fontweight="bold")

# --- Block size sweep ---
ax = axes[0, 0]
ax.plot(block_gcc["num_bytes"],  block_gcc["total_cycles"],  marker="o", color=BLUE,  label="gcc.trace")
ax.plot(block_swim["num_bytes"], block_swim["total_cycles"], marker="o", color=GREEN, label="swim.trace")
ax.set_title("Block size sweep\n(256 sets, 4-way, wa/wb/lru)", fontsize=10)
ax.set_xlabel("block size (bytes)")
ax.set_ylabel("total cycles")
ax.yaxis.set_major_formatter(ticker.FuncFormatter(fmt_cycles))
ax.set_xticks([4, 8, 16, 32, 64])
ax.legend(); ax.grid(True, alpha=0.3)

# --- Associativity sweep ---
ax = axes[0, 1]
ax.plot(assoc_gcc["num_blocks"],  assoc_gcc["total_cycles"],  marker="o", color=BLUE,  label="gcc.trace")
ax.plot(assoc_swim["num_blocks"], assoc_swim["total_cycles"], marker="o", color=GREEN, label="swim.trace")
ax.set_title("Associativity sweep\n(~16KB total, 16B blocks, wa/wb/lru)", fontsize=10)
ax.set_xlabel("associativity (num blocks per set)")
ax.set_ylabel("total cycles")
ax.yaxis.set_major_formatter(ticker.FuncFormatter(fmt_cycles))
ax.set_xscale("log", base=2)
ax.xaxis.set_major_formatter(ticker.FuncFormatter(lambda v, _: f"{int(v)}-way"))
ax.legend(); ax.grid(True, alpha=0.3)

# --- Cache size sweep ---
ax = axes[0, 2]
kb_gcc  = size_gcc["num_sets"]  * size_gcc["num_blocks"]  * size_gcc["num_bytes"]  // 1024
kb_swim = size_swim["num_sets"] * size_swim["num_blocks"] * size_swim["num_bytes"] // 1024
ax.plot(kb_gcc,  size_gcc["total_cycles"],  marker="o", color=BLUE,  label="gcc.trace")
ax.plot(kb_swim, size_swim["total_cycles"], marker="o", color=GREEN, label="swim.trace")
ax.set_title("Cache size sweep\n(4-way, 16B blocks, wa/wb/lru)", fontsize=10)
ax.set_xlabel("total cache size (KB)")
ax.set_ylabel("total cycles")
ax.yaxis.set_major_formatter(ticker.FuncFormatter(fmt_cycles))
ax.set_xticks([4, 8, 16, 32, 64])
ax.legend(); ax.grid(True, alpha=0.3)

# --- Write policy sweep ---
ax = axes[1, 0]
labels = ["wa/wb/lru", "wa/wt/lru", "nwa/wt/lru"]
x = range(len(labels))
width = 0.35
ax.bar([i - width/2 for i in x], policy_gcc["total_cycles"],  width, color=BLUE,  label="gcc.trace")
ax.bar([i + width/2 for i in x], policy_swim["total_cycles"], width, color=GREEN, label="swim.trace")
ax.set_title("Write policy sweep\n(256 sets, 4-way, 16B blocks, lru)", fontsize=10)
ax.set_ylabel("total cycles")
ax.yaxis.set_major_formatter(ticker.FuncFormatter(fmt_cycles))
ax.set_xticks(list(x)); ax.set_xticklabels(labels)
ax.legend(); ax.grid(True, alpha=0.3, axis="y")

# --- Eviction policy sweep ---
ax = axes[1, 1]
labels = ["lru", "fifo"]
x = range(len(labels))
ax.bar([i - width/2 for i in x], evict_gcc["total_cycles"],  width, color=BLUE,  label="gcc.trace")
ax.bar([i + width/2 for i in x], evict_swim["total_cycles"], width, color=GREEN, label="swim.trace")
ax.set_title("Eviction policy sweep\n(256 sets, 4-way, 16B blocks, wa/wb)", fontsize=10)
ax.set_ylabel("total cycles")
ax.yaxis.set_major_formatter(ticker.FuncFormatter(fmt_cycles))
ax.set_xticks(list(x)); ax.set_xticklabels(labels)
ax.legend(); ax.grid(True, alpha=0.3, axis="y")

axes[1, 2].axis("off")

plt.tight_layout()
plt.savefig("cache_results.png", dpi=150, bbox_inches="tight")
print("saved cache_results.png")