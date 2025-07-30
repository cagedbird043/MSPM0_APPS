# ==============================================================================
# MSPM0G3507 CMake 项目 - 终极完整版
# 结合了Python预处理和GLOB自动发现源文件
# ==============================================================================

# 包含工具链配置文件
include(toolchain.cmake)

# 推荐 CMake 版本 3.14 或更高
cmake_minimum_required(VERSION 3.14)

# 确保能找到Python解释器
find_package(Python3 REQUIRED)

project(MSPM0G3507App LANGUAGES C CXX ASM)

# ==============================================================================
# 用户可配置的路径 (缓存变量)
# ==============================================================================
set(TI_SDK_ROOT "" CACHE PATH "指定 TI MSPM0 SDK 的根目录路径")
set(SYSCONFIG_CLI_PATH "" CACHE FILEPATH "指定 SysConfig CLI 可执行文件的路径 (sysconfig_cli.sh 或 .bat)")

# 检查路径是否已设置
if(NOT TI_SDK_ROOT)
    message(FATAL_ERROR "错误: TI_SDK_ROOT 未设置!")
endif()
if(NOT SYSCONFIG_CLI_PATH)
    message(FATAL_ERROR "错误: SYSCONFIG_CLI_PATH 未设置!")
endif()

message(STATUS "正在使用 TI SDK 路径: ${TI_SDK_ROOT}")
message(STATUS "正在使用 SysConfig CLI 路径: ${SYSCONFIG_CLI_PATH}")
# ==============================================================================


# 1. 定义目标芯片型号
set(TARGET_CHIP "MSPM0G3507")

# 2. 定义芯片相关的变量
set(LINKER_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/configs/G3507/linker.ld")
set(SVD_FILE_PATH "${TI_SDK_ROOT}/source/ti/devices/msp/m0p/startup_system_files/mspm0g3507.svd")
set(GENERATED_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/generated/G3507")

# 3. 定义编译和链接选项
set(ARM_ARCH_FLAGS "-mcpu=cortex-m0plus" "-mthumb")
set(COMMON_COMPILE_FLAGS "-Wall" "-g" "-Os" "-ffunction-sections" "-fdata-sections")
set(LINKER_FLAGS_COMMON "-nostartfiles")
set(LINKER_FLAGS_SCRIPT "-T${LINKER_SCRIPT}")
set(LINKER_FLAGS_GC "-Wl,--gc-sections,-Map=${PROJECT_NAME}.map")
set(LINKER_FLAGS_SPECS "--specs=nosys.specs")

# 4. 定义包含目录
include_directories(
    "${TI_SDK_ROOT}/source"
    "${TI_SDK_ROOT}/source/third_party/CMSIS/Core/Include"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/common"
    "${GENERATED_SRC_DIR}"
)

# --- SysConfig 代码生成 (Python预处理终极方案) ---

# 定义相关文件路径
set(SDK_PRODUCT_JSON "${TI_SDK_ROOT}/.metadata/product.json")
set(SYSCONFIG_INPUT_FILE "${CMAKE_CURRENT_SOURCE_DIR}/src/common/main.syscfg")
set(SYSCONFIG_OUTPUT_DIR "${GENERATED_SRC_DIR}")
set(BOARD_JSON_FILE "${TI_SDK_ROOT}/source/ti/boards/.meta/LP_MSPM0G3507.syscfg.json")
set(SYSCONFIG_TEMP_FILE "${CMAKE_CURRENT_BINARY_DIR}/main.syscfg.temp")
set(PYTHON_PREPROCESSOR_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/preprocess_syscfg.py")
set(SYSCONFIG_GENERATED_C "${SYSCONFIG_OUTPUT_DIR}/ti_msp_dl_config.c")
set(SYSCONFIG_GENERATED_H "${SYSCONFIG_OUTPUT_DIR}/ti_msp_dl_config.h")

# 定义一个自定义命令，它负责生成文件
add_custom_command(
    OUTPUT ${SYSCONFIG_GENERATED_C} ${SYSCONFIG_GENERATED_H}
    
    # 第一步：调用Python脚本进行预处理
    COMMAND ${Python3_EXECUTABLE} ${PYTHON_PREPROCESSOR_SCRIPT}
            "${BOARD_JSON_FILE}"
            "${SYSCONFIG_INPUT_FILE}"
            "${SYSCONFIG_TEMP_FILE}"
            
    # 第二步：调用CLI，使用Python生成的临时文件
    COMMAND ${SYSCONFIG_CLI_PATH}
        --product "${SDK_PRODUCT_JSON}"
        --device  "${TARGET_CHIP}"
        --package "LQFP-64(PM)"
        --script  "${SYSCONFIG_TEMP_FILE}"
        --output  "${SYSCONFIG_OUTPUT_DIR}"
        
    MAIN_DEPENDENCY ${SYSCONFIG_INPUT_FILE}
    DEPENDS ${PYTHON_PREPROCESSOR_SCRIPT}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "正在为 MSPM0G3507 生成 SysConfig 文件 (Python预处理 + CLI生成)"
    VERBATIM
)

# 定义一个自定义目标，用于“触发”上面的命令
add_custom_target(sysconfig_generate
    DEPENDS ${SYSCONFIG_GENERATED_C} ${SYSCONFIG_GENERATED_H}
)

# --- SysConfig 代码生成结束 ---


# 5. 定义源文件 (使用GLOB_RECURSE自动捕获)
file(GLOB_RECURSE ALL_PROJECT_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/common/*.c"
)

# 将自动捕获的源文件和SysConfig生成的源文件合并
set(ALL_SRC_FILES
    ${ALL_PROJECT_SOURCES}
    ${SYSCONFIG_GENERATED_C}
)

# 6. 定义 SDK 库文件
set(SDK_LIBS
    "${TI_SDK_ROOT}/source/ti/drivers/lib/gcc/m0p/drivers_mspm0g1x0x_g3x0x.a"
    "${TI_SDK_ROOT}/source/ti/driverlib/lib/gcc/m0p/mspm0g1x0x_g3x0x/driverlib.a"
)

# 7. 定义可执行目标
add_executable(${PROJECT_NAME}.elf ${ALL_SRC_FILES})

# 8. 明确指定 elf 目标依赖于 SysConfig 虚拟目标
add_dependencies(${PROJECT_NAME}.elf sysconfig_generate)

# 9. 应用编译和链接选项
target_compile_options(${PROJECT_NAME}.elf PRIVATE
    ${ARM_ARCH_FLAGS}
    ${COMMON_COMPILE_FLAGS}
    "-I/usr/lib/arm-none-eabi/include"
    "-I/usr/lib/gcc/arm-none-eabi/12.2.1/include"
)

target_compile_definitions(${PROJECT_NAME}.elf PRIVATE
    __MSPM0G3507__
)

target_link_options(${PROJECT_NAME}.elf PRIVATE
    ${ARM_ARCH_FLAGS}
    ${LINKER_FLAGS_COMMON}
    ${LINKER_FLAGS_SCRIPT}
    ${LINKER_FLAGS_GC}
    ${LINKER_FLAGS_SPECS}
)

# 10. 链接库
target_link_libraries(${PROJECT_NAME}.elf PRIVATE ${SDK_LIBS} m)

# 11. 定义构建后命令
add_custom_command(
    TARGET ${PROJECT_NAME}.elf POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O binary ${PROJECT_NAME}.elf ${PROJECT_NAME}.bin
    COMMAND ${CMAKE_OBJCOPY} -O ihex ${PROJECT_NAME}.elf ${PROJECT_NAME}.hex
    COMMAND ${CMAKE_SIZE} ${PROJECT_NAME}.elf
    COMMENT "正在生成 .bin, .hex 文件并显示大小"
)

# 12. IDE 集成支持
set_target_properties(${PROJECT_NAME}.elf PROPERTIES CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CORTEX_DEBUG_SVD_FILE "${SVD_FILE_PATH}" CACHE STRING "用于 Cortex-Debug 的 SVD 文件路径")