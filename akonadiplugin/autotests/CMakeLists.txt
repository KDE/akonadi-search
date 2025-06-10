include(ECMMarkAsTest)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_BINARY_DIR}/../../agent/
    ${CMAKE_CURRENT_BINARY_DIR}/..
)
add_executable(searchplugintest)

target_sources(
    searchplugintest
    PRIVATE
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
        ${CMAKE_CURRENT_BINARY_DIR}/../../agent/akonadi_indexer_agent_calendar_debug.cpp
        ${CMAKE_CURRENT_BINARY_DIR}/../../agent/akonadi_indexer_agent_email_debug.cpp
        ${CMAKE_CURRENT_BINARY_DIR}/../akonadiplugin_indexer_debug.cpp
)

add_definitions(-DAKONADI_SEARCH_NO_PLUGINS=TRUE)

add_test(NAME searchplugintest COMMAND searchplugintest)
ecm_mark_as_test(searchplugintest)

target_link_libraries(
    searchplugintest
    Qt::Test
    KPim6::AkonadiCore
    KPim6::AkonadiMime
    KPim6::Mime
    KF6::Contacts
    ${XAPIAN_LIBRARIES}
    KF6::CalendarCore
    KPim6::AkonadiSearchPIM
    KPim6::AkonadiSearchXapian
    KF6::Codecs
    KF6::TextUtils
)
