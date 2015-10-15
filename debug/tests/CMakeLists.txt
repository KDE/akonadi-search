include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}/..
)

set(akonadisearchdebug_dialog_gui_SRCS akonadisearchdebug_dialog_gui.cpp)
add_executable(akonadisearchdebug_dialog_gui ${akonadisearchdebug_dialog_gui_SRCS})
ecm_mark_as_test(akonadisearchdebug_dialog_gui)
target_link_libraries(akonadisearchdebug_dialog_gui
  Qt5::Core
  Qt5::Gui
  Qt5::Widgets
  KF5::AkonadiCore
  KF5::I18n
  KF5::AkonadiSearchDebug
)

