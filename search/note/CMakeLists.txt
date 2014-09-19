add_library(notesearchstore MODULE notesearchstore.cpp ../pimsearchstore.cpp)

target_link_libraries(notesearchstore
    Qt5::Core
    ${XAPIAN_LIBRARIES}
    BalooPimCore
    BalooPimXapian
)

install(TARGETS notesearchstore DESTINATION ${PLUGIN_INSTALL_DIR}/kf5/baloo)
