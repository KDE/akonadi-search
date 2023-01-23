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
    ../akonadi_indexer_agent_debug.cpp
    ../stringutil.cpp
)

set(indexer_LIBS
    Qt::Test
    KF5::AkonadiCore
    KF5::AkonadiMime
    KF5::AkonadiAgentBase
    KF5::Mime
    KF${KF_MAJOR_VERSION}::Contacts
    KF${KF_MAJOR_VERSION}::CalendarCore
    KF5::AkonadiSearchPIM
    KF5::AkonadiSearchXapian
    KF${KF_MAJOR_VERSION}::I18n
    KF${KF_MAJOR_VERSION}::Codecs
    KF${KF_MAJOR_VERSION}::ConfigCore
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


if(NOT WIN32)
add_executable(collectionquerytest collectionquerytest.cpp ${query_SRCS} ${indexer_SRCS} ${scheduler_SRCS})
add_test(NAME collectionquerytest COMMAND collectionquerytest)
ecm_mark_as_test(collectionquerytest)
target_link_libraries(collectionquerytest
    ${indexer_LIBS}
    ${QJSON_LIBRARIES}
    KF5::AkonadiSearchPIM
    KF5::AkonadiAgentBase
    KF${KF_MAJOR_VERSION}::ConfigCore
    Qt::Widgets
    Qt::DBus
)
endif()
endif()
