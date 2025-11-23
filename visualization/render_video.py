import numpy as np
import matplotlib.pyplot as plt
import matplotlib.animation as animation
import struct
import sys
import time
from matplotlib.colors import LinearSegmentedColormap
from pathlib import Path
from datetime import datetime
from multiprocessing import Pool, cpu_count

COLOR_SCHEMES = [
    ('#02343F', '#F0EDCC'),
    ('#331B3F', '#ACC7B4'),
    ('#000000', '#FFFFFF'),
    ('#0A174E', '#F5D042'),
    ('#07553B', '#CED46A'),
    ('#50586C', '#DCE2F0'),
    ('#815854', '#F9EBDE'),
    ('#1E4174', '#DDA94B'),
    ('#A4193D', '#FFDFB9'),
    ('#FFDFDE', '#6A7BA2'),
    ('#3B1877', '#DA5A2A'),
    ('#5F4B8B', '#E69A8D'),  
    ('#00203F', '#ADEFD1'),
    ('#606060', '#D6ED17'),
    ('#2C5F2D', '#97BC62'),
    ('#00539C', '#EEA47F'),
    ('#101820', '#FEE715'),
]

def hex_to_rgb(hex_color):
    hex_color = hex_color.lstrip('#')
    return tuple(int(hex_color[i:i+2], 16) / 255.0 for i in (0, 2, 4))

def create_custom_colormap(alive_hex, dead_hex):
    alive_rgb = hex_to_rgb(alive_hex)
    dead_rgb = hex_to_rgb(dead_hex)
    colors = [dead_rgb, alive_rgb]
    return LinearSegmentedColormap.from_list('custom', colors, N=256)

def load_generation(filepath):
    with open(filepath, 'rb') as f:
        width = struct.unpack('i', f.read(4))[0]
        height = struct.unpack('i', f.read(4))[0]
        data = np.frombuffer(f.read(), dtype=np.uint8)
        return data.reshape((height, width))

def load_generation_wrapper(args):
    idx, filepath = args
    return idx, load_generation(filepath)

