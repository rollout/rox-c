#!/bin/sh

DEFAULT_INSTALL_DIR=/usr/local
INSTALL_DIR=$DEFAULT_INSTALL_DIR
INSTALL_SUBDIR=rollout-sdk

PROJECT_NAME="ROX SDK"

SKIP_CLEAN=0
SKIP_INSTALL=0
SKIP_BUILDING_THIRD_PARTY_LIBS=0

while getopts ":SCId:" opt; do
  case ${opt} in
    S ) # skip building third party libs
      SKIP_BUILDING_THIRD_PARTY_LIBS=1
      ;;
    C ) # don't clean directories
      SKIP_CLEAN=1
      ;;
    I ) # skip installation step
      SKIP_INSTALL=1
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

INSTALL_PREFIX=${INSTALL_DIR}/${INSTALL_SUBDIR}

if [ ! $SKIP_BUILDING_THIRD_PARTY_LIBS ]; then
  echo "Building third party libraries..."
  cd vendor || exit
  if [ ! $SKIP_CLEAN ]; then
    rm -rf build
    mkdir build
  fi
  cd build || exit
  cmake ..
  make
fi

echo "Building ${PROJECT_NAME}..."
cd ../
if [ ! $SKIP_CLEAN ]; then
  rm -rf build
  mkdir build
fi
cd build || exit

if [ ! $SKIP_INSTALL ]; then
  echo "Installing ${PROJECT_NAME} into ${INSTALL_PREFIX}."
  make install
  ldconfig "${INSTALL_DIR}/lib"
  echo "${PROJECT_NAME} successfully installed into ${INSTALL_PREFIX}."
fi

echo "${PROJECT_NAME} installation finished."
