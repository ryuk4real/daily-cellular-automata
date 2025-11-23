import os
import re
import requests
from pathlib import Path

def parse_rule_from_logs():
    # Read the last generated rule from output
    info_file = Path('output/rule_info.txt')
    
    if info_file.exists():
        with open(info_file, 'r') as f:
            lines = f.readlines()
            rule = ""
            generations = "500"
            neighborhood = ""
            colors_alive = ""
            colors_dead = ""
            activity = "N/A"
            
            for line in lines:
                if line.startswith("Rule:"):
                    rule = line.split(":", 1)[1].strip()
                elif line.startswith("Generations:"):
                    generations = line.split(":", 1)[1].strip()
                elif line.startswith("Neighborhood:"):
                    neighborhood = line.split(":", 1)[1].strip()
                elif line.startswith("Colors_Alive:"):
                    colors_alive = line.split(":", 1)[1].strip()
                elif line.startswith("Colors_Dead:"):
                    colors_dead = line.split(":", 1)[1].strip()
                elif line.startswith("Activity:"):
                    activity = line.split(":", 1)[1].strip()
            
            return rule, generations, neighborhood, colors_alive, colors_dead, activity
    
    return "Unknown", "500", "Unknown", "#000000", "#FFFFFF", "N/A"

def format_caption(rule, generations, neighborhood, colors_alive, colors_dead, activity):

    # Parse neighborhood from rule
    if "NM" in rule:
        neighborhood_full = "Moore"
        neighborhood_short = "NM"
    elif "NN" in rule:
        neighborhood_full = "Von Neumann"
        neighborhood_short = "NN"
    else:
        neighborhood_full = "Unknown"
        neighborhood_short = "?"
    
    # Determine activity type
    activity_type = "Dynamic"
    try:
        score = float(activity.replace('%', '').strip())
        if score < 1.0:
            activity_type = "Static"
        elif 5.0 <= score <= 20.0:
            activity_type = "Structured"
        elif score > 30.0:
            activity_type = "Chaotic"
    except ValueError:
        activity_type = "Unknown"
    
    caption = f"""**> Daily Cellular Automata**
                    **Rule:** `{rule}`
                    **Generations:** {generations}
                    **Neighborhood:** {neighborhood_full} ({neighborhood_short})
                    **Activity Score:** {activity} ({activity_type})
                    **Colors:** {colors_alive} (alive) - {colors_dead} (dead)
                """
    return caption

def send_video_to_telegram(bot_token, channel_id, video_path, caption):
    url = f"https://api.telegram.org/bot{bot_token}/sendVideo"
    
    with open(video_path, 'rb') as video_file:
        files = {'video': video_file}
        data = {
            'chat_id': channel_id,
            'caption': caption,
            'parse_mode': 'Markdown'
        }
        
        response = requests.post(url, files=files, data=data)
        
        if response.status_code == 200:
            print("[OK] Video posted successfully to Telegram!")
            return True
        else:
            print(f"[ERROR] Failed to post video: {response.status_code}")
            print(f"Response: {response.text}")
            return False

def main():
    bot_token = os.environ.get('TELEGRAM_BOT_TOKEN')
    channel_id = os.environ.get('TELEGRAM_CHANNEL_ID')
    
    if not bot_token or not channel_id:
        print("[ERROR] Error: TELEGRAM_BOT_TOKEN and TELEGRAM_CHANNEL_ID must be set")
        return
    
    video_path = 'automata.mp4'
    
    if not Path(video_path).exists():
        print(f"[ERROR] Error: Video file {video_path} not found")
        return
    
    # Parse rule information
    rule, generations, neighborhood, colors_alive, colors_dead, activity = parse_rule_from_logs()
    
    # Format caption
    caption = format_caption(rule, generations, neighborhood, colors_alive, colors_dead, activity)
    
    print("> Posting video to Telegram...")
    print(f"Caption:\n{caption}")
    
    # Send video
    send_video_to_telegram(bot_token, channel_id, video_path, caption)

if __name__ == '__main__':
    main()
