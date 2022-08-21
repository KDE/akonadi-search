# SPDX-License-Identifier: CC0-1.0
# SPDX-FileCopyrightText: none
kde_enable_exceptions()
include_directories(
    ${XAPIAN_INCLUDE_DIR}
)
if (BUILD_QCH)
    ecm_install_qch_export(
        TARGETS KF5AkonadiSearchPIM_QCH
        FILE KF5AkonadiSearchPIMQchTargets.cmake
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel
    )
    set(PACKAGE_INCLUDE_QCHTARGETS "include(\"\${CMAKE_CURRENT_LIST_DIR}/KF5AkonadiSearchPIMQchTargets.cmake\")")
endif()

add_library(KF5AkonadiSearchPIM)
add_library(KF5::AkonadiSearchPIM ALIAS KF5AkonadiSearchPIM)


target_sources(KF5AkonadiSearchPIM PRIVATE
    query.cpp
    resultiterator.cpp
    contactquery.cpp
    emailquery.cpp
    notequery.cpp
    contactcompleter.cpp
    collectionquery.cpp
    indexeditems.cpp

    ../search/email/agepostingsource.cpp

    query.h
    resultiterator.h
    contactquery.h
    emailquery.h
    notequery.h
    contactcompleter.h
    collectionquery.h
    indexeditems.h

    ../search/email/agepostingsource.h
    )

ecm_qt_declare_logging_category(KF5AkonadiSearchPIM HEADER akonadi_search_pim_debug.h IDENTIFIER AKONADI_SEARCH_PIM_LOG CATEGORY_NAME org.kde.pim.akonadi_search_pim
        DESCRIPTION "akonadisearch(akonadi pim lib)"
        OLD_CATEGORY_NAMES log_akonadi_search_pim
        EXPORT AKONADISEARCH
    )

if (COMPILE_WITH_UNITY_CMAKE_SUPPORT)
    set_target_properties(KF5AkonadiSearchPIM PROPERTIES UNITY_BUILD ON)
endif()


generate_export_header(KF5AkonadiSearchPIM BASE_NAME AKONADI_SEARCH_PIM EXPORT_FILE_NAME search_pim_export.h)

target_link_libraries(KF5AkonadiSearchPIM
    PUBLIC
    Qt::Core

    PRIVATE
    KF5::AkonadiCore
    ${XAPIAN_LIBRARIES}
)

set_target_properties(KF5AkonadiSearchPIM PROPERTIES
    VERSION ${AKONADISEARCH_VERSION}
    SOVERSION ${AKONADISEARCH_SOVERSION}
    EXPORT_NAME AkonadiSearchPIM
)

target_include_directories(KF5AkonadiSearchPIM INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF}/AkonadiSearch>")

install(TARGETS KF5AkonadiSearchPIM EXPORT KF5AkonadiSearchTargets ${KF_INSTALL_TARGETS_DEFAULT_ARGS})

set(AKONADISEARCH_INCLUDES
    query.h
    contactquery.h
    emailquery.h
    contactcompleter.h
    notequery.h
    collectionquery.h
    indexeditems.h

    resultiterator.h
    )

install(FILES
    ${AKONADISEARCH_INCLUDES}
    ${CMAKE_CURRENT_BINARY_DIR}/search_pim_export.h

    DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/AkonadiSearch/PIM
    COMPONENT Devel
)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

if (BUILD_QCH)
    ecm_add_qch(
        KF5AkonadiSearchPIM_QCH
        NAME KF5AkonadiSearchPIM
        BASE_NAME KF5AkonadiSearchPIM
        VERSION ${PIM_VERSION}
        ORG_DOMAIN org.kde
        SOURCES # using only public headers, to cover only public API
        ${AKONADISEARCH_INCLUDES}
        MD_MAINPAGE "${CMAKE_SOURCE_DIR}/README.md"
        #IMAGE_DIRS "${CMAKE_SOURCE_DIR}/docs/pics"
        LINK_QCHS
            Qt5Core_QCH
            Qt5Gui_QCH
            Qt5Widgets_QCH
        INCLUDE_DIRS
            ${CMAKE_CURRENT_BINARY_DIR}
        BLANK_MACROS
            AKONADI_SEARCH_PIM_EXPORT
        TAGFILE_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        QCH_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        COMPONENT Devel
    )
endif()
