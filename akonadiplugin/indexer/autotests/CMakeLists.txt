include(ECMMarkAsTest)


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
set_tests_properties(indexingplugintest PROPERTIES RESOURCE_LOCK akonadi_search_db)
ecm_mark_as_test(indexingplugintest)

target_link_libraries(indexingplugintest
    Qt::Test
    KPim6::AkonadiCore
    KPim6::AkonadiMime
    KF6::Mime
    KF6::Contacts
    KF6::CalendarCore
    KPim6::AkonadiSearch
    KF6::Codecs
)

