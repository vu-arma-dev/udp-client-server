# udp_client_server (Windows Implementation)
Bare-bones C++ UDP implementation for ARMA Lab robot control. This is a version of the code in `main` indented for windows 

Based on the implementation found [here](https://linux.m2osw.com/c-implementation-udp-clientserver), which is part of the [Snap!](https://snapwebsites.org/) C++ implementation.

# udp_utils
This is a wrapper around `udp_client_server` that provides some typedefs and static methods that make the library easier to use. Unless you need the flexibility of `udp_client_server`, use `udp_utils` and do not use `udp_client_server` directly. 

# Installation 
First make sure you have cmake and gcc/g++ installed. 

```
git clone -b windows --single-branch https://github.com/vu-arma-dev/udp_client_server.git

# Example Code
An example of using `udp_utils` is provided in `src/udp_example.cpp`. 

## Compiling
To Compile this example code, create a `build` folder in the root of this codebase. Then, from the root folder, run in cmd:

```
cmake -S . -B .\build\ -G"Ninja"
cmake --build .\build\
```
You can then run the code by double-clicking on `udp_example.exe` from the file explorer or by running:
```
cd ./build/
udp_example.exe
```