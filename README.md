c-dvar
======

D-Bus Variant Type-System

The c-dvar project implements the D-Bus Variant Type-System. It is a simple
stream encoder and decoder, according to the D-Bus Specification. It is a
self-contained implementation centered around the D-Bus Variant Type-System,
suitable for any project handling D-Bus.

### Project

 * **Website**: <https://c-util.github.io/c-dvar>
 * **Bug Tracker**: <https://github.com/c-util/c-dvar/issues>

### Requirements

The requirements for this project are:

 * `libc` (e.g., `glibc >= 2.16`)

At build-time, the following software is required:

 * `meson >= 0.41`
 * `pkg-config >= 0.29`

### Build

The meson build-system is used for this project. Contact upstream
documentation for detailed help. In most situations the following
commands are sufficient to build and install from source:

```sh
mkdir build
cd build
meson setup ..
ninja
meson test
ninja install
```

No custom configuration options are available.

### Repository:

 - **web**:   <https://github.com/c-util/c-dvar>
 - **https**: `https://github.com/c-util/c-dvar.git`
 - **ssh**:   `git@github.com:c-util/c-dvar.git`

### License:

 - **Apache-2.0** OR **LGPL-2.1-or-later**
 - See AUTHORS file for details.
