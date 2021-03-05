# Installing ROX C SDK

## Linux & MacOS

### Prerequisites

1. CMake version 3.9 or higher.
2. GCC
3. Git

On MacOS they could be installed using Homebrew (https://brew.sh/):

```
brew install cmake
brew install gcc
brew install git
```

### Installation

Clone ROX C SDK Git repository:

```
git clone https://github.com/rollout/rox-c.git
```

Run install script:

```
cd rox-c && ./install.sh -d /path/to/install/dir
```

This will install the rollout c sdk into the subdirectory called rollout-sdk of the 
directory specified in the command. So it would look something 
like `/path/to/install/dir/rollout-sdk`.

Please note that parameter `-d` is optional. If omitted, SDK will be installed 
into `/usr/local/rollout-sdk` (which requires superuser privileges).

By default, server SDK is built and installed which has no flag freeze
and flag overrides features. To install client SDK version containing these features,
add `-C` argument to the command:

```
./install.sh -C -d /path/to/install/dir
```

## Windows 

### Prerequisites

ROX C SDK uses `OpenSSL` and `Curl` libraries which aren't provided for Windows. 
So they are downloaded and built from sources during the installation process as well as
other libs such as `PCRE2`, `zlib`, `cJSON` and `pthreads-win32`.    

To make sure these libraries can be built on Windows the following tools should be 
installed first:

1. Strawberry Perl (http://strawberryperl.com/)
2. NASM (https://www.nasm.us/) - executable should be added to PATH after installation. 

Both are needed for building `OpenSSL` library.

Also these tools are required for running the build:

1. CMake 3.9 or higher.
2. Visual Studio with C compiler tools installed.
3. Git

### Installation
 
1. Open `Cross Tools Command Prompt for VS 20XX <(Your Visual Studio Version here)`.
2. Navigate to the repository root folder.
3. Run `install <INSTALLATION_PATH>` where `<INSTALLATION_PATH>` is where you want to install the SDK 
(if omitted, it will be installed into `C:\Program Files\rollout-sdk` which may require Administrator permissions).
4. (optional) add `<INSTALLATION_PATH>\bin` to system `PATH` variable.
