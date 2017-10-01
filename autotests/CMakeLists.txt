include(ECMAddTests)

function(add_lib_test src)
    get_filename_component(name ${src} NAME_WE)
    ecm_add_test(${src} ${ARGN}
        LINK_LIBRARIES Qt5::Core Qt5::Widgets Qt5::Test KF5::AkonadiCore KF5AkonadiSearch
                       ${XAPIAN_LIBRARIES} KF5::Contacts KF5::CalendarCore KF5::Mime
        TEST_NAME ${name})
endfunction()

add_lib_test(querypropertymappertest.cpp ../src/querypropertymapper.cpp)
add_lib_test(querymappertest.cpp)
add_lib_test(storetest.cpp)
