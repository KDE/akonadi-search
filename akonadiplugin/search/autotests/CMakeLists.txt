include(ECMMarkAsTest)

set(QT_REQUIRED_VERSION "5.9.0")
find_package(Qt5Test ${QT_REQUIRED_VERSION} CONFIG REQUIRED)


include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/..
  ${CMAKE_CURRENT_BINARY_DIR}/../../../agent/
  ${CMAKE_CURRENT_BINARY_DIR}/..
)

set(searchplugintest_SRCS
    searchplugintest.cpp
    ../searchplugin.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/../akonadiplugin_search_debug.cpp
)

add_definitions(-DAKONADI_SEARCH_NO_PLUGINS=TRUE)

add_executable( searchplugintest ${searchplugintest_SRCS} )
add_test( NAME searchplugintest COMMAND searchplugintest )
ecm_mark_as_test(searchplugintest)

target_link_libraries(searchplugintest
    Qt5::Test
    KF5::AkonadiCore
    KF5::AkonadiMime
    KF5::Mime
    KF5::Contacts
    KF5::CalendarCore
    KF5::AkonadiSearch
    KF5::Codecs
)
