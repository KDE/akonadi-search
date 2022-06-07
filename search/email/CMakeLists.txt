add_library(emailsearchstore MODULE )
target_sources(emailsearchstore PRIVATE agepostingsource.cpp emailsearchstore.cpp ../pimsearchstore.cpp agepostingsource.h emailsearchstore.h ../pimsearchstore.h)
target_link_libraries(emailsearchstore
    Qt${QT_MAJOR_VERSION}::Core
    KF5::AkonadiCore
    ${XAPIAN_LIBRARIES}
    KF5AkonadiSearchCore
    KF5AkonadiSearchXapian
)

install(TARGETS emailsearchstore DESTINATION ${KDE_INSTALL_PLUGINDIR}/pim${QT_MAJOR_VERSION}/akonadi)
