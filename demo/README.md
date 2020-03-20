# Running demo

## Linux & MacOS

If Rollout C SDK is installed into `/usr/local` then just run

```
./build.sh
```

Otherwise specify the full path to the installation directory:

```
ROLLOUT_SDK_ROOT=/path/to/install/dir/rollout-sdk ./build.sh
```

After building the project run:

```
cd build/release
# on Linux it would be LD_LIBRARY_PATH=/path/to/install/dir/rollout-sdk/lib
DYLD_LIBRARY_PATH=/path/to/install/dir/rollout-sdk/lib ROLLOUT_MODE=QA ./rollout_basic_demo
```

Note that `LD_LIBRARY_PATH` (on Linux) and `DYLD_LIBRARY_PATH` on MacOS is only needed when 
ROX SDK is installed in custom directory.

## Windows

In case when Rollout C SDK is installed in the custom directory run:

```
set ROLLOUT_SDK_ROOT=\path\to\rollout-sdk
```

then in all cases run:

```
build
```

To run the demo change to the `build\release` directory and run 

```
set ROLLOUT_MODE=QA
rollout_basic_demo
```