cmake_minimum_required(VERSION 3.20.0)

# The custom board lives in this repo: app/boards/smookie/esp32phone_v2
set(BOARD_ROOT ${CMAKE_CURRENT_SOURCE_DIR})

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(esp32phone_v2_fw LANGUAGES C)

target_include_directories(app PRIVATE include)
target_sources(app PRIVATE
  src/main.c
  src/core/channels.c
  src/core/registry.c
  src/core/framework.c
  src/services/input_svc.c
  src/ui/status_bar.c
  src/ui/softkey_bar.c
  src/ui/widgets.c
  src/apps/home.c
  src/apps/settings.c
)

zephyr_linker_sources(SECTIONS sections-rom.ld)
