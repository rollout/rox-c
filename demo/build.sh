#!/bin/sh

if [ ! -d "${ROX_SDK_ROOT}" ]; then
  ROX_SDK_ROOT=/usr/local/rollout-sdk
fi

echo "Looking for Rollout SDK in ${ROX_SDK_ROOT} (specify ROX_SDK_ROOT variable to override it)"

if [ ! -d "${ROX_SDK_ROOT}" ]; then
  echo "Directory ${ROX_SDK_ROOT} not exists. Existing."
  exit 1
fi

echo "Building Rollout SDK demo..."
rm -rf build/release
mkdir -p build/release
cd build/release || exit
cmake ../.. -DCMAKE_BUILD_TYPE=Release -DROX_SDK_ROOT="${ROX_SDK_ROOT}"
make

echo "Rollout SDK demo build finished."
