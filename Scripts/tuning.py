import math
import sys
import random


# Just Intonation ratios for one octave, relative to the tonic (0..11):
#   index 0  → unison (1/1)
#   index 1  → minor 2nd (25/24)
#   index 2  → major 2nd (9/8)
#   ...
#   index 11 → major 7th (15/8)
JI_RATIOS = [
    1/1,      # 0: unison
    25/24,    # 1: minor 2nd
    9/8,      # 2: major 2nd
    6/5,      # 3: minor 3rd
    5/4,      # 4: major 3rd
    4/3,      # 5: perfect 4th
    45/32,    # 6: diminished 5th (tritone)
    3/2,      # 7: perfect 5th
    8/5,      # 8: minor 6th
    5/3,      # 9: major 6th
    9/5,      # 10: minor 7th
    15/8      # 11: major 7th
]

def note_frequencies_just_intonation(N, key):
    """
    Print N consecutive notes (from C0 upward) in Just Intonation,
    but force the chosen 'key' to match its 12-TET frequency exactly.

    - N: total notes to generate, starting from C0 (i = 0,1,2,...,N-1)
    - key: 0 = C, 1 = C#, ..., 11 = B
    """
    # C0 in 12-TET (A4 = 440 Hz) is:
    C0_FREQ = 16.351597831287414

    for i in range(N):
        # i is the “absolute” semitone index above C0.  i=0→C0, i=1→C#0, …, i=12→C1, etc.
        semitone = i % 12
        octave_index = i // 12

        # Find the nearest “tonic‐index” (i_tonic) at or below i:
        #   o_tonic = floor((i – key) / 12)
        o_tonic = (i - key) // 12
        i_tonic = o_tonic * 12 + key
        #
        # Now, i_tonic is the semitone index of the tonic (e.g. F in octave o_tonic),
        # which we’ll force to be exactly its 12-TET pitch.

        # 1) Compute 12‐TET frequency of that tonic:
        f_tet_tonic = C0_FREQ * (2 ** (i_tonic / 12))

        # 2) Within the current octave (i_tonic → i_tonic+11), find the JI‐ratio index:
        di = i - i_tonic  # between 0 and 11
        ratio = JI_RATIOS[di]

        # 3) The actual JI frequency for “note i” is:
        freq_ji = f_tet_tonic * ratio

        print(f"{freq_ji:.16f}f,")


def note_frequencies_from_c0(N):
    A4_INDEX = 57  # Index of A4 if C0 is index 0
    A4_FREQ = 440.0

    for i in range(N):
        # Using the formula: f = 2^((n - A4_INDEX)/12) * A4_FREQ
        freq = 2 ** ((i - A4_INDEX) / 12) * A4_FREQ
        print(f"{freq:.16f}f,")

def note_frequencies_from_c0_rnd(N):
    A4_INDEX = 57   # Index of A4 if C0 is index 0
    A4_FREQ  = 440.0

    for i in range(N):
        base_freq = 2 ** ((i - A4_INDEX) / 12) * A4_FREQ
        cents_offset = random.uniform(-12, 12)
        detune_factor = 2 ** (cents_offset / 1200)
        freq = base_freq * detune_factor

        print(f"{freq:.16f}f,")

def note_frequencies_from_c0_24TET(N):
    C3_FREQ = 65.40639132512

    for i in range(N):
        freq = 2 ** (i / 24) * C3_FREQ
        print(f"{freq:.16f}f,")

if __name__ == "__main__":
    N   = int(sys.argv[1])
    if len(sys.argv) == 3:
        key = int(sys.argv[2]) % 12
        note_frequencies_just_intonation(N, key)
    else:
        note_frequencies_from_c0_24TET(N)
