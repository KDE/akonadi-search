add_library(emailsearchstore MODULE)
target_sources(
    emailsearchstore
    PRIVATE
        agepostingsource.cpp
        emailsearchstore.cpp
        ../pimsearchstore.cpp
        agepostingsource.h
        emailsearchstore.h
        ../pimsearchstore.h
)
target_link_libraries(
    emailsearchstore
    Qt::Core
    KPim6::AkonadiCore
    ${XAPIAN_LIBRARIES}
    KPim6AkonadiSearchCore
    KPim6AkonadiSearchXapian
)

install(TARGETS emailsearchstore DESTINATION ${KDE_INSTALL_PLUGINDIR}/pim6/akonadi)
