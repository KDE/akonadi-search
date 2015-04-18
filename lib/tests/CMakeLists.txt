include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/..
)

add_executable(contactsearch contactsearchtest.cpp)

target_link_libraries(contactsearch
  Qt5::Core

  KF5::AkonadiCore
  KF5::AkonadiMime
  KF5::Mime
  KF5::Contacts

  KF5::BalooPim
)

add_executable(emailquery emailquerytest.cpp)

target_link_libraries(emailquery
  Qt5::Core
  KF5::KDELibs4Support

  KF5::AkonadiCore
  KF5::Mime

  KF5::BalooPim
)

add_executable(contactcompleter contactcompletiontest.cpp)

target_link_libraries(contactcompleter
  Qt5::Core
  KF5::AkonadiCore
  KF5::Mime
  KF5::BalooPim
)
