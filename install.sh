#!/bin/sh

set -e

DEFAULT_INSTALL_DIR=/usr/local
INSTALL_DIR=$DEFAULT_INSTALL_DIR
INSTALL_SUBDIR=rollout-sdk

PROJECT_NAME="ROX SDK"

DO_CLEAN=0
SKIP_INSTALL=0
SKIP_TESTS=0
SKIP_BUILDING_THIRD_PARTY_LIBS=0

while getopts ":ScITd:" opt; do
  case ${opt} in
    S ) # skip building third party libs
      SKIP_BUILDING_THIRD_PARTY_LIBS=1
      ;;
    c ) # don't clean directories
      DO_CLEAN=1
      ;;
    I ) # skip installation step
      SKIP_INSTALL=1
      ;;
    T ) # skip testing step
      SKIP_TESTS=1
      ;;
    d ) # specify installation directory
      INSTALL_DIR=$OPTARG
      ;;
    \? ) echo "Usage: install.sh [-S] [-C] [-I] [-d]"
      ;;
  esac
done

if [ ! -w "${INSTALL_DIR}" ]; then
  echo "Directory ${INSTALL_DIR} is not writable. Trying to install into system directlory by non-root user?"
  exit 1
fi

INSTALL_PREFIX="${INSTALL_DIR}/${INSTALL_SUBDIR}"
CWD=$(pwd)

if [ "${SKIP_BUILDING_THIRD_PARTY_LIBS}" -ne "1" ]; then
  echo "Building third party libraries."
  cd vendor || exit
  if [ "${DO_CLEAN}" -ne "0" ]; then
    echo "Cleaning up build directory."
    rm -rf build
  fi
  mkdir -p build
  cd build || exit
  cmake ..
  make
else
  echo "Not building third party libraries."
fi

echo "Building ${PROJECT_NAME}..."
cd "${CWD}" || exit
if [ "${DO_CLEAN}" -ne "0" ]; then
  echo "Cleaning up build directory."
  rm -rf build/release
fi
mkdir -p build/release
cd build/release || exit

cmake ../../ -DCMAKE_INSTALL_PREFIX="${INSTALL_PREFIX}" -DROLLOUT_SKIP_TESTS=${SKIP_TESTS} -DCMAKE_BUILD_TYPE=Release
make

if [ "${SKIP_TESTS}" -ne "1" ]; then
  TEST_TIMEOUT_SECONDS=20
  CK_DEFAULT_TIMEOUT=${TEST_TIMEOUT_SECONDS} ctest --output-on-failure --timeout ${TEST_TIMEOUT_SECONDS}
fi

if [ "${SKIP_INSTALL}" -ne "1" ]; then
  echo "Installing ${PROJECT_NAME} into ${INSTALL_PREFIX}."
  make install
  if [ "${INSTALL_DIR}" = "${DEFAULT_INSTALL_DIR}" ]; then
    echo "${INSTALL_PREFIX}/lib" > /etc/ld.so.conf.d/rollout.conf
    ldconfig
  fi
  echo "${PROJECT_NAME} is successfully installed into ${INSTALL_PREFIX}."
else
  echo "Skipping installation."
fi

echo "${PROJECT_NAME} build finished."
