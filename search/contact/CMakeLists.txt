add_library(contactsearchstore MODULE contactsearchstore.cpp ../pimsearchstore.cpp)

target_link_libraries(contactsearchstore
    Qt5::Core
    ${XAPIAN_LIBRARIES}
    BalooPimCore
    BalooPimXapian
)

install(TARGETS contactsearchstore DESTINATION ${KDE_INSTALL_PLUGINDIR}/kf5/baloo)
