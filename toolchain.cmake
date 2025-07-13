# ~/Projects/MSPM0_APPS/toolchain.cmake

# --- Mandatory settings for cross-compilation ---
set(CMAKE_SYSTEM_NAME Generic) # Indicate that we are compiling for a generic system
set(CMAKE_SYSTEM_PROCESSOR arm)

# Set the cross compilers (these must be the full paths or in PATH)
# These are the same as what you set in your VS Code Kit.
# Remove redundant settings from CMakeLists.txt if they are also here.
set(CMAKE_C_COMPILER arm-none-eabi-gcc)
set(CMAKE_CXX_COMPILER arm-none-eabi-g++)
set(CMAKE_ASM_COMPILER arm-none-eabi-gcc) # Not strictly needed by CMake, but good for consistency
set(CMAKE_OBJCOPY arm-none-eabi-objcopy)
set(CMAKE_SIZE arm-none-eabi-size)
set(CMAKE_GDB arm-none-eabi-gdb)

# --- Crucial for bare-metal / embedded cross-compilation ---
# Tell CMake that the compilers are known to work for the target.
# This prevents CMake from trying to compile and link a test program
# which would fail due to missing bare-metal syscalls (e.g., _exit, _sbrk).
set(CMAKE_C_COMPILER_FORCED TRUE)
set(CMAKE_CXX_COMPILER_FORCED TRUE)

# Optionally, you can specify the compiler ID to match the actual compiler.
# This helps CMake in guessing compiler features without running tests.
# set(CMAKE_C_COMPILER_ID GNU)
# set(CMAKE_CXX_COMPILER_ID GNU)
# set(CMAKE_C_COMPILER_ABI_INFO_INIT "arm") # This might hint at ABI, but not always needed with FORCED TRUE

# Don't search for programs, libraries, or includes in the host system root path.
# This ensures CMake only looks for target-specific files.
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# Define the target architecture and ABI
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mcpu=cortex-m0plus -mthumb")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mcpu=cortex-m0plus -mthumb")
set(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} -mcpu=cortex-m0plus -mthumb")

# Optional: Add compiler specs for bare-metal (Newlib-nano specific)
# This is usually managed by target_link_libraries in CMakeLists.txt,
# but can be added here if needed for compiler flags during initial tests
# For the `undefined reference to _exit` errors, this is the linker part:
# set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --specs=nosys.specs -nostdlib")
# However, CMAKE_COMPILER_FORCED TRUE should bypass the link test entirely.