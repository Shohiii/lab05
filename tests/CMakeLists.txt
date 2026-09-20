add_executable(check
    bank_test.cpp
)

target_link_libraries(check
    banking
    gmock_main
)

add_test(NAME banking_tests COMMAND check)
