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

    note/noteindexer.cpp
    note/notequerymapper.cpp
    note/notequerypropertymapper.cpp
    note/notestore.cpp
)

set(TMP
    query.cpp
    resultiterator.cpp
    store.cpp
    term.cpp
    xapiandatabase.cpp
    xapiandocument.cpp
    xapianqueryparser.cpp
    xapiansearchstore.cpp
    xapiantermgenerator.cpp
    stores/email/agepostingsource.cpp
    stores/email/emailsearchstore.cpp
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
