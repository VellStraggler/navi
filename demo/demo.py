"""
5/26: 30 minutes
5/28: 4:45 minutes
5/29: 2:45 hour
5/30: 2:30 minutes
https://chatgpt.com/share/683042df-6154-8000-b505-c21223c7b418
"""
import random
from threading import Thread
import time
import pygame
from pydub import AudioSegment
import numpy as np
import pygame.rect
import librosa
from flit import Flit
from blot import Blot
FRAME_RATE = 30
SEED = 42

def __main__():
    audio_file = "demo/audio.wav"
    resolution = (1920, 1080)
    reduced_resolution = (720, 480)

    # audio_data, sample_rate = read_audio_file(audio_file)
    audio_frames_info = read_audio_components(audio_file)
    # Initial video generation should be low-quality for preview
    # video_preview = generate_video(audio_components, reduced_resolution, seed)
    spawnpoint = (reduced_resolution[0] // 2, reduced_resolution[1] // 2)
    flits = get_flits(spawnpoint, SEED, reduced_resolution)
    display_preview(flits, audio_frames_info, reduced_resolution, audio_file)
    # Render the video at full resolution when prompted
    # input("Press Enter to render the full-resolution video...")
    # video_render = generate_video(components, resolution, seed)
    # save_video(video_render, "demo/output.mp4", audio_file)
    # print("Video saved to demo/output.mp4")

def read_audio_file(file_path):
    """
    Reads an audio file and returns the audio data and sample rate.
    """
    audio_data, sample_rate = librosa.load(file_path, sr=None)
    return audio_data, sample_rate

"""Divide audio into separate audio tracks of all instruments"""
def read_audio_components(audio_file):
    print("Reading audio components...")
    # get number of instruments
    # i.e. voice, guitar, piano, drums
    volumes = get_volumes(audio_file)
    pitches = get_highest_pitches(audio_file)
    tempo = get_tempo(audio_file)
    min_frames = min(len(volumes), len(pitches))
    beats = get_beat_oscillations(tempo, min_frames)
    print("Audio components read.")
    frame_information = []
    total_frames = min_frames
    for i in range(total_frames):
        info = []
        info.append(volumes[i])
        info.append(pitches[i])
        info.append(beats[i])
        # Add other information about the frame here
        # e.g. pitch, timbre, etc.
        frame_information.append(info)
    return frame_information

def get_volumes(audio_file):
    sound = AudioSegment.from_file(audio_file)
    frame_ms = 1000/FRAME_RATE
    volumes = []
    for i in range(0, len(sound), int(frame_ms)):
        frame = sound[i:i + int(frame_ms)]
        volumes.append(frame.dBFS + 30 if frame.dBFS != float("-inf") else -30)
    return volumes

def get_highest_pitches(audio_file):
    start_time = time.time()
    y, sample_rate = librosa.load(audio_file)
    frame_length = int(sample_rate / FRAME_RATE)
    thread_count = 2
    # THREADS are SLOWING DOWN THE PROCESSING
    all_pitches = [[] for _ in range(thread_count)]
    max = 0
    # Get 4 threads to process the audio
    threads = []
    for j in range(thread_count):
        start = j * (len(y) // thread_count)
        end = (j + 1) * (len(y) // thread_count) if j < thread_count-1 else len(y)
        pitches = all_pitches[j]
        thread = Thread(target=process_audio_segment, args=(start, end, y, sample_rate, pitches, max, frame_length))
        threads.append(thread)
        thread.start()

    for thread in threads:
        thread.join()
    
    print("processing all pitches took", time.time() - start_time, "seconds")
    combined_pitches = []
    for pitches in all_pitches:
        combined_pitches.extend(pitches)
    return combined_pitches

def process_audio_segment(start, end, y, sample_rate, pitches, max, frame_length):

    # for i in range(0, len(y), frame_length):
    for i in range(start, end, frame_length):
        frame = y[i:i + frame_length]
        if len(frame) < frame_length:
            break
        f0, voiced_flag, voiced_probs = librosa.pyin(
            frame,
            fmin = librosa.note_to_hz('C2'),
            fmax = librosa.note_to_hz('C7'),
            sr = sample_rate
        )

        # Get the highest pitch in the frame
        valid_pitches = f0[~np.isnan(f0)]
        current_max = np.max(valid_pitches) if len(valid_pitches) > 0 else 0.0
        pitches.append(int(current_max))
        # processed = int(i / len(y) * 100)
        # if processed % 10 == 0:
        #     if processed not in percentages:
        #         print(f"{processed}% of audio processed")
        #         percentages.add(processed)

    print("Pitches processed. Thread finished.")
    return pitches

def get_beat_oscillations(tempos, min_frames):
    """Creates distance from the flit's central position,
    reaching critical distance on the beat (0-1)."""
    heights = []
    for i in range(min_frames):
        # Calculate the oscillation height based on the tempo
        # and the frame index
        height = np.sin(2 * np.pi * tempos / 60 * i / FRAME_RATE)
        heights.append(height)
    return heights

    

def get_tempo(audio_file):
    y, sr = librosa.load(audio_file)
    tempo, _ = librosa.beat.beat_track(y=y, sr=sr)
    if tempo < 60:
        tempo *= 2
    elif tempo > 150:
        tempo /= 2
    return tempo

def generate_video(components, resolution, seed):
    return None

def display_preview(flits, audio_frames_info, resolution, audio):
    pygame.init()
    pygame.mixer.init()
    pygame.mixer.music.load(audio)
    screen = pygame.display.set_mode(resolution)
    clock = pygame.time.Clock()
    running = True
    frame_counter = 0
    input("Press Enter to start the preview...")
    pygame.mixer.music.play()
    while pygame.mixer.music.get_busy():
        screen.fill((0, 0, 0))
        frame_info = audio_frames_info[frame_counter]
        draw_flits(flits, screen)
        update_flits(flits, frame_info)
        cull_blots(flits)
        # Draw the video frame here
        pygame.display.flip()
        clock.tick(FRAME_RATE)  # Limit to 30 FPS
        frame_counter += 1
    pygame.quit()

def cull_blots(flits):
    before = 0
    after = 0
    for flit in flits:
        before += len(flit.blots)
        for i in range(len(flit.blots) - 1, -1, -1):
            if flit.blots[i].counter == 0:
                del flit.blots[i]
        after += len(flits[0].blots)
    if before != after:
        print("Removed", before - after, "blots.")

def draw_blot(blots: list, screen, index):
    blot = blots[index]
    if blot.radius < 256 and blot.is_parent:
        blot.color = (
            255 - blot.radius,
            255 - blot.radius,
            255 - blot.radius
        )
        pygame.draw.circle(screen, blot.color, (blot.x, blot.y), blot.radius)
        blot.radius = min(blot.radius + 2, 256)
    
def draw_tail(blots, screen, index):
    """also randomizes the blot"""
    if (not blots):
        return
    if (len(blots) == 1):
        return
    prevblot = blots[index - 1]
    blot = blots[index]  

    # Straight line blots
    pygame.draw.circle(screen, (254,254,254), (blot.x, blot.y), int(blot.counter))
    # interpolated blot \/
    pygame.draw.circle(screen, (254,254,254), ((prevblot.x + blot.x)//2, (prevblot.y + blot.y)//2), int(blot.counter))

    # "Fun" blots
    pygame.draw.circle(screen, (254,254,254), (blot.xr, blot.yr), min(int(blot.counter), 2))
    pygame.draw.circle(screen, (254,254,254), ((prevblot.xr + blot.xr)//2, (prevblot.yr + blot.yr)//2), min(int(blot.counter), 2))

    blot.update_position()
    blot.counter = max(0, blot.counter - 0.1)
    blot.randomize()

def draw_flits(flits, screen):
    for i in range(len(flits[0].blots)):
        for flit in flits:
            blots = flit.blots
            blot = flit.blots[i]
            if(blot.counter > 1.6 and blot.counter < 14.5 and blot.is_parent and len(blots) < 3000):
                # baby blot!
                baby = Blot(int(blot.x + ((blot.counter//2) * random.randint(-1,1))), int(blot.y+ ((blot.counter//2) * random.randint(-1,1))), 0)
                baby.counter = blot.counter -1
                baby.is_parent = False
                baby.direction = blot.direction
                blots.insert(i,baby)
                i += 1
    for i in range(len(flits[0].blots)):
        for flit in flits:
            draw_blot(flit.blots, screen, i)
    for flit in flits:
        shadow_color = (flit.color[0] // 2, flit.color[1] // 2, flit.color[2] // 2)
        pygame.draw.circle(screen, shadow_color, flit.get_global_position(), flit.radius * 2)
    for i in range(len(flits[0].blots)):
        for flit in flits:
            draw_tail(flit.blots, screen, i)
    for flit in flits:
        pygame.draw.circle(screen, flit.color, flit.get_global_position(), flit.radius)
        new_blot = Blot(int(flit.get_global_position()[0]), int(flit.get_global_position()[1]), flit.radius/2)
        new_blot.direction = flit.direction
        new_blot.flits = flits
        flit.blots.append(new_blot)

def update_flits(flits, frame_info):
    volume = frame_info[0]
    pitches = frame_info[1]
    beats = frame_info[2]
    for flit in flits:
        flit.update_oscillation(beats)
        flit.update_radius(volume)
        flit.update_direction(1)
        flit.update_position()
        flit.update_color()


def save_video(video, output_path, audio_file):
    pass

def get_flits(spawnpoint=(0,0), seed=SEED, resolution=(1920, 1080)):
    flits = []
    flit = Flit((spawnpoint[0] + 100, spawnpoint[1]))
    flit2 = Flit((spawnpoint[0]- 100, spawnpoint[1]))
    flit.add_screen_edge_obstacles(resolution)
    flit.add_flit_as_obstacle(flit2)
    flit2.add_screen_edge_obstacles(resolution)
    flit2.add_flit_as_obstacle(flit)
    # FIXME: Use the random seed to place the flits
    flits.append(flit)
    flits.append(flit2)
    return flits

class Component:
    def __init__(self):
        timeline = Timeline()

class Timeline:
    def __init__(self):
        actions = []
        
        volume = 0.5
        vibrato = 0.0
        reverb = 0.5
        frequency = 440.0
        pitch = 440.0
        tempo = 120.0
        sustain = 0.5
        timbre = 0.5 #unsure of this one

if __name__ == "__main__":
    __main__()