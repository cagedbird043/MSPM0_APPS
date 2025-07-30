# ==============================================================================
# MSPM0G3507 CMake 项目 - 通用模板 (中文注释最终版 - 正确的自定义目标)
# ==============================================================================

# 包含工具链配置文件
include(toolchain.cmake)

# 推荐 CMake 版本 3.14 或更高
cmake_minimum_required(VERSION 3.14)
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

# --- SysConfig 代码生成 (CMake原生解析JSON并替换别名的终极方案) ---

# 定义相关文件路径
set(SDK_PRODUCT_JSON "${TI_SDK_ROOT}/.metadata/product.json")
set(SYSCONFIG_INPUT_FILE "${CMAKE_CURRENT_SOURCE_DIR}/src/common/main.syscfg")
set(SYSCONFIG_OUTPUT_DIR "${GENERATED_SRC_DIR}")
set(BOARD_JSON_FILE "${TI_SDK_ROOT}/source/ti/boards/.meta/LP_MSPM0G3507.syscfg.json")
set(SYSCONFIG_TEMP_FILE "${CMAKE_CURRENT_BINARY_DIR}/main.syscfg.temp")

# 1. 读取板级JSON定义文件
file(READ ${BOARD_JSON_FILE} BOARD_JSON_CONTENT)
# 移除换行符，方便正则表达式处理
string(REPLACE "\n" "" BOARD_JSON_CONTENT ${BOARD_JSON_CONTENT})
string(REPLACE "\r" "" BOARD_JSON_CONTENT ${BOARD_JSON_CONTENT})

# 2. 读取项目syscfg文件
file(READ ${SYSCONFIG_INPUT_FILE} PROJECT_SYSCFG_CONTENT)

# 3. 循环查找并替换所有 "boosterpack.X" 别名
#    这个正则表达式会匹配所有 "boosterpack." 后面跟着数字的模式
string(REGEX MATCHALL "\"boosterpack\\.[0-9]+\"" BOOSTERPACK_ALIASES ${PROJECT_SYSCFG_CONTENT})

# 去除重复的别名
list(REMOVE_DUPLICATES BOOSTERPACK_ALIASES)

message(STATUS "在 .syscfg 文件中找到需要替换的别名: ${BOOSTERPACK_ALIASES}")

# 遍历找到的每一个别名
foreach(ALIAS ${BOOSTERPACK_ALIASES})
    # 从别名中提取出数字，例如从 "boosterpack.15" 中提取 15
    string(REGEX REPLACE "\"boosterpack\\.([0-9]+)\"" "\\1" PIN_NUMBER ${ALIAS})

    # 在JSON内容中，根据这个数字找到对应的物理引脚名
    # 这个正则表达式查找 "number": PIN_NUMBER, ... "name": "PIN_NAME" 这样的模式
    string(REGEX MATCH "\"number\":[ ]*${PIN_NUMBER},[^\}]*\"name\":[ ]*\"([A-Z0-9]+)\"" MATCHED_PIN_BLOCK ${BOARD_JSON_CONTENT})

    if(MATCHED_PIN_BLOCK)
        # 从匹配到的块中，提取出物理引脚名 (第一个捕获组)
        set(PHYSICAL_PIN "${CMAKE_MATCH_1}")
        message(STATUS "  -> 正在将 ${ALIAS} 替换为 \"${PHYSICAL_PIN}\"")
        # 在项目syscfg内容中执行替换
        string(REPLACE ${ALIAS} "\"${PHYSICAL_PIN}\"" PROJECT_SYSCFG_CONTENT ${PROJECT_SYSCFG_CONTENT})
    else()
        message(WARNING "  -> 未能在 ${BOARD_JSON_FILE} 中找到别名 ${ALIAS} 的定义！")
    endif()
endforeach()

# 4. 将替换后的内容写入临时文件
file(WRITE ${SYSCONFIG_TEMP_FILE} "${PROJECT_SYSCFG_CONTENT}")

# 定义生成的 .c 和 .h 文件路径（假设生成文件名为 ti_sysconfig.c/h）
set(SYSCONFIG_GENERATED_C "${SYSCONFIG_OUTPUT_DIR}/ti_msp_dl_config.c")
set(SYSCONFIG_GENERATED_H "${SYSCONFIG_OUTPUT_DIR}/ti_msp_dl_config.h")

# 2. 定义一个自定义命令，它只负责一件事：生成文件。
#    这个命令的输出就是我们需要的 .c 和 .h 文件。
add_custom_command(
    OUTPUT ${SYSCONFIG_GENERATED_C} ${SYSCONFIG_GENERATED_H}
    COMMAND ${SYSCONFIG_CLI_PATH}
        --product ${SDK_PRODUCT_JSON}
        --device ${TARGET_CHIP}
        --package "LQFP-64(PM)"
        --script ${SYSCONFIG_TEMP_FILE}
        --output ${SYSCONFIG_OUTPUT_DIR}
    # 这个命令的主要依赖是那个临时syscfg文件
    MAIN_DEPENDENCY ${SYSCONFIG_TEMP_FILE}
    # 也依赖原始syscfg文件
    DEPENDS ${SYSCONFIG_INPUT_FILE}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "正在为 MSPM0G3507 生成 SysConfig 文件 (已自动解析引脚别名)"
    VERBATIM
)

# 6. 定义一个自定义目标，它的唯一作用就是“触发”上面的自定义命令。
#    我们将让我们的主目标依赖于这个触发器。
add_custom_target(sysconfig_generate
    DEPENDS ${SYSCONFIG_GENERATED_C} ${SYSCONFIG_GENERATED_H}
)


# 7. 定义源文件 (现在可以安全地包含生成的 .c 文件了)
file(GLOB_RECURSE ALL_SRC_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c"
)
# 现在我们可以直接在这里包含它，因为CMake通过add_custom_command已经知道了它的生成规则
list(APPEND ALL_SRC_FILES ${SYSCONFIG_GENERATED_C})

# 6. 定义 SDK 库文件
set(SDK_LIBS
    "${TI_SDK_ROOT}/source/ti/drivers/lib/gcc/m0p/drivers_mspm0g1x0x_g3x0x.a"
    "${TI_SDK_ROOT}/source/ti/driverlib/lib/gcc/m0p/mspm0g1x0x_g3x0x/driverlib.a"
)

# 8. 定义可执行目标 (注意！不再需要 target_sources)
add_executable(${PROJECT_NAME}.elf ${ALL_SRC_FILES})


# 9. 明确指定 elf 目标依赖于 SysConfig 生成目标
#    这一步至关重要，它建立了构建顺序
add_dependencies(${PROJECT_NAME}.elf sysconfig_generate)


# 8. 应用编译和链接选项
target_compile_options(${PROJECT_NAME}.elf PRIVATE
    ${ARM_ARCH_FLAGS}
    ${COMMON_COMPILE_FLAGS}
    "-I/usr/lib/arm-none-eabi/include" # <-- 请根据你的实际路径修改
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

# 9. 链接库
target_link_libraries(${PROJECT_NAME}.elf PRIVATE ${SDK_LIBS} m)


# 10. 定义构建后命令
add_custom_command(
    TARGET ${PROJECT_NAME}.elf POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O binary ${PROJECT_NAME}.elf ${PROJECT_NAME}.bin
    COMMAND ${CMAKE_OBJCOPY} -O ihex ${PROJECT_NAME}.elf ${PROJECT_NAME}.hex
    COMMAND ${CMAKE_SIZE} ${PROJECT_NAME}.elf
    COMMENT "正在生成 .bin, .hex 文件并显示大小"
)

# 11. IDE 集成支持
set_target_properties(${PROJECT_NAME}.elf PROPERTIES CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CORTEX_DEBUG_SVD_FILE "${SVD_FILE_PATH}" CACHE STRING "用于 Cortex-Debug 的 SVD 文件路径")