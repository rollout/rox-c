# rox-c

## Supported Platforms

TBD

## Development

### Prerequisites

#### CMake

https://cmake.org/download/

MIN version tested is TBD.

### Third party libs

Below is the list of third party libraries used in the Software. All of them may be downloaded
and compiled automatically, see "Building third party libraries" section below. However the build 
tries its best in finding the already installed libraries in the system and, if found, to use them 
during compilation.

#### OpenSSL

OpenSSL can be installed locally or built from sources during the 3rd party libraries build.

##### Option 1. Installing OpenSSL locally

TBD

##### Option 2. Building OpenSSL from sources

To build OpenSSL from sources the following tools are needed.

*Windows*

1. Strawberry Perl (http://strawberryperl.com/)
2. NASM https://www.nasm.us/ - executable should be added to PATH after installation. 

*Linux*

TBD

*MacOS*

TBD

TBD: MIN and MAX tested versions.

#### Curl

TBD: MIN and MAX tested versions. 

#### PCRE2

TBD: MIN and MAX tested versions.

#### CJSON

TBD: MIN and MAX tested versions.

#### ZLIB

TBD: MIN and MAX tested versions.

#### Check?

TBD: MIN and MAX tested versions.

### Build

#### Building third party libraries

*Windows*

1. Open `Cross Tools Command Prompt for VS 20XX <(Your Visual Studio Version here)`.
2. Navigate to the ROX SDK directory. 
3. Run these commands:

```
mkdir vendor\build
cd vendor\build
cmake ..
nmake
ctest
```

*Linux & MacOS* 

```
cd vendor && ./build-third-party-libs.sh
```

#### Building ROX SDK

*Windows*

```
cd build
cmake ..
nmake
ctest
```

*Linux & MacOS* 

```
./clean-cmake.sh
```
