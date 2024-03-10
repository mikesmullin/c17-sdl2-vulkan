# Game 6

This is a prototype as I explore programming language, libraries, and tech stack viability for future game titles.

My intent for this work is:
- to achieve a high degree of control and autonomy over the entire tech stack
- to further refine my skill and appreciation for the engine layers (system level software, and hardware)
- to impart a path for differentiation through innovation and discovery in unusual and unconventional approaches
- to manifest independent game production and portfolio expansion
- to realize my desire to create, self-actualize, and contribute back to the industry that I love
- to help others do the same

## Prerequisites
- **GPU** supporting Vulkan
- Recent version of **Windows**, Mac, or Linux OS in 64-bit architecture
- [**Vulkan SDK**](https://www.lunarg.com/vulkan-sdk/) (recommended)
- **Node.js** (for build scripts)
- **Clang** (recommended compiler)

## Screenshot
![screenshot](docs/imgs/screenshot1.png)

## Video
[![video](docs/video/2024-03-10_Survival.gif)](docs/video/2024-03-10_Survival.mp4)


## Building

### on Windows
   ```
   C:\> node build_scripts/Makefile.mjs all
   ```

### on Linux
```bash
# Install Vulkan SDK
wget -qO- https://packages.lunarg.com/lunarg-signing-key-pub.asc | sudo tee /etc/apt/trusted.gpg.d/lunarg.asc
sudo wget -qO /etc/apt/sources.list.d/lunarg-vulkan-jammy.list http://packages.lunarg.com/vulkan/lunarg-vulkan-jammy.list
sudo apt update
sudo apt install vulkan-sdk vulkan-tools vulkan-validationlayers-dev spirv-tools

# Install SDL2
sudo apt install libsdl2-dev

# Install Clang compiler
sudo apt install clang

# Install Node.js
curl -o- https://raw.githubusercontent.com/nvm-sh/nvm/v0.39.7/install.sh | bash
cd build_scripts/
nvm install
npm install
cd ..

# Build
node build_scripts/Makefile.mjs all
```

### on Mac
```bash
# Install Vulkan SDK
https://sdk.lunarg.com/sdk/download/1.3.275.0/mac/vulkansdk-macos-1.3.275.0.dmg

# Install SDL2
brew install sdl2

# Install Node.js
curl -o- https://raw.githubusercontent.com/nvm-sh/nvm/v0.39.7/install.sh | bash
cd build_scripts/
nvm install
npm install
cd ..

# Build
node build_scripts/Makefile.mjs all
```

## Debugging
- Can use VSCode (see `.vscode/tasks.json`), or;
- Can debug with `gdb`

## Inspirations

- **Alex Austin** ([@crypticsea](https://twitter.com/crypticsea))
  - Mar 25, 2021 - Steam ["Sub Rosa"](https://store.steampowered.com/app/272230/Sub_Rosa/) (early access game)
  - Sep 1, 2023 - YouTube ["How I make games in C"](https://www.youtube.com/watch?v=u2JRIdHhcic) (Part 1)
  - Feb 28, 2024 - YouTube ["How I make games in C part 2"](https://www.youtube.com/watch?v=CI-QriinX8o)

- **Klei Entertainment** ([forums](https://forums.kleientertainment.com/forums/forum/73-dont-starve-together/))
  - Apr 21, 2016 - Steam ["Don't Starve Together"](https://store.steampowered.com/app/322330/Dont_Starve_Together/) (game)