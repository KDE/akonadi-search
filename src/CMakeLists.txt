kde_enable_exceptions()
include_directories(
    ${XAPIAN_INCLUDE_DIR}
)

set(AKONADISEARCH_LIB_SRCS
    indexer.cpp
    querymapper.cpp
    querymapper_p.cpp
    querypropertymapper.cpp
    resultiterator.cpp
    store.cpp
    xapiandatabase.cpp
    xapiandocument.cpp
    xapiantermgenerator.cpp

    email/emailindexer.cpp
    email/emailquerymapper.cpp
    email/emailquerypropertymapper.cpp
    email/emailstore.cpp

    contact/contactgroupindexer.cpp
    contact/contactindexer.cpp
    contact/contactquerymapper.cpp
    contact/contactquerypropertymapper.cpp
    contact/contactstore.cpp

    incidence/incidenceindexer.cpp
    incidence/incidencequerymapper.cpp
    incidence/incidencequerypropertymapper.cpp
    incidence/incidencestore.cpp

    collection/collectionindexer.cpp
    collection/collectionquerymapper.cpp
    collection/collectionquerypropertymapper.cpp
    collection/collectionstore.cpp

    note/noteindexer.cpp
    note/notequerymapper.cpp
    note/notequerypropertymapper.cpp
    note/notestore.cpp
)

ecm_generate_headers(AkonadiSearch_LIB_HEADERS
    HEADER_NAMES
    Indexer
    ObjectCache
    QueryMapper
    ResultIterator
    Store
    REQUIRED_HEADERS AkonadiSearch_LIB_HEADERS
)

ecm_qt_declare_logging_category(
    AKONADISEARCH_LIB_SRCS HEADER akonadisearch_debug.h
    IDENTIFIER AKONADISEARCH_LOG
    CATEGORY_NAME org.kde.pim.akonadisearch_lib)

add_library(KF5AkonadiSearch ${AKONADISEARCH_LIB_SRCS})
add_library(KF5::AkonadiSearch ALIAS KF5AkonadiSearch)

generate_export_header(
    KF5AkonadiSearch BASE_NAME AKONADISEARCH EXPORT_FILE_NAME akonadisearch_export.h)

target_link_libraries(KF5AkonadiSearch
    PUBLIC
        Qt5::Core
        Qt5::Gui
        KF5::AkonadiCore
    PRIVATE
        KF5::Mime
        KF5::AkonadiMime
        KF5::Contacts
        KF5::CalendarCore
        ${XAPIAN_LIBRARIES}
)

set_target_properties(KF5AkonadiSearch PROPERTIES
    VERSION ${AKONADISEARCH_VERSION_STRING}
    SOVERSION ${AKONADISEARCH_SOVERSION}
    EXPORT_NAME AkonadiSearch
)

install(TARGETS KF5AkonadiSearch EXPORT KF5AkonadiSearchTargets ${KF5_INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/akonadisearch_export.h
    ${AkonadiSearch_LIB_HEADERS}
    DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5}/AkonadiSearch COMPONENT Devel
)
