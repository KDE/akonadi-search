include(ECMMarkAsTest)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/..
  ${CMAKE_CURRENT_BINARY_DIR}/../../agent/
  ${CMAKE_CURRENT_BINARY_DIR}/..
)
add_executable( searchplugintest)

target_sources(searchplugintest PRIVATE
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
    ../../agent/stringutil.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/../../agent/akonadi_indexer_agent_debug.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/../../agent/akonadi_indexer_agent_calendar_debug.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/../akonadiplugin_indexer_debug.cpp
)

add_definitions(-DAKONADI_SEARCH_NO_PLUGINS=TRUE)

add_test( NAME searchplugintest COMMAND searchplugintest )
ecm_mark_as_test(searchplugintest)

target_link_libraries(searchplugintest
    Qt::Test
    KPim${KF_MAJOR_VERSION}::AkonadiCore
    KPim${KF_MAJOR_VERSION}::AkonadiMime
    KPim${KF_MAJOR_VERSION}::Mime
    KF${KF_MAJOR_VERSION}::Contacts
    ${XAPIAN_LIBRARIES}
    KF${KF_MAJOR_VERSION}::CalendarCore
    KPim${KF_MAJOR_VERSION}::AkonadiSearchPIM
    KPim${KF_MAJOR_VERSION}::AkonadiSearchXapian
    KF${KF_MAJOR_VERSION}::Codecs
)
