# MSPM0G3507 现代化嵌入式开发模板

[![Build Status](https://img.shields.io/badge/Build-Passing-brightgreen)](https://github.com/your-username/your-repo)
[![License](https://img.shields.io/badge/License-MIT-blue.svg)](https://opensource.org/licenses/MIT)
[![Powered by](https://img.shields.io/badge/Powered%20by-VSCode%20%7C%20CMake%20%7C%20Docker-blueviolet)](https://code.visualstudio.com/)

这是一个为 **Texas Instruments MSPM0G3507** 系列微控制器打造的、**终极的、跨平台的现代化嵌入式开发环境模板**。它彻底摆脱了传统 IDE 的束缚，拥抱业界领先的开源工具链，旨在提供极致的开发效率和团队协作体验。

---

## ✨ 核心特性

- **⚡ 闪电般的速度**: 从编译到烧录，全流程优化，告别漫长等待。
- **🤖 AI 原生**: 完美集成 **GitHub Copilot** 和 **Clangd**，提供顶级的代码补全和智能感知。
- **🐳 容器化**: 基于 Docker/Podman，实现**一键部署**，确保团队成员拥有 100% 一致的开发环境。
- **💻 跨平台**: 完美兼容主流 **Linux 发行版** 和 **Windows (通过 WSL2)**。
- **🎨 图形化支持**: 无缝的 **X11 转发**，让你可以在容器内流畅使用 SysConfig、J-Flash 等 GUI 工具。
- **🔧 工具链自由**: 支持使用 **TI XDS110 (官方 LaunchPad)** 和 **J-Link** (包括克隆版) 进行烧录和调试。
- **🚀 自动化工作流**:
  - **SysConfig** 图形化配置硬件 -> CMake 自动生成代码。
  - **VS Code** 一键 `F5` 实现“编译 -> 烧录 -> 调试”全流程自动化。
  - 自定义任务实现“只烧录不调试”的快速部署。

---

## 🚀 快速开始

本项目提供了两种使用方式：**完美的容器化方案 (强烈推荐)** 和简陋的 Windows J-Link 烧录法 (用于快速验证)。

### 方案一：完美容器化方案 (Docker / Podman)

这是推荐的、一劳永逸的解决方案。它将所有依赖项封装在一个干净、隔离的环境中。

#### 1. 先决条件

- **宿主机**:
  - 主流 Linux 发行版 (如 Ubuntu, Arch, Fedora) 或 Windows 10/11 (已安装 WSL2)。
  - 已安装 **Docker** 或 **Podman**。
  - 已安装 **VS Code** 及 **Remote - Containers** 或 **Remote - SSH** 扩展。
  - (Linux) 确保 X11 服务器正在运行。
  - (WSL2) 确保已安装并配置 [WSLg](https://docs.microsoft.com/en-us/windows/wsl/tutorials/gui-apps)，以便支持 GUI 应用。

#### 2. 部署与启动

> **注意**: 我们的团队 Docker 镜像 `ti-env:latest` 尚在内部使用阶段 (Coming Soon!)。以下是通用的启动命令，你可以基于此构建自己的镜像。

**Docker 用户启动命令:**

```bash
# -- 替换为你自己的项目路径 --
export PROJECT_PATH=/path/to/your/MSPM0_APPS

# -- 启动容器 --
docker run -it --rm \
    --name ti-dev-container \
    -p 127.0.0.1:2222:22 \
    --privileged \
    --env DISPLAY=$DISPLAY \
    --volume /tmp/.X11-unix:/tmp/.X11-unix:rw \
    --volume $PROJECT_PATH:/root/Projects/:rw \
    --volume ~/.ssh/id_rsa.pub:/root/.ssh/authorized_keys:ro \
    --shm-size=1g \
    your-ti-env-image:latest \
    /usr/sbin/sshd -D
```

**Podman 用户参考 (Systemd Unit):**

<details>
<summary>点击展开 Podman Systemd 配置参考</summary>

```ini
[Unit]
Description=TI Development Container
Wants=network-online.target
After=network-online.target
RequiresMountsFor=%h

[Container]
Image=localhost/ti-env:latest
ContainerName=ti-dev-container
PublishPort=127.0.0.1:2222:22
Environment=DISPLAY=${DISPLAY}
Volume=/tmp/.X11-unix:/tmp/.X11-unix:rw
Volume=%h/Projects/:/root/Projects/:rw
Volume=%h/.ssh/id_rsa.pub:/root/.ssh/authorized_keys:ro
Volume=vscode-server-data:/root/.vscode-server:rw
AddDevice=/dev/bus/usb
Volume=/run/udev:/run/udev:ro
PodmanArgs=--privileged --shm-size=1g
Exec=/usr/sbin/sshd -D

[Service]
Restart=always

[Install]
WantedBy=default.target
```

</details>

#### 3. 连接与使用

1.  **连接到容器**:
    - 在 VS Code 中，使用 **Remote - SSH** 扩展连接到 `ssh://root@127.0.0.1:2222`。
    - 或者使用 **Remote - Containers** 扩展 `Attach to Running Container...`。
2.  **打开项目**: 在 VS Code 中，打开位于 `/root/Projects/` 的项目文件夹。
3.  **首次配置**:
    - CMake Tools 插件会自动运行，并提示你设置 `TI_SDK_ROOT` 和 `SYSCONFIG_CLI_PATH`。
    - 根据容器内的实际路径进行设置 (例如 `/root/ti/mspm0_sdk/...` 和 `/root/ti/sysconfig/sysconfig_cli.sh`)。
4.  **开始开发**:
    - **硬件配置**: 在容器终端运行 `/path/to/sysconfig_gui.sh` 启动 SysConfig，窗口会自动显示在你的宿主机桌面上。
    - **编译**: 按 `Ctrl+Shift+B`。
    - **调试**: 连接好 **官方 LaunchPad (XDS110)**，直接按 `F5`。
    - **烧录并运行**: 按 `Ctrl+Shift+P` -> `Run Task` -> `Flash & Run (XDS110 + OpenOCD)`。

---

### 方案二：简陋的 Windows J-Link 烧录法

此方法仅用于在没有完整环境时，快速将固件烧录到只支持 J-Link 的盗版板子上。

#### 1. 先决条件

- Windows 操作系统。
- 已安装 **SEGGER J-Link** 官方软件包。
- 你已经通过其他方式（例如在这个模板的容器环境中）编译出了 `.elf` 或 `.bin` 文件。

#### 2. 烧录步骤

1.  **连接硬件**: 将你的克隆版 J-Link 连接到电脑和目标板。**注意：可能需要反复热插拔**才能被 J-Link 软件正确识别。
2.  **启动 J-Flash Lite**:
    - 从开始菜单找到并运行 `J-Flash Lite`。
    - **Device**: 选择 `Texas Instruments` -> `MSPM0G3507`。
    - **Interface**: 选择 `SWD`，速度 `4000 kHz`。
3.  **选择固件**:
    - 点击 `...` 按钮，找到你的 `.elf` 或 `.bin` 固件文件。
4.  **烧录**:
    - 点击 `Program Device` 按钮。
    - 如果遇到无法擦除 (erase) 但可以烧录 (program) 的情况，这是克隆版 J-Link 的常见问题。通常程序也能被成功烧录。

---

## 🏗️ 如何将此模板用于新项目

将这个模板改造成你自己的新项目非常简单：

1.  **复制模板**: 完整地复制整个项目文件夹，并重命名为你新项目的名字 (例如 `MyAwesomeProject`)。
2.  **清理旧文件**:
    - 删除 `build/` 目录。
    - 删除 `generated/` 目录下的所有内容。
    - 进入 `src/common/`，删除 `gpio_toggle_output.c` 和 `gpio_toggle_output.syscfg`。
3.  **添加新文件**:
    - 将你的新应用代码（例如 `my_awesome_project.c`）放入 `src/common/`。
    - 创建一个新的 `.syscfg` 文件（例如 `my_awesome_project.syscfg`），推荐使用 SysConfig GUI 的 `File -> New` 功能创建。
4.  **修改 `CMakeLists.txt`**:
    - **修改项目名**: 将 `project(MSPM0G3507App ...)` 改为 `project(MyAwesomeProject ...)`。
    - **修改 SysConfig 输入文件**: 将 `set(SYSCONFIG_INPUT_FILE ...)` 中的文件名改为 `my_awesome_project.syscfg`。
    - **修改源文件列表**: 在 `set(ALL_SRC_FILES ...)` 中，将 `gpio_toggle_output.c` 改为 `my_awesome_project.c`。
5.  **完成**:
    - 用 VS Code 打开新项目文件夹，CMake 会自动配置。
    - 你现在可以开始在新项目上进行开发了！

---

## 🤝 致谢

感谢所有在这次史诗级调试之旅中提供帮助的伙伴和工具。这个模板的诞生，离不开对每一个错误信息的执着追寻和对工具链底层原理的深入探索。

Enjoy coding!
