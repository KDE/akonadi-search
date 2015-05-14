include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/..
)

add_executable(emailindexer emailtest.cpp ../emailindexer.cpp ../abstractindexer.cpp)

target_link_libraries(emailindexer
  Qt5::Test
  KF5::AkonadiCore
  KF5::AkonadiMime
  KF5::Mime
  KF5::AkonadiSearchPIM
  KF5::AkonadiSearchXapian
  KF5::Codecs
  Qt5::Widgets
)
