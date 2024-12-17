# repository
https://github.com/google/dawn?tab=readme-ov-file

# build
https://github.com/google/dawn/blob/main/docs/quickstart-cmake.md

# git hash
052be48b9a1a75abfc88123238439d5fd647355c

# PowerShell Proxy
$env:all_proxy="socks5://127.0.0.1:7890"

# Cmake
## config
```
cmake -S . -B out -DDAWN_FETCH_DEPENDENCIES=ON -DDAWN_ENABLE_INSTALL=ON -DCMAKE_BUILD_TYPE=Release
```
## Build
```
cmake --build out --config Release
```
## install
```
cmake --install out/Release --prefix install/Release
```