add_library(contactsearchstore MODULE contactsearchstore.cpp ../pimsearchstore.cpp)

target_link_libraries(contactsearchstore
    Qt5::Core
    ${XAPIAN_LIBRARIES}
    BalooPimCore
    BalooPimXapian
)

install(TARGETS contactsearchstore DESTINATION ${PLUGIN_INSTALL_DIR}/kf5/baloo)
