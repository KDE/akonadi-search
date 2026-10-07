include(ECMAddTests)

function(add_lib_test src)
    get_filename_component(name ${src} NAME_WE)
    ecm_add_test(${src} ${ARGN}
        LINK_LIBRARIES Qt::Core Qt::Widgets Qt::Test KPim6::AkonadiCore KPim6AkonadiSearch
                       KPim6::AkonadiMime KF6::Contacts KF6::CalendarCore KF6::Mime
                       ${XAPIAN_LIBRARIES}
        TEST_NAME ${name}
    )
endfunction()

add_lib_test(querypropertymappertest.cpp ../src/querypropertymapper.cpp)
add_lib_test(querymappertest.cpp)
add_lib_test(storetest.cpp)
add_lib_test(contactcompletertest.cpp)

# These tests share the Xapian databases in the QStandardPaths test location
set_tests_properties(storetest contactcompletertest PROPERTIES RESOURCE_LOCK akonadi_search_db)
