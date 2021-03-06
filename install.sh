#!/bin/sh

set -e

SCRIPT=$(basename "$0")
DEFAULT_INSTALL_DIR=/usr/local
INSTALL_DIR=$DEFAULT_INSTALL_DIR
INSTALL_SUBDIR=rollout-sdk

PROJECT_NAME="ROX SDK"

DO_CLEAN=0
FIND_LEAKS=0
SKIP_INSTALL=0
SKIP_TESTS=1
BUILD_TYPE=Release
BUILD_SUBDIR=release
SKIP_BUILDING_THIRD_PARTY_LIBS=0
TEST_TIMEOUT_SECONDS=1200
SDK_VERSION=CLIENT

HELP=$(
  cat <<EOF
${SCRIPT} [OPTIONS]
Options:
  -S        Skip building third party libs.
  -C        Skip installing client SDK.
  -I        Skip installation step.
  -d <path> Specify installation directory.
  -s        Enable debug symbols.
  -c        Clean build directories.
  -l        Perform memory leak check using valgrind.
  -t        Run tests.
  -T <sec>  Specify test timeout in seconds default is (${TEST_TIMEOUT_SECONDS}).
EOF
)

while getopts ":SsCcIT:d:lt" opt; do
  case ${opt} in
  S) # skip building third party libs
    SKIP_BUILDING_THIRD_PARTY_LIBS=1
    ;;
  c) # clean build directories
    DO_CLEAN=1
    ;;
  C) # skip client SDK
    SDK_VERSION=SERVER
    ;;
  I) # skip installation step
    SKIP_INSTALL=1
    ;;
  t) # run tests
    SKIP_TESTS=0
    ;;
  T) # specify test timeout
    TEST_TIMEOUT_SECONDS=$OPTARG
    ;;
  d) # specify installation directory
    INSTALL_DIR=$OPTARG
    ;;
  s) # enable debug symbols
    BUILD_TYPE=Debug
    BUILD_SUBDIR=debug
    ;;
  l) # find memory leaks
    FIND_LEAKS=1
    ;;
  \?)
    echo "Usage: ${HELP}"
    ;;
  *)
    echo "Invalid Option: -$OPTARG" 1>&2
    echo "Usage: ${HELP}"
    exit 1
    ;;
  esac
done

if [ "${SKIP_INSTALL}" -ne "1" ] && [ ! -w "${INSTALL_DIR}" ]; then
  echo "Directory ${INSTALL_DIR} is not writable. Trying to install into system directory by non-root user?"
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
  rm -rf build/${BUILD_SUBDIR}
fi
mkdir -p build/${BUILD_SUBDIR}
cd build/${BUILD_SUBDIR} || exit

cmake ../../ \
-DCMAKE_INSTALL_PREFIX="${INSTALL_PREFIX}" \
-DROX_SKIP_TESTS=${SKIP_TESTS} \
-DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
-DROX_FIND_LEAKS=${FIND_LEAKS} \
-DROX_${SDK_VERSION}=Yes

make

if [ "${SKIP_TESTS}" -ne "1" ]; then
  CTEST_COMMAND="CK_DEFAULT_TIMEOUT=${TEST_TIMEOUT_SECONDS} ctest --output-on-failure --timeout ${TEST_TIMEOUT_SECONDS}"
  if [ "${FIND_LEAKS}" -ne "0" ]; then
    CTEST_COMMAND="${CTEST_COMMAND} -D ExperimentalMemCheck"
  fi
  bash -c "${CTEST_COMMAND}"
fi

if [ "${SKIP_INSTALL}" -ne "1" ]; then
  echo "Installing ${PROJECT_NAME} into ${INSTALL_PREFIX}."
  make install
  if [ "${INSTALL_DIR}" = "${DEFAULT_INSTALL_DIR}" ] && [ "${OSTYPE}" = "linux-gnu" ]; then
    echo "Running ldconfig..."
    echo "${INSTALL_PREFIX}/lib" >/etc/ld.so.conf.d/rollout.conf
    ldconfig
  fi
  echo "${PROJECT_NAME} is successfully installed into ${INSTALL_PREFIX}."
else
  echo "Skipping installation."
fi

echo "${PROJECT_NAME} build finished."
