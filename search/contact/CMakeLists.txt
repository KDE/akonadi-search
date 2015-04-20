add_library(contactsearchstore MODULE contactsearchstore.cpp ../pimsearchstore.cpp)

target_link_libraries(contactsearchstore
    Qt5::Core
    ${XAPIAN_LIBRARIES}
    KF5AkonadiSearchCore
    KF5AkonadiSearchXapian
)

install(TARGETS contactsearchstore DESTINATION ${KDE_INSTALL_PLUGINDIR}/akonadi)
