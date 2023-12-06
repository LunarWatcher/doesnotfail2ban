#!/usr/bin/bash

git clone https://github.com/LunarWatcher/doesnotfail2ban dnf2b
cd dnf2b
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX /opt/dnf2b
sudo make -j $(nproc) install
