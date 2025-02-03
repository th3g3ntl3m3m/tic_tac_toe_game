# Tic Tac Toe 1.0: The Ultimate Brain Battle!

## Overview

Welcome to Tic Tac Toe 1.0, a console-based game that reinvents the classic Tic Tac Toe with exciting features:

- **Console Gameplay:** Enjoy the traditional Tic Tac Toe experience directly in your terminal.
- **AI Bot Challenge:** Face off against an intelligent bot with adjustable difficulty levels from 1 (easy) to 10 (hard).
- **Customizable Board:** Choose your preferred board size for a personalized gaming experience.
- **Advanced Caching Options:**  
  - **LRU Cache:** Utilize a Least Recently Used caching mechanism to efficiently manage game state.
  - **Standard Cache:** Alternatively, use a straightforward caching approach.
  - **Configurable Cache Size:** Specify the cache size based on your performance needs.

## Dependencies

No additional external dependencies are required for this project. The game uses the standard C++ library along with common build tools:

- **CMake**
- **GCC/G++ (or any compatible C++ compiler)**

## Building on Ubuntu

### Prerequisites

Ensure you have the following packages installed on your Ubuntu system:

```bash
sudo apt update
sudo apt install build-essential cmake
```

### Build Instructions

1. **Clone the repository or download the source code.** The project directory should be structured as follows:

    ```
    .
    ├── CMakeLists.txt
    ├── version.in
    ├── include
    │   └── LRUCache.hpp
    └── src
        └── TicTacToe.cpp
    ```

2. **Create a build directory and navigate into it:**

    ```bash
    mkdir build && cd build
    ```

3. **Configure the project with CMake:**

    ```bash
    cmake ..
    ```

4. **Build the project:**

    ```bash
    cmake --build .
    ```

After a successful build, the executable `TicTacToe` will be created in the build directory.

## Running the Game

To launch the game, simply run the executable from the build directory:

```bash
./TicTacToe
```

### Game Startup Options

Upon starting the game, you will be prompted with several configuration options. Here's what to expect and how to choose:

1. **Board Size:**  
   - **Prompt:** *"Enter board size:"*  
   - **Action:** Input the desired size of the board (for example, entering `3` for a standard Tic Tac Toe board).

2. **AI Bot Difficulty:**  
   - **Prompt:** *"Enter AI bot difficulty (1 - 10):"*  
   - **Action:** Enter a number between 1 and 10. This sets the difficulty level of the bot. If you enter a value greater than 10 or less than 1, the game will adjust it to fit within the allowed range.

3. **Cache Usage:**  
   - **Prompt:** *"Use cache? (y/n):"*  
   - **Action:** Type `y` (or `Y`) if you want to enable caching to speed up game computations, or `n` if you do not wish to use caching.

4. **Cache Size (if caching is enabled):**  
   - **Prompt:** *"Enter cache size in megabytes:"*  
   - **Action:** Input the desired cache size in megabytes. This allows you to control the memory allocated for caching.

5. **LRU Cache Option (if caching is enabled):**  
   - **Prompt:** *"Use LRU cache? (y/n):"*  
   - **Action:** Enter `y` (or `Y`) to use the LRU (Least Recently Used) cache mechanism, or `n` if you prefer the standard cache implementation.

Follow these on-screen prompts to configure the game according to your preferences. Enjoy the enhanced, customizable experience as you challenge the AI in this modern take on Tic Tac Toe!
