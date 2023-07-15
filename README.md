## Required:
- pcsx2.exe
- SLUS-20001 (TTT) V2.00

## What this tool does:
1. Attaches to pcsx2.exe
2. Reads upon P1's current bone positions, current move and current frame timer each frame
3. For every new detected move, save the bone position (24*3 axes) of each frame
4. If every frame of the move has been saved, a new file named 'X.tttposes' will be created, file containing every bone position of each frame
5. If only a partial amount of frame from the move have been saved (many animations end earlier than their actual end), the animation will still be saved but partially:
    - Let us imagine a 50 frames move where frame 1-20 have been saved. A file `X__20-50f.tttposes_partial` will be created, containing the first twenty frames of animations.
    - If later on, frame 21 - 30 for that same move are also saved, A file `X__30-50f.tttposes_partial` will be created, containing the first 30 frames of animations.
    - PS: Moves that are 1 or 2 frames long or less will never be saved partially. A move cannot be saved if it's 1st or 2nd frame are missing.

## Exported files format
- Exported files are stored in binary format, here are their content:
    - First 4 bytes are magic bytes for identification purposes
    - Next 4 bytes store the amount of frames the "animation" has (little endian)
    - Next: sizeof(float) * 24 * 3 * ANIMATION_DURATION (little endian)
- The list of float is ordered as follow (each of the bone listed here have 3 values, XYZ, stored in that exact order):
```
    Pivot
    Neck
    Right Eye
    Left Eye
    Right Clavicle
    Right Arm
    Right Elbow
    Right Hand
    Right Forefinger
    Right Little Finger
    Left Clavicle
    Left Arm
    Left Elbow
    Left Hand
    Left Forefinger
    Left Little Finger
    Right Hip
    Right Knee
    Right Foot
    Right Toes
    Left Hip
    Left Knee
    Left Foot
    Left Toes
```

## What to do 
1. Start the game, **MAKE SURE YOU AND THE OPPONENT ARE THE SAME CHARACTER**
2. Start the tool, let it run, do a lot of moves, get hit, etc
Do try to let moves go on as long as possible and avoid cancelling them early by moving or attacking again
