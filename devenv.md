# Development environment

## OS X Catalana

 - install latest xcode
 - install cmake >=3.17 using i.e. brew
 - add you to the `_developer` group `sudo dscl . append /Groups/_developer GroupMembership`
 - `sudo /usr/sbin/DevToolsSecurity --enable`

 ### Debug profiler's unit-tests

 Open `tests/Profiler.Native.Tests` in vscode, set a breakpoint and run the `gtest` target.

 ### Debug the profiler

  - Checkout `https://github.com/dotnet/cli/tree/release/3.1.4xx` into `/opt/cli`, run build.sh.
  - Open `src/profiler` in vscode.
  - Run `cmake -DCMAKE_BUILD_TYPE=Debug . && make` to build the profile so.
  - `dotnet tool install -g Interception.Generator --version 1.2021.228 `
  - Create `/tmp/profiler.json` with `Interception.Generator`.
  - Change `args` and `cwd` as you need in `.vscode/launch.json`.
  - Set the breakpoints and start the `Launch` target.