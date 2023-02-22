ecm_add_test(termgeneratortest.cpp termgeneratortest.h
    TEST_NAME "termgeneratortest"
    LINK_LIBRARIES Qt::Test KPim${KF_MAJOR_VERSION}::AkonadiSearchXapian
)

ecm_add_test(queryparsertest.cpp queryparsertest.h
    TEST_NAME "queryparsertest"
    LINK_LIBRARIES Qt::Test KPim${KF_MAJOR_VERSION}::AkonadiSearchXapian
)
