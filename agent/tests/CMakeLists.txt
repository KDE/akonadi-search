include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/..
)

add_executable(emailindexer emailtest.cpp ../emailindexer.cpp ../abstractindexer.cpp ../akonadi_indexer_agent_debug.cpp)

target_link_libraries(emailindexer
  Qt${QT_MAJOR_VERSION}::Test
  KF5::AkonadiCore
  KF5::AkonadiMime
  KF5::Mime
  KF5::AkonadiSearchPIM
  KF5::AkonadiSearchXapian
  KF5::Codecs
  Qt${QT_MAJOR_VERSION}::Widgets
)
