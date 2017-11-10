include(ECMMarkAsTest)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_BINARY_DIR}/..
)
add_executable(searchplugintest)

target_sources(
    searchplugintest
    PRIVATE
        searchplugintest.cpp
        ../searchplugin.cpp
        ${CMAKE_CURRENT_BINARY_DIR}/../akonadiplugin_search_debug.cpp
)

add_definitions(-DAKONADI_SEARCH_NO_PLUGINS=TRUE)

add_test(NAME searchplugintest COMMAND searchplugintest)
ecm_mark_as_test(searchplugintest)

target_link_libraries(
    searchplugintest
    Qt::Test
    KPim6::AkonadiCore
    KPim6::AkonadiMime
    KF6::Mime
    KF6::Contacts
    KF6::CalendarCore
    KF5::AkonadiSearch
    KF6::Codecs
)
