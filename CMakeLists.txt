# ~/Projects/MSPM0_APPS/CMakeLists.txt
# 这是 MSPM0_APPS 项目的 CMakeLists.txt 最终版本
# 包含了所有之前讨论过的路径、参数和编译选项的修正

# 包含工具链配置文件，用于交叉编译设置
include(toolchain.cmake)

cmake_minimum_required(VERSION 3.10)
# project() 命令只定义语言，不进行任何额外的编译器检测参数，因为 toolchain.cmake 会处理
project(MSPM0G3507App LANGUAGES C CXX ASM)

# ** 1. 定义目标芯片型号 (只关注G3507) **
set(TARGET_CHIP "MSPM0G3507")

# ** 3. TI SDK 路径 **
# 使用绝对路径，避免 Shell 的 ~ 解析问题
set(TI_SDK_ROOT "/home/cagedbird/ti/mspm0_sdk_2_05_01_00")

# ** 4. 根据 TARGET_CHIP 设置芯片特定变量 **
set(LINKER_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/configs/G3507/linker.ld")
set(SVD_FILE_PATH "${TI_SDK_ROOT}/source/ti/devices/msp/m0p/startup_system_files/mspm0g3507.svd")
set(GENERATED_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/generated/G3507")
set(STARTUP_FILE "${CMAKE_CURRENT_SOURCE_DIR}/src/common/startup_mspm0g350x_gcc.c")

# 定义 SysConfig 使用的封装名称 (转义括号)
set(PACKAGE_NAME_ESCAPED "LQFP-64\\(PM\\)")

# ** 5. 编译/链接选项 **
# 架构相关的编译标志
set(ARM_ARCH_FLAGS "-mcpu=cortex-m0plus" "-mthumb") # 定义为列表，避免引号问题
# 通用编译标志
set(COMMON_COMPILE_FLAGS "-Wall" "-g" "-Os" "-ffunction-sections" "-fdata-sections") # 定义为列表

# 链接器标志的各个部分
set(LINKER_FLAGS_COMMON "-nostartfiles")
set(LINKER_FLAGS_SCRIPT "-T${LINKER_SCRIPT}")
set(LINKER_FLAGS_GC "-Wl,--gc-sections,-Map=${PROJECT_NAME}.map")

# **添加一个 GCC spec 文件，处理裸机环境下的标准库链接**
set(LINKER_FLAGS_SPECS "--specs=nosys.specs") # **添加这一行**

# ** 6. 包含目录 **
include_directories(
    # 1. TI SDK 的顶层 source 目录，用于解析 <ti/drivers/...>, <ti/devices/...> 等
    "${TI_SDK_ROOT}/source"

    # 2. CMSIS 核心头文件路径 (包含 core_cm0plus.h)
    "${TI_SDK_ROOT}/source/third_party/CMSIS/Core/Include"

    # 3. 你的项目源代码和SysConfig生成代码的头文件路径
    "${CMAKE_CURRENT_SOURCE_DIR}/src/common"
    "${GENERATED_SRC_DIR}"
)


# --- SysConfig 代码生成自定义命令 ---
# 定义 SysConfig CLI 路径 (使用绝对路径)
set(SYSCONFIG_CLI_PATH "/home/cagedbird/ti/sysconfig_1.24.1/sysconfig_cli.sh")

# 定义 SDK 的 product.json 路径 (使用绝对路径)
set(SDK_PRODUCT_JSON "${TI_SDK_ROOT}/.metadata/product.json")

# 定义 SysConfig 输入和输出文件/目录
set(SYSCONFIG_INPUT_FILE "${CMAKE_CURRENT_SOURCE_DIR}/src/common/gpio_toggle_output.syscfg")
set(SYSCONFIG_OUTPUT_DIR "${GENERATED_SRC_DIR}")

# 定义 SysConfig 生成的 C/H 文件 (作为 add_custom_command 的 OUTPUT)
set(SYSCONFIG_GENERATED_C "${SYSCONFIG_OUTPUT_DIR}/ti_msp_dl_config.c")
set(SYSCONFIG_GENERATED_H "${SYSCONFIG_OUTPUT_DIR}/ti_msp_dl_config.h")

# 添加自定义命令来运行 SysConfig
add_custom_command(
    OUTPUT ${SYSCONFIG_GENERATED_C} ${SYSCONFIG_GENERATED_H}
    # COMMAND 使用多行时，不再需要行尾的 \
    COMMAND ${SYSCONFIG_CLI_PATH}
        --product "${SDK_PRODUCT_JSON}"
        --device ${TARGET_CHIP}
        --package "${PACKAGE_NAME_ESCAPED}"
        --script "${SYSCONFIG_INPUT_FILE}"
        --output "${SYSCONFIG_OUTPUT_DIR}"
    DEPENDS ${SYSCONFIG_INPUT_FILE} # 当 .syscfg 文件变化时，重新运行此命令
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} # 运行命令的工作目录
    COMMENT "Generating SysConfig files for ${TARGET_CHIP}"
)
# --- SysConfig 代码生成自定义命令结束 ---


# ** 7. 源文件 **
file(GLOB_RECURSE COMMON_SRC_FILES
    "src/common/*.c"
    "src/common/*.cpp"
    "src/common/*.s"
)
# 移除 SysConfig 文件自身，因为它不是编译源文件
list(REMOVE_ITEM COMMON_SRC_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/common/gpio_toggle_output.syscfg")

# 包含 SysConfig 生成的 C 文件作为源文件
list(APPEND COMMON_SRC_FILES "${SYSCONFIG_GENERATED_C}")

set(ALL_SRC_FILES ${COMMON_SRC_FILES})


# 8. 链接SDK库
set(SDK_LIBS
    # drivers 库
    "${TI_SDK_ROOT}/source/ti/drivers/lib/gcc/m0p/drivers_mspm0g1x0x_g3x0x.a"
    # driverlib 库
    "${TI_SDK_ROOT}/source/ti/driverlib/lib/gcc/m0p/mspm0g1x0x_g3x0x/driverlib.a"
    # 如果还需要其他库（例如 RTOS），请在这里添加
)

# ** 9. 定义可执行目标 **
add_executable(${PROJECT_NAME}.elf ${ALL_SRC_FILES})

# 应用编译选项 (使用 SHELL: 前缀避免引号问题)
target_compile_options(${PROJECT_NAME}.elf PRIVATE
    SHELL:
    ${ARM_ARCH_FLAGS}
    ${COMMON_COMPILE_FLAGS}
)

# 添加 DeviceFamily 宏定义和芯片型号宏定义
target_compile_definitions(${PROJECT_NAME}.elf PRIVATE
    __MSPM0G3507__ # 精确的芯片型号宏，符合 DeviceFamily.h 的要求
)

# 链接选项 (使用 SHELL: 前缀避免引号问题)
target_link_options(${PROJECT_NAME}.elf PRIVATE
    SHELL:
    ${ARM_ARCH_FLAGS}
    ${LINKER_FLAGS_COMMON}
    ${LINKER_FLAGS_SCRIPT}
    ${LINKER_FLAGS_GC}
    ${LINKER_FLAGS_SPECS} # **添加这一行**
)

# 链接库 (标准库和SDK库)
target_link_libraries(${PROJECT_NAME}.elf PRIVATE ${SDK_LIBS} m) # **移除 newlib 和 nano**


# ** 10. 添加自定义构建步骤 (生成.bin和.hex) **
add_custom_command(
    TARGET ${PROJECT_NAME}.elf POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O binary ${PROJECT_NAME}.elf ${PROJECT_NAME}.bin
    COMMAND ${CMAKE_OBJCOPY} -O ihex ${PROJECT_NAME}.elf ${PROJECT_NAME}.hex
    COMMAND ${CMAKE_SIZE} ${PROJECT_NAME}.elf
    COMMENT "Generating .bin, .hex files and displaying size"
)

# ** 11. 为VS Code的C/C++扩展生成编译命令数据库 **
set_target_properties(${PROJECT_NAME}.elf PROPERTIES CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ** 12. 将SVD文件路径传递给VS Code (非常重要，用于Cortex-Debug)**
set(CORTEX_DEBUG_SVD_FILE "${SVD_FILE_PATH}" CACHE STRING "SVD file path for Cortex-Debug")