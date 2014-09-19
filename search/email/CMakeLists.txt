add_library(emailsearchstore MODULE agepostingsource.cpp emailsearchstore.cpp ../pimsearchstore.cpp)

target_link_libraries(emailsearchstore
    Qt5::Core
    ${XAPIAN_LIBRARIES}
    BalooPimCore
    BalooPimXapian
)

install(TARGETS emailsearchstore DESTINATION ${PLUGIN_INSTALL_DIR}/kf5/baloo)
