add_library(emailsearchstore MODULE agepostingsource.cpp emailsearchstore.cpp ../pimsearchstore.cpp)

target_link_libraries(emailsearchstore
    Qt5::Core
    ${XAPIAN_LIBRARIES}
    KF5AkonadiSearchCore
    KF5AkonadiSearchXapian
)

install(TARGETS emailsearchstore DESTINATION ${KDE_INSTALL_PLUGINDIR}/akonadi)
