# Installing dnf2b

## Requirements

* C++17 compiler
    * If you're running this on a Raspberry Pi or another device without proper stdlib support for C++17, and you have no idea how to (safely) upgrade GCC's stdlib when there's nothing in the repos, install Clang and the corresponding version of libc++. Clang installs alongside GCC, and an up-to-date libc++ can be acquired more easily (in my biased experience)
* CMake 3.10 or newer
* Linux
* For firewall blocking: ufw. see the "Setting up the environment" section.

## Installing

### Clang with libc++

If you're running on an RPi or another device without support for GCC's stdlib, or for some other reason feel like forcing libc++ over libstdc++, this is done by adding a flag when running cmake: `-DUSE_LIBCPP=ON`

The rest of the process is fully identical to everything else; just replace the second command with `cmake .. -DUSE_LIBCPP=ON`

### Everything else

As is tradition:
```
mkdir build && cd build
cmake ..
make -j 4
sudo make install
```

## Basic setup


