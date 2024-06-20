include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/..
)

macro(add_search_test name)
    ecm_add_test("${name}test.cpp"
                TEST_NAME "${test}"
                LINK_LIBRARIES Qt::Core KPim6::AkonadiCore KPim6::AkonadiMime KPim6::Mime KF6::Contacts KPim6::AkonadiSearchPIM
   )
endmacro()

# FIXME: isolated test
#add_search_test(contactsearch)
add_search_test(emailquery)
add_search_test(contactcompletion)
