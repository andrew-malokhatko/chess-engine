# Chess Engine

A lightweight chess engine built from scratch in C++. It plays at an estimated strength of 1300–1400 Elo, making it a solid casual opponent.
The engine features a simple GUI built with raylib and is easy to build using CMake for cross-platform support.

![Chess-Engine](https://github.com/user-attachments/assets/a06b4ad3-32a0-4fb9-8ce9-e39a60247188)


# Build & run
Prerequisites:
- C++17 or later
- raylib (either installed system-wide or built as part of the project)
- CMake (version 3.10 or newer)

First clone the repository:
```
git clone https://github.com/andrew-malokhatko/chess-engine
```

Then build with CMake:
```
mkdir build
cd build
cmake ..
cmake --build .
```

Finally run with:
```
./chess-engine
```

# List of Features
Game Controls: Options to undo moves, start a new game, play against a human, or engage in a blitz game.
Board Customization: A "Flip Board" feature to switch the board's perspective.
Analysis Tools:

    Threat Maps: Displays threat maps for both White and Black pieces.
    Check Masks: Highlights check threats for both White and Black.
    Pin Masks: Identifies pinned pieces for both sides.

Opening Book: Access to book moves for standard opening strategies.
Engine Metrics:

    Depth, evaluation, pruned nodes, transposition table hits, and visited nodes are tracked (all currently at 0 in the image).

Time Management: Timers for both players (set to 3:00 each in the image).
FEN Support: Option to view or input the board position using FEN notation (not completely finished).

# Implementation details

- Developed using **C++** with a custom board representation (see bitboards) to enable efficient move generation, optimized for speed and accuracy.

- Multiple performance and functional tests to optimize and debug legal move generation, ensuring reliable and fast move calculations.

- Resizable and user-friendly interface using **raylib**, providing a smooth and engaging user experience.

- **Search Optimization**  
  - **Move Ordering**  
  - **Iterative Deepening**  
  - **Principal Variation**  

- (**Caching System**) **Transposition Table** to cache repeated game positions, significantly speeding up calculations for recurring scenarios.

- Tailored evaluation function to assess board positions, enabling the engine to make stronger and more strategic moves.

- **Multithreading** to improve performance, allowing parallel processing of search and evaluation tasks.

- Finally used **CMake** for efficient project management and cross-platform compatibility.

**For more Information check out https://www.chessprogramming.org**
