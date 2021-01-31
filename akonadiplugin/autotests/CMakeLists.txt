include(ECMMarkAsTest)

find_package(Qt5Test ${QT_REQUIRED_VERSION} CONFIG REQUIRED)


include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/..
  ${CMAKE_CURRENT_BINARY_DIR}/../../agent/
  ${CMAKE_CURRENT_BINARY_DIR}/..
)

set(searchplugintest_SRCS
    searchplugintest.cpp
    ../searchplugin.cpp
    ../../agent/emailindexer.cpp
    ../../agent/calendarindexer.cpp
    ../../agent/contactindexer.cpp
    ../../agent/abstractindexer.cpp
    ../../agent/akonotesindexer.cpp
    ../../search/pimsearchstore.cpp
    ../../search/email/emailsearchstore.cpp
    ../../search/email/agepostingsource.cpp
    ../../search/contact/contactsearchstore.cpp
    ../../search/calendar/calendarsearchstore.cpp
    ../../search/note/notesearchstore.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/../../agent/akonadi_indexer_agent_debug.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/../akonadiplugin_indexer_debug.cpp
)

add_definitions(-DAKONADI_SEARCH_NO_PLUGINS=TRUE)

add_executable( searchplugintest ${searchplugintest_SRCS} )
add_test( NAME searchplugintest COMMAND searchplugintest )
ecm_mark_as_test(searchplugintest)

target_link_libraries(searchplugintest
    Qt::Test
    KF5::AkonadiCore
    KF5::AkonadiMime
    KF5::Mime
    KF5::Contacts
    ${XAPIAN_LIBRARIES}
    KF5::CalendarCore
    KF5::AkonadiSearchPIM
    KF5::AkonadiSearchXapian
    KF5::Codecs
)
