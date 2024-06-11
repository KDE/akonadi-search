add_definitions(-DMAIL_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/testdata" )

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/..
)

set(indexer_SRCS
    ../emailindexer.cpp
    ../contactindexer.cpp
    ../akonotesindexer.cpp
    ../calendarindexer.cpp
    ../abstractindexer.cpp
    ../collectionindexer.cpp
    ../../search/pimsearchstore.cpp
    ../../search/email/emailsearchstore.cpp
    ../../search/email/agepostingsource.cpp
    ../../search/contact/contactsearchstore.cpp
    ../../search/calendar/calendarsearchstore.cpp
    ../akonadi_indexer_agent_debug.cpp
    ../akonadi_indexer_agent_calendar_debug.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/../../agent/akonadi_indexer_agent_email_debug.cpp
)

set(indexer_LIBS
    Qt::Test
    KPim6::AkonadiCore
    KPim6::AkonadiMime
    KPim6::AkonadiAgentBase
    KPim6::Mime
    KF6::Contacts
    KF6::CalendarCore
    KPim6::AkonadiSearchPIM
    KPim6::AkonadiSearchXapian
    KF6::I18n
    KF6::Codecs
    KF6::ConfigCore
    KF6::TextUtils
)

set(indexertest_SRCS
    indexertest.cpp
    ${indexer_SRCS}
)

add_definitions(-DAKONADI_SEARCH_NO_PLUGINS=TRUE)

add_executable(indexertest ${indexertest_SRCS})
add_test(NAME indexertest COMMAND indexertest)
ecm_mark_as_test(indexertest)
target_link_libraries(indexertest
    ${indexer_LIBS}
)

set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
if (KDEPIM_RUN_AKONADI_TEST)
	set(KDEPIMLIBS_RUN_ISOLATED_TESTS TRUE)
	set(KDEPIMLIBS_RUN_SQLITE_ISOLATED_TESTS TRUE)

	set(scheduler_SRCS
	    ../scheduler.cpp
	    ../index.cpp
	    ../collectionindexingjob.cpp
	    ${indexer_SRCS}
	)

	add_akonadi_isolated_test_advanced(schedulertest.cpp "${scheduler_SRCS}" "${indexer_LIBS}")
	add_akonadi_isolated_test_advanced(collectionindexingjobtest.cpp "${scheduler_SRCS}" "${indexer_LIBS}")


if(NOT WIN32 AND NOT APPLE)
add_executable(collectionquerytest collectionquerytest.cpp ${query_SRCS} ${indexer_SRCS} ${scheduler_SRCS})
add_test(NAME collectionquerytest COMMAND collectionquerytest)
ecm_mark_as_test(collectionquerytest)
target_link_libraries(collectionquerytest
    ${indexer_LIBS}
    KPim6::AkonadiSearchPIM
    KPim6::AkonadiAgentBase
    KF6::ConfigCore
    Qt::Widgets
    Qt::DBus
    KF6::TextUtils
)
endif()
endif()
