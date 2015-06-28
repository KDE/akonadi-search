add_definitions(-DTRANSLATION_DOMAIN=\"akonadi_baloo_indexer\")

kde_enable_exceptions()
include_directories(
    ${XAPIAN_INCLUDE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)


set(akonadi_indexing_agent_SRCS
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
    akonadi_indexer_agent_debug.cpp
)

qt5_add_dbus_adaptor(akonadi_indexing_agent_SRCS org.freedesktop.Akonadi.Indexer.xml agent.h AkonadiIndexingAgent)

add_executable(akonadi_indexing_agent ${akonadi_indexing_agent_SRCS})

target_link_libraries(akonadi_indexing_agent
    Qt5::Core
    KF5::AkonadiCore
    KF5::AkonadiMime
    KF5::Mime
    KF5::Contacts
    KF5::AkonadiAgentBase
    KF5::ItemModels
    KF5::CalendarCore
    KF5::AkonadiSearchXapian
)

install(TARGETS akonadi_indexing_agent ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES akonadiindexingagent.desktop
        DESTINATION "${CMAKE_INSTALL_PREFIX}/share/akonadi/agents")

add_subdirectory(tests)
add_subdirectory(autotests)