def create_video(input_folder, output_file='automata.mp4', fps=None, max_frames=None, duration=None):
    start_time = time.time()
    input_path = Path(input_folder)
    files = sorted(input_path.glob('gen_*.bin'))
    
    if not files:
        print(f"No generation files found in {input_folder}")
        return
    
    # Read rule info to get generation limit, rule string, and seed
    rule_str = "Unknown Rule"
    seed_val = None
    activity_str = "N/A"
    
    info_file = input_path / 'rule_info.txt'
    if info_file.exists():
        with open(info_file, 'r') as f:
            for line in f:
                if line.startswith("Generations:"):
                    try:
                        gen_limit = int(line.split(":")[1].strip())
                        if len(files) > gen_limit:
                            print(f"Limiting frames to {gen_limit} (from rule_info.txt)")
                            files = files[:gen_limit]
                    except ValueError:
                        pass
                elif line.startswith("Rule:"):
                    rule_str = line.split(":", 1)[1].strip()
                elif line.startswith("Seed:"):
                    try:
                        seed_val = int(line.split(":")[1].strip())
                    except ValueError:
                        pass
                elif line.startswith("Activity:"):
                    activity_str = line.split(":", 1)[1].strip()
    
    if max_frames:
        files = files[:max_frames]
    
    if duration is not None and fps is None:
        fps = len(files) / duration
        print(f"Calculating FPS for {duration}s video: {fps:.2f} fps")
    elif fps is None:
        fps = 60
    
    print(f"Loading {len(files)} generations in parallel...")
    load_start = time.time()
    num_processes = min(cpu_count(), len(files))
    
    with Pool(processes=num_processes) as pool:
        tasks = [(i, files[i]) for i in range(len(files))]
        results = pool.map(load_generation_wrapper, tasks)
    
    results.sort(key=lambda x: x[0])
    frames = [result[1] for result in results]
    
    load_time = time.time() - load_start
    print(f"Loaded all frames in {load_time:.2f} seconds using {num_processes} processes")
    
    first_frame = frames[0]
    max_state = max(np.max(frame) for frame in frames)
    print(f"Grid dimensions: {first_frame.shape}")
    print(f"Alive cells in first frame: {np.sum(first_frame == 1)}")
    print(f"Max state value: {max_state}")
    
    # Set random seed if available
    if seed_val is not None:
        np.random.seed(seed_val)
    
    # Select random color scheme
    # Ensure it's different from the previous day's color if using date seed
    color_idx = np.random.randint(0, len(COLOR_SCHEMES))
    
    if seed_val is not None:
        try:
            # Check previous day's seed
            seed_str = str(seed_val)
            if len(seed_str) == 8:
                dt = datetime.strptime(seed_str, "%Y%m%d")
                prev_dt = dt.toordinal() - 1
                prev_date = datetime.fromordinal(prev_dt)
                prev_seed = int(prev_date.strftime("%Y%m%d"))
                
                # Get previous day's color index
                np.random.seed(prev_seed)
                prev_color_idx = np.random.randint(0, len(COLOR_SCHEMES))
                
                # Reset seed to current day
                np.random.seed(seed_val)
                color_idx = np.random.randint(0, len(COLOR_SCHEMES))
                
                # If same as yesterday, pick next one
                if color_idx == prev_color_idx:
                    color_idx = (color_idx + 1) % len(COLOR_SCHEMES)
        except ValueError:
            pass

    alive_color, dead_color = COLOR_SCHEMES[color_idx]
    print(f"Color scheme: Alive={alive_color}, Dead={dead_color}")
    
    # Save color info to rule_info.txt
    if info_file.exists():
        with open(info_file, 'a') as f:
            f.write(f"Colors_Alive: {alive_color}\n")
            f.write(f"Colors_Dead: {dead_color}\n")
    
    # Get date for title (from seed if valid date, otherwise today)
    date_str = datetime.now().strftime("%d %B %Y")
    if seed_val is not None:
        try:
            seed_str = str(seed_val)
            if len(seed_str) == 8:
                dt = datetime.strptime(seed_str, "%Y%m%d")
                date_str = dt.strftime("%d %B %Y")
        except ValueError:
            pass
    
    # Setup figure
    fig, ax = plt.subplots(figsize=(7, 7))
    ax.axis('off')
    
    # Create custom colormap
    cmap = create_custom_colormap(alive_color, dead_color)
    
    if max_state <= 1:
        display_frames = frames
        vmin, vmax = 0, 1
    else:
        display_frames = []
        for frame in frames:
            display_frame = np.zeros_like(frame, dtype=float)
            display_frame[frame == 0] = 0.0
            display_frame[frame == 1] = 1.0
            for state in range(2, max_state + 1):
                blend = 1.0 - ((state - 1) / max_state)
                display_frame[frame == state] = blend
            display_frames.append(display_frame)
        vmin, vmax = 0, 1
    
    im = ax.imshow(display_frames[0], cmap=cmap, interpolation='nearest', vmin=vmin, vmax=vmax)
    
    fig.text(0.5, 0.98, 'Daily Cellular Automata', 
             ha='center', va='top', fontsize=16, fontweight='bold',
             transform=fig.transFigure)
    fig.text(0.5, 0.95, date_str, 
             ha='center', va='top', fontsize=14, fontweight='normal',
             transform=fig.transFigure)
    
    alive_count = np.sum(first_frame == 1)
    stats = ax.text(0.5, -0.05, f'Generation 0 | Alive: {alive_count}',
                   horizontalalignment='center',
                   verticalalignment='top',
                   transform=ax.transAxes,
                   fontsize=12)
    
    # Activity Score below stats
    ax.text(0.5, -0.08, f'Activity Score: {activity_str}',
           horizontalalignment='center',
           verticalalignment='top',
           transform=ax.transAxes,
           fontsize=12)
    
    last_printed = [-1]
    
    def update(frame_idx):
        if frame_idx % 100 == 0 and frame_idx != last_printed[0]:
            print(f"Processing frame {frame_idx}/{len(files)}")
            last_printed[0] = frame_idx
        
        grid = display_frames[frame_idx]
        alive = np.sum(frames[frame_idx] == 1)
        im.set_array(grid)
        stats.set_text(f'Generation {frame_idx} | Alive: {alive}')
        return [im, stats]
    
    print("\nCreating animation...")
    anim = animation.FuncAnimation(
        fig, update, frames=len(files), 
        interval=1000/fps, blit=True
    )
    
    # Ensure output directory exists
    Path(output_file).parent.mkdir(parents=True, exist_ok=True)
    
    print(f"Saving video to {output_file} ({fps:.2f} fps, ~{len(files)/fps:.1f}s duration)...")
    save_start = time.time()
    
    # Configure high-quality writer
    # - bitrate=5000: High bitrate for quality
    # - crf=18: Visually lossless quality (lower is better)
    # - preset=slow: Better compression efficiency
    # - pix_fmt=yuv420p: Ensures compatibility with Telegram/QuickTime
    writer = animation.FFMpegWriter(
        fps=fps,
        metadata=dict(artist='Daily Cellular Automata'),
        bitrate=5000,
        extra_args=['-vcodec', 'libx264', '-pix_fmt', 'yuv420p', '-preset', 'slow', '-crf', '18']
    )
    
    anim.save(output_file, writer=writer, dpi=100)
    save_time = time.time() - save_start
    
    total_time = time.time() - start_time
    
    print(f"\nDone!")
    print(f"Frame loading time: {load_time:.2f} seconds")
    print(f"Video rendering time: {save_time:.2f} seconds")
    print(f"Total execution time: {total_time:.2f} seconds")
    plt.close()

if __name__ == '__main__':
    if len(sys.argv) < 2:
        print("Usage: python render_video.py <input_folder> [output_file] [fps|duration=X] [max_frames]")
        sys.exit(1)
    
    input_folder = sys.argv[1]
    output_file = sys.argv[2] if len(sys.argv) > 2 else 'automata.mp4'
    
    fps = None
    duration = None
    if len(sys.argv) > 3:
        param = sys.argv[3]
        if param.startswith('duration='):
            duration = float(param.split('=')[1])
        else:
            fps = float(param)
    
    max_frames = int(sys.argv[4]) if len(sys.argv) > 4 else None
    
    create_video(input_folder, output_file, fps, max_frames, duration)
