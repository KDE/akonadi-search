kde_enable_exceptions()
include_directories(
    ${XAPIAN_INCLUDE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

add_executable(akonadi_indexing_agent)

target_sources(akonadi_indexing_agent PRIVATE
    abstractindexer.cpp
    agent.cpp
    emailindexer.cpp
    contactindexer.cpp
    akonotesindexer.cpp
    calendarindexer.cpp
    collectionindexer.cpp
    priority.cpp
    scheduler.cpp
    collectionindexingjob.cpp
    index.cpp
    collectionupdatejob.cpp
)

ecm_qt_declare_logging_category(akonadi_indexing_agent HEADER akonadi_indexer_agent_debug.h IDENTIFIER AKONADI_INDEXER_AGENT_LOG CATEGORY_NAME org.kde.pim.akonadi_indexer_agent
        DESCRIPTION "akonadisearch (akonadi indexer agent)"
        OLD_CATEGORY_NAMES log_akonadi_indexer_agent
        EXPORT AKONADISEARCH
    )

qt_add_dbus_adaptor(akonadi_indexing_agent_SRCS org.freedesktop.Akonadi.Indexer.xml agent.h AkonadiIndexingAgent)
target_sources(akonadi_indexing_agent PRIVATE ${akonadi_indexing_agent_SRCS})

if (COMPILE_WITH_UNITY_CMAKE_SUPPORT)
    set_target_properties(akonadi_indexing_agent PROPERTIES UNITY_BUILD ON)
endif()

target_link_libraries(akonadi_indexing_agent
    Qt::Core
    KF5::AkonadiCore
    KF5::AkonadiMime
    KF5::Mime
    KF5::Contacts
    KF5::AkonadiAgentBase
    KF5::CalendarCore
    KF5::AkonadiSearchXapian
    KF5::AkonadiSearchPIM
    KF5::Codecs
    KF5::I18n
    KF5::ConfigCore
    Qt::Widgets
)

install(TARGETS akonadi_indexing_agent ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES akonadiindexingagent.desktop
        DESTINATION "${KDE_INSTALL_DATAROOTDIR}/akonadi/agents")

if(BUILD_TESTING)
    add_subdirectory(tests)
    add_subdirectory(autotests)
endif()
