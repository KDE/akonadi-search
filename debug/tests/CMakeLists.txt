include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}/..
)

add_executable(akonadisearchdebug_dialog_gui akonadisearchdebug_dialog_gui.cpp)
ecm_mark_as_test(akonadisearchdebug_dialog_gui)
target_link_libraries(akonadisearchdebug_dialog_gui
  Qt::Core
  Qt::Gui
  Qt::Widgets
  KF5::AkonadiCore
  KF5::I18n
  KF5::AkonadiSearchDebug
)

