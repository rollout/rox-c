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

This will install the rollout c sdk into the subdirectory called rollout-sdk of the directory specified in the command.
So it would look something like `/path/to/install/dir/rollout-sdk`.

Please note that parameter `-d` is optional. If omitted, SDK will be installed into `/usr/local/rollout-sdk` (which
requires superuser privileges).

By default, client SDK is built and installed which has flag freeze and flag overrides features. To install server-only
SDK without these features, add `-C` argument to the command:

```
./install.sh -C -d /path/to/install/dir
```

## Windows

### Prerequisites

ROX C SDK uses `OpenSSL` and `Curl` libraries which aren't provided for Windows. So they are downloaded and built from
sources during the installation process as well as other libs such as `PCRE2`, `zlib`, `cJSON` and `pthreads-win32`.

To make sure these libraries can be built on Windows the following tools should be installed first:

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

## Releasing

The SDK version is defined in the root `CMakeLists.txt` by `ROX_VERSION_MAJOR`, `ROX_VERSION_MINOR` and
`ROX_VERSION_PATCH`. The SDK reports it to the server as `lib_version` and uses it in the library file name, so the
version must be bumped for every release.

Follow these steps in order:

1. **Merge the code changes.** Merge all the changes you want to release into `master`.
2. **Bump the version in a separate PR.** Open a PR that updates the version in `CMakeLists.txt`, then merge it into
   `master`. Choose the new version number according to the kind of change being released:
   - **Patch** (`X.Y.Z` → `X.Y.Z+1`) for bug fixes.
   - **Minor** (`X.Y.Z` → `X.Y+1.0`) for new, backward-compatible features.
   - **Major** (`X.Y.Z` → `X+1.0.0`) for breaking changes.
3. **Create the tag and push it.** Once the version bump PR is merged, update your local `master`, create a tag that
   matches the new version on the merge commit, and push the tag:

   ```
   git checkout master
   git pull
   git tag vX.Y.Z
   git push origin vX.Y.Z
   ```

   The tag (`vX.Y.Z`) must match the version in `CMakeLists.txt`.
4. **Update the changelog.** Add the release to the
   [C/C++ changelog](https://docs.cloudbees.com/docs/cloudbees-unify-changelog/latest/fmsdk/c-cpp-changelog).