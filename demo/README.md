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
cd build
# on Linux it would be LD_LIBRARY_PATH=/path/to/install/dir/rollout-sdk/lib
DYLD_LIBRARY_PATH=/path/to/install/dir/rollout-sdk/lib ROLLOUT_MODE=QA ./rollout_basic_demo
```

Note that `LD_LIBRARY_PATH` (on Linux) and `DYLD_LIBRARY_PATH` on MacOS is only needed when 
ROX SDK is installed in custom directory.

## Windows

TBD