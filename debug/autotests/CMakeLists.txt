include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}/..
)

macro(add_akonadisearchdebug_unittest _source)
    ecm_add_test(${_source}
        NAME_PREFIX "akonadisearchdebug-"
        LINK_LIBRARIES Qt::Test Qt::Gui KF5::Completion KF5::CoreAddons KF5::AkonadiSearchDebug KF5::AkonadiCore
        GUI
    )
endmacro ()

add_akonadisearchdebug_unittest(akonadisearchdebugdialogtest.cpp)
add_akonadisearchdebug_unittest(akonadisearchdebugwidgettest.cpp)
add_akonadisearchdebug_unittest(akonadisearchdebugsearchpathcomboboxtest.cpp)
