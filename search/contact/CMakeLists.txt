add_library(contactsearchstore MODULE)
target_sources(
    contactsearchstore
    PRIVATE
        contactsearchstore.cpp
        ../pimsearchstore.cpp
        contactsearchstore.h
        ../pimsearchstore.h
)
target_link_libraries(
    contactsearchstore
    Qt::Core
    KPim6::AkonadiCore
    ${XAPIAN_LIBRARIES}
    KPim6AkonadiSearchCore
    KPim6AkonadiSearchXapian
)

install(
    TARGETS contactsearchstore
    DESTINATION ${KDE_INSTALL_PLUGINDIR}/pim6/akonadi
)
