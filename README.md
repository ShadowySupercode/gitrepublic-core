# GitRepublic Core

## What is `gitrepublic-core` ?

TODO

## Build
Adding executables and library output is simple. Add the concerned source and header files and link them to the executable.
Then execute the `build.sh` script. Like this:

    ./build.sh -t gitrepublic-core

To build all targets defined by the `CMakeLists.txt`, run the script without any special options:

    ./build.sh

Output binaries are dumped in `out` directory according to the chosen build mode. For `Debug` build mode:
* Binary executables found in `out/bin/Debug`
* Shared libaries found in `out/lib/Debug`

The user could specify the build mode by the setting the `-m` flag of the script with values: `Release|Debug`.
If not specified, default build mode is `Debug`.

The user could clear any existing build by setting the flag `-c` to `All|Release|Debug`. This completely deletes the targeted build to clear. Example:

    ./build -m Release -c All

## Installing Dependencies

TODO