kde_enable_exceptions()
include_directories(
    ${XAPIAN_INCLUDE_DIR}
)

set(AKONADISEARCH_LIB_SRCS
    contactcompleter.cpp
    indexeditems.cpp
    indexer.cpp
    querymapper.cpp
    querymapper_p.cpp
    querypropertymapper.cpp
    resultiterator.cpp
    searchrunner.cpp
    store.cpp
    xapiandatabase.cpp
    xapiandocument.cpp
    xapiantermgenerator.cpp
    utils.cpp

    email/emailindexer.cpp
    email/emailquerymapper.cpp
    email/emailquerypropertymapper.cpp
    email/emailstore.cpp

    emailcontacts/emailcontactsindexer.cpp
    emailcontacts/emailcontactsstore.cpp

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
    ContactCompleter
    IndexedItems
    Indexer
    ObjectCache
    QueryMapper
    ResultIterator
    SearchRunner
    Store
    REQUIRED_HEADERS AkonadiSearch_LIB_HEADERS
)

ecm_qt_declare_logging_category(
    AKONADISEARCH_LIB_SRCS HEADER akonadisearch_debug.h
    IDENTIFIER AKONADISEARCH_LOG
    CATEGORY_NAME org.kde.pim.akonadisearch_lib)

add_library(KPim6AkonadiSearch ${AKONADISEARCH_LIB_SRCS})
add_library(KPim6::AkonadiSearch ALIAS KPim6AkonadiSearch)

generate_export_header(
    KPim6AkonadiSearch BASE_NAME AKONADISEARCH EXPORT_FILE_NAME akonadisearch_export.h)

target_link_libraries(KPim6AkonadiSearch
    PUBLIC
        Qt::Core
        Qt::Gui
        KPim6::AkonadiCore
    PRIVATE
        KF6::Mime
        KPim6::AkonadiMime
        KF6::Contacts
        KF6::CalendarCore
        KF6::Codecs
        Qt::Concurrent
        ${XAPIAN_LIBRARIES}
)

target_include_directories(KPim6AkonadiSearch INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR}/KPim6/AkonadiSearch>")

set_target_properties(KPim6AkonadiSearch PROPERTIES
    VERSION ${AKONADISEARCH_VERSION}
    SOVERSION ${AKONADISEARCH_SOVERSION}
    EXPORT_NAME AkonadiSearch
)

install(TARGETS KPim6AkonadiSearch EXPORT KPim6AkonadiSearchTargets ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/akonadisearch_export.h
    ${AkonadiSearch_LIB_HEADERS}
    DESTINATION ${KDE_INSTALL_INCLUDEDIR}/KPim6/AkonadiSearch COMPONENT Devel
)
