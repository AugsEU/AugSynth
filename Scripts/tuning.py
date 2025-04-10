import math
import sys

def note_frequencies_from_c0(N):
    A4_INDEX = 57  # Index of A4 if C0 is index 0
    A4_FREQ = 440.0

    for i in range(N):
        # Using the formula: f = 2^((n - A4_INDEX)/12) * A4_FREQ
        freq = 2 ** ((i - A4_INDEX) / 12) * A4_FREQ
        print(f"{freq:.16f}f,")

if __name__ == "__main__":
    note_frequencies_from_c0(int(sys.argv[1]))
