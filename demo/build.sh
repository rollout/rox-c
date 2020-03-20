#!/bin/sh

if [ ! -d "${ROLLOUT_SDK_ROOT}" ]; then
  ROLLOUT_SDK_ROOT=/usr/local/rollout-sdk
fi

echo "Looking for Rollout SDK in ${ROLLOUT_SDK_ROOT} (specify ROLLOUT_SDK_ROOT variable to override it)"

if [ ! -d "${ROLLOUT_SDK_ROOT}" ]; then
  echo "Directory ${ROLLOUT_SDK_ROOT} not exists. Existing."
  exit 1
fi

echo "Building Rollout SDK demo..."
rm -rf build/release
mkdir -p build/release
cd build/release || exit
cmake ../.. -DCMAKE_BUILD_TYPE=Release -DROLLOUT_SDK_ROOT="${ROLLOUT_SDK_ROOT}"
make

echo "Rollout SDK demo build finished."
