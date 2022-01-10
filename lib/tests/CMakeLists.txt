include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/..
)

macro(add_search_test name)
    ecm_add_test("${name}test.cpp"
                TEST_NAME "${test}"
                LINK_LIBRARIES Qt${QT_MAJOR_VERSION}::Core KF5::AkonadiCore KF5::AkonadiMime KF5::Mime KF5::Contacts KF5::AkonadiSearchPIM
    )
endmacro()

# FIXME: isolated test
#add_search_test(contactsearch)
add_search_test(emailquery)
add_search_test(contactcompletion)
