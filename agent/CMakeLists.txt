# SPDX-License-Identifier: CC0-1.0
# SPDX-FileCopyrightText: none

## Html to text converter target

add_executable(akonadi_html_to_text htmltotext.cpp)

if (Corrosion_FOUND)
    corrosion_import_crate(MANIFEST_PATH ${CMAKE_CURRENT_SOURCE_DIR}/rs/htmlparser/Cargo.toml)
    target_link_libraries(akonadi_html_to_text PUBLIC htmlparser -Wl,--gc-sections)

    target_include_directories(akonadi_html_to_text PRIVATE
        ${CMAKE_BINARY_DIR}/cargo/build/${Rust_CARGO_TARGET}/cxxbridge/htmlparser/src/)

    target_compile_definitions(akonadi_html_to_text PRIVATE -DHAS_HTMLPARSER)
endif()

target_link_libraries(akonadi_html_to_text
PRIVATE
    Qt5::Core
    Qt5::Gui
    KF5::CoreAddons
    KF5::I18n
)

install(TARGETS akonadi_html_to_text ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

## Indexing agent target

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
    KPim${KF_MAJOR_VERSION}::AkonadiCore
    KPim${KF_MAJOR_VERSION}::AkonadiMime
    KPim${KF_MAJOR_VERSION}::Mime
    KF${KF_MAJOR_VERSION}::Contacts
    KPim${KF_MAJOR_VERSION}::AkonadiAgentBase
    KF${KF_MAJOR_VERSION}::CalendarCore
    KPim${KF_MAJOR_VERSION}::AkonadiSearchXapian
    KPim${KF_MAJOR_VERSION}::AkonadiSearchPIM
    KF${KF_MAJOR_VERSION}::Codecs
    KF${KF_MAJOR_VERSION}::I18n
    KF${KF_MAJOR_VERSION}::ConfigCore
    Qt::Widgets
)

install(TARGETS akonadi_indexing_agent ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES akonadiindexingagent.desktop
        DESTINATION "${KDE_INSTALL_DATAROOTDIR}/akonadi/agents")

if(BUILD_TESTING)
    add_subdirectory(tests)
    add_subdirectory(autotests)
endif()
