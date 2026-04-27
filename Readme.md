# 编译/发布过程 LOG

## 1、项目结构

```txt
my_project/
├── pyproject.toml         # Python 打包配置文件
├── CMakeLists.txt         # C++ 编译逻辑
├── src/
│   ├── xxx.cpp            # C++ 源代码
│   ├── xxx.h              # C++ 源代码
│   └── binding.cpp        # pybind11 定义的接口代码
└── enc2db/
    └── __init__.py        # python 接口

```

## 2、写 CMakeLists.txt

#[include](CMakeLists.txt)


## 3、写 pyproject.toml

#[include](pyproject.toml)

## 4、写 binding.cpp 

binding.cpp 提供了最终 python 可调用的核心 cpp 函数，采用 `PYBIND11_MODULE` 去做一些映射，比如把 vector 映射成 list，把 map 映射成 dict。

#[include](src/binding.cpp)

## 5、写 __init__.py

__init__.py 是最终暴露给 python 的接口

#[include](enc2db/__init__.py) 


## 6、编译

### 6.1 本地 windows 编译

**1 前置环境：**
- VS2022：提供 windows cpp 编译环境
- CMake
- Git
- Python

**2 利用 vcpkg 安装依赖：**

```bash
# 设置终端的临时代理
set HTTPS_PROXY=http://127.0.0.1:7890
set HTTP_PROXY=http://127.0.0.1:7890

cd D:\
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat

# 安装依赖（x64，这步比较慢，耐心等待）
.\vcpkg.exe install gdal:x64-windows
.\vcpkg.exe install sqlite3:x64-windows
.\vcpkg.exe install geos:x64-windows
.\vcpkg.exe install libspatialite:x64-windows

# 全局集成（让 CMake 自动找到这些库）
.\vcpkg integrate install
```

**3 编译：**
这步可以直接把项目编译为 pyd，即 cpp 项目的 python 绑定

```bash
mkdir build
cd build

cmake .. -DCMAKE_TOOLCHAIN_FILE=D:/UyNauhL/vcpkg/scripts/buildsystems/vcpkg.cmake -DCMAKE_BUILD_TYPE=Release -Dpybind11_DIR="D:\UyNauhL\ENC\enc2db\.venv\lib\site-packages\pybind11\share\cmake\pybind11"

cmake --build . --config Release
```

**4 wheel打包**

我这里scikit-build-core不太好用 CLI 模式，所以这里用 python 命令打包 wheel

```bash
cd D:\UyNauhL\ENC\enc2db
python -c "import scikit_build_core.build; scikit_build_core.build.build_wheel('dist')"
```

最终输出 .whl 文件，用户可以直接 pip install ../path/../dist/enc2db-0.1.0-py3-none-any.whl 安装 enc2db


### 6.2 跨平台编译

基于 GitHub Actions 实现跨平台编译

