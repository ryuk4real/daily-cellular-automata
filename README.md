# Daily Cellular Automata

Generates unique cellular automata patterns daily using HROT notation and posts the generated video to the [Daily Cellular Automata](https://t.me/daily_cellular_automata) Telegram channel.

## Setup

Install system dependencies (Debian/Ubuntu):

```bash
sudo apt-get update
sudo apt-get install -y gcc make ffmpeg
```

Install system dependencies (Arch Linux):

```bash
sudo pacman -S gcc make ffmpeg
```

Create a virtual environment and install Python dependencies:

```bash
python3 -m venv venv
source venv/bin/activate
pip install -r requirements.txt
```

## Quick Start

Generate today's pattern:
```bash
make daily
```

Generate a specific date pattern:
```bash
make daily SEED=20251122
```

Run Conway's Game of Life test:
```bash
make test
```

## HROT Notation

**HROT** (Higher Range Outer Totalistic) notation as used by LifeViewer and Golly:

```
Rr,Cc,Slist,Blist,Nn
```

### Components:

- **Rr**: Neighborhood range (r is from 1 to 8)
  - R1 = 3×3 grid, R2 = 5×5 grid, R3 = 7×7 grid, etc.
  - Automatically clamped to grid size (max = width/2)

- **Cc**: Number of cell states (c is from 2 to 16)
  - C2 = binary (alive/dead)
  - C3+ = multi-state (dying cells fade through states)

- **Slist**: Neighbor counts for State 1 cells to survive
  - Single values: `S2,3` = survives with 2 or 3 neighbors
  - Ranges: `S2-5` = survives with 2, 3, 4, or 5 neighbors
  - Mixed: `S2,4-6,8` = survives with 2, 4, 5, 6, or 8 neighbors

- **Blist**: Neighbor counts for dead cells to become births
  - Same notation as survival
  - Example: `B3` = births with exactly 3 neighbors

- **Nn**: Extended neighborhood type (optional)
  - Omitted = Moore (default, square, all 8 directions)
  - `NN` = Von Neumann (diamond, 4 cardinal directions)

### Examples:

```
R1,C2,S2,3,B3              # Conway's Game of Life (Moore default)
R2,C3,S4-7,B5,6,NN         # Multi-state with Von Neumann
R1,C2,S2,3,B3,6            # HighLife (Moore default)
R5,C128,S10-20,B15         # Large neighborhood, many states
```

### Multi-State Behavior:

When C > 2, cells have multiple states:
- **State 0**: Dead (checks birth rule B)
- **State 1**: Alive (checks survival rule S)
- **State 2+**: Dying (auto-increment State→State+1 until reaching 0)

Only State 1 cells count as neighbors. Dying cells (State 2+) don't contribute to neighbor counts.

## Daily Generation

Each day (12:00 CET) generates a unique rule with:
- Random range (R1-R8)
- Random states (C2-C16)
- Random neighborhood
- Random survive/birth conditions (each neighbor count has 15% probability)
- 100x100 grid, 200 generations, 35% alive cell density

Seed format: `YYYYMMDD` (20251122 = Nov 22, 2025) to reproduce the daily generated CA otherwise the seed can be any string.

## Output

- **Simulation**: Binary files in `output/`
- **Video**: 30-second MP4 with randomized high-contrast colors
- **Telegram**: Auto-posted daily with rule details in the [Daily Cellular Automata Channel](https://t.me/daily_cellular_automata)

### Activity Score (Hamming Flux)

The simulation calculates an "Activity Score" to measure the dynamism of the rule. Formally known as **Hamming Flux**, this metric quantifies the rate of change in the system by calculating the normalized Hamming distance between consecutive generations averaged over the entire simulation.

$$\text{Activity} = \frac{1}{G \cdot W \cdot H} \sum_{g=1}^{G} \sum_{x,y} [S_{g}(x,y) \neq S_{g-1}(x,y)] \times 100\%$$

Where:
- **$G$**: Total number of generations simulated
- **$W$**: Grid width (number of columns)
- **$H$**: Grid height (number of rows)
- **$S_g(x,y)$**: State of the cell at coordinates $(x,y)$ at generation g
- **$[\text{condition}]$**: Iverson bracket (1 if condition is true, 0 otherwise). Specifically, $[S_{g}(x,y) \neq S_{g-1}(x,y)]$ evaluates to 1 if the cell changed state from the previous generation, and 0 if it remained the same.

This metric is used to approximate the **[Edge of Chaos](https://en.wikipedia.org/wiki/Edge_of_chaos)** (a phase transition zone coined by computer scientist Christopher Langton). Rules in this zone exhibit complex, life-like behaviors and correspond to **Class 4** in [Stephen Wolfram's classification](https://mathworld.wolfram.com/CellularAutomaton.html).

- **< 1% (Static)**: Systems that quickly freeze or enter short-period loops (Wolfram Class 1 & 2).
- **5-20% (Structured)**: The "[Goldilocks zone](https://en.wikipedia.org/wiki/Habitable_zone)" where gliders, spaceships, and complex interactions often emerge (Wolfram Class 4).
- **> 30% (Chaotic)**: High-volatility noise similar to TV static (Wolfram Class 3).

**Reference:**
> [Langton, C. G. (1990). "Computation at the edge of chaos: Phase transitions and emergent computation." *Physica D: Nonlinear Phenomena*, 42(1-3), 12-37](https://www.sciencedirect.com/science/article/abs/pii/016727899090064V?via%3Dihub)

## Requirements
To run the project locally, you need:
- GCC with OpenMP
- Python 3, matplotlib, numpy, ffmpeg, virtualenv
- Telegram bot token (for automated posting)

## Future Improvements

Checklist of additional neighborhoods to implement (based on [Larger than Life (LtL)](https://golly.sourceforge.io/Help/Algorithms/Larger_than_Life.html#nbrhds)):

- [x] Moore (NM)
- [x] Von Neumann (NN)
- [ ] Circular (NC)
- [ ] L2/Euclidean (N2)
- [ ] Checkerboard (NB)
- [ ] Aligned Checkerboard (ND)
- [ ] Cross (N+)
- [ ] Saltire (NX)
- [ ] Star (N*)
- [ ] Hash (N#)

## License

GPL-3.0