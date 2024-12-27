# repository
https://github.com/google/dawn?tab=readme-ov-file

https://dawn.googlesource.com/dawn/

# build 
https://github.com/google/dawn/blob/main/docs/quickstart-cmake.md

# Build Version
https://dawn.googlesource.com/dawn/+/refs/heads/chromium/6918

# PowerShell Proxy
$env:all_proxy="socks5://127.0.0.1:7890"

# Cmake
## config
需要安装Python，并Python路径添加到Path环境变量
```
cmake -S . -B out -DDAWN_FETCH_DEPENDENCIES=ON -DDAWN_ENABLE_INSTALL=ON -DCMAKE_BUILD_TYPE=Release
```
## Build
```
cmake --build out --config Release
```
## install
```
cmake --install out --prefix install/Release --config Release
```