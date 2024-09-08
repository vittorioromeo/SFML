# VRSFML — Vittorio Romeo's SFML Fork

> *(This is a fork of SFML by Vittorio Romeo.)*

- [**Major changes and design rationale**](DESIGN.md)

- [**Upstream SFML repository**](https://github.com/SFML/SFML)



## Build Instructions on fresh Ubuntu 24.04

```
COMPILER=gcc-14
PRESET=vrdev_gcc

# Install necessary tools and dependencies
sudo apt-get update
sudo apt-get install -y --no-install-recommends nodejs npm git mesa-utils
sudo apt-get install -y --no-install-recommends libxrandr-dev libxi-dev libxcursor-dev libglfw3-dev libudev-dev libfreetype-dev libogg-dev libvorbis-dev libflac-dev
sudo npm install -g setup-cpp@v0.38.3
sudo setup-cpp --compiler ${COMPILER} --cmake true --ninja true --ccache true --make true

# Export env vars for using tools
sudo -s
source .cpprc
exit
source .cpprc

# Verify tools availability
glxinfo | head -n10
git --version
g++ --version
cmake --version
ninja --version
ccache --version

# Create a working directory
mkdir -p work
cd work

# Clone the repo
git clone https://github.com/menuet/VRSFML.git            # TODO: use https://github.com/vittorioromeo/VRSFML.git once merged

# Cmake Configure and Build
cd VRSFML
git checkout try-fix-build                                # TODO: do not use this line once merged
cmake --preset ${PRESET}
cmake --build build_${PRESET}
```
