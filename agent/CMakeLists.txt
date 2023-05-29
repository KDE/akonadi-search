# SPDX-License-Identifier: CC0-1.0
# SPDX-FileCopyrightText: none
kde_enable_exceptions()
include_directories(
    ${XAPIAN_INCLUDE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)


add_executable(akonadi_indexing_agent)

if (Corrosion_FOUND)
    corrosion_import_crate(MANIFEST_PATH ${CMAKE_CURRENT_SOURCE_DIR}/rs/htmlparser/Cargo.toml)
    target_link_libraries(akonadi_indexing_agent PUBLIC htmlparser -Wl,--gc-sections)

    target_include_directories(akonadi_indexing_agent PRIVATE
        ${CMAKE_BINARY_DIR}/cargo/build/${Rust_CARGO_TARGET}/cxxbridge/htmlparser/src/)

    target_compile_definitions(akonadi_indexing_agent PRIVATE -DHAS_HTMLPARSER)
endif()

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

    abstractindexer.h
    agent.h
    emailindexer.h
    contactindexer.h
    akonotesindexer.h
    calendarindexer.h
    collectionindexer.h
    priority.h
    scheduler.h
    collectionindexingjob.h
    index.h
    collectionupdatejob.h

    stringutil.h
    stringutil.cpp
)

ecm_qt_declare_logging_category(akonadi_indexing_agent HEADER akonadi_indexer_agent_debug.h IDENTIFIER AKONADI_INDEXER_AGENT_LOG CATEGORY_NAME org.kde.pim.akonadi_indexer_agent
        DESCRIPTION "akonadisearch (akonadi indexer agent)"
        OLD_CATEGORY_NAMES log_akonadi_indexer_agent
        EXPORT AKONADISEARCH
    )

ecm_qt_declare_logging_category(akonadi_indexing_agent HEADER akonadi_indexer_agent_calendar_debug.h IDENTIFIER AKONADI_INDEXER_AGENT_CALENDAR_LOG CATEGORY_NAME org.kde.pim.akonadi_indexer_agent.calendar
        DESCRIPTION "akonadisearch (akonadi indexer agent calendar)"
        EXPORT AKONADISEARCH
    )

ecm_qt_declare_logging_category(akonadi_indexing_agent HEADER akonadi_indexer_agent_email_debug.h IDENTIFIER AKONADI_INDEXER_AGENT_EMAIL_LOG CATEGORY_NAME org.kde.pim.akonadi_indexer_agent.email
        DESCRIPTION "akonadisearch (akonadi indexer agent email)"
        EXPORT AKONADISEARCH
    )

qt_add_dbus_adaptor(akonadi_indexing_agent_SRCS org.freedesktop.Akonadi.Indexer.xml agent.h AkonadiIndexingAgent)
target_sources(akonadi_indexing_agent PRIVATE ${akonadi_indexing_agent_SRCS})

if (COMPILE_WITH_UNITY_CMAKE_SUPPORT)
    set_target_properties(akonadi_indexing_agent PROPERTIES UNITY_BUILD ON)
endif()

target_link_libraries(akonadi_indexing_agent
PRIVATE
    Qt::Core
    KPim6::AkonadiCore
    KPim6::AkonadiMime
    KPim6::Mime
    KF6::Contacts
    KPim6::AkonadiAgentBase
    KF6::CalendarCore
    KPim6::AkonadiSearchXapian
    KPim6::AkonadiSearchPIM
    KF6::Codecs
    KF6::I18n
    KF6::ConfigCore
    Qt::Widgets
)

install(TARGETS akonadi_indexing_agent ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES akonadiindexingagent.desktop
        DESTINATION "${KDE_INSTALL_DATAROOTDIR}/akonadi/agents")

if(BUILD_TESTING)
    add_subdirectory(tests)
    add_subdirectory(autotests)
endif()
