include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/..
)

macro(add_search_test name)
    ecm_add_test("${name}test.cpp"
                TEST_NAME "${test}"
                LINK_LIBRARIES Qt::Core KPim${KF_MAJOR_VERSION}::AkonadiCore KPim${KF_MAJOR_VERSION}::AkonadiMime KPim${KF_MAJOR_VERSION}::Mime KF${KF_MAJOR_VERSION}::Contacts KPim${KF_MAJOR_VERSION}::AkonadiSearchPIM
    )
endmacro()

# FIXME: isolated test
#add_search_test(contactsearch)
add_search_test(emailquery)
add_search_test(contactcompletion)
