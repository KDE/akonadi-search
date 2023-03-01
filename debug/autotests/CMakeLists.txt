include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}/..
)

macro(add_akonadisearchdebug_unittest _source)
    get_filename_component( _name ${_source} NAME_WE )
    ecm_add_test(${_source} ${_name}.h
        TEST_NAME ${_name}
        NAME_PREFIX "akonadisearchdebug-"
        LINK_LIBRARIES Qt::Test Qt::Gui KF${KF_MAJOR_VERSION}::Completion KF${KF_MAJOR_VERSION}::CoreAddons KPim${KF_MAJOR_VERSION}::AkonadiSearchDebug KPim${KF_MAJOR_VERSION}::AkonadiCore
        GUI
    )
endmacro ()

add_akonadisearchdebug_unittest(akonadisearchdebugdialogtest.cpp)
add_akonadisearchdebug_unittest(akonadisearchdebugwidgettest.cpp)
add_akonadisearchdebug_unittest(akonadisearchdebugsearchpathcomboboxtest.cpp)
