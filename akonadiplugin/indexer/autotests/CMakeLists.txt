include(ECMMarkAsTest)

set(QT_REQUIRED_VERSION "5.8.0")
find_package(Qt5Test ${QT_REQUIRED_VERSION} CONFIG REQUIRED)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/..
  ${CMAKE_CURRENT_BINARY_DIR}/..
)

set(indexingplugintest_SRCS
    indexingplugintest.cpp
    ../indexingplugin.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/../akonadiplugin_indexing_debug.cpp
)

add_executable( indexingplugintest ${indexingplugintest_SRCS} )
add_test( NAME indexingplugintest COMMAND indexingplugintest )
ecm_mark_as_test(indexingplugintest)

target_link_libraries(indexingplugintest
    Qt5::Test
    KF5::AkonadiCore
    KF5::AkonadiMime
    KF5::Mime
    KF5::Contacts
    KF5::CalendarCore
    KF5::AkonadiSearch
    KF5::Codecs
)

