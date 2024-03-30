
add_executable(brcpp "main.cpp" "brc/brc.hpp" "brc/brc.cpp")
set_target_properties(brcpp PROPERTIES CXX_STANDARD 23)
target_link_libraries(brcpp 
    PUBLIC
    doctest::doctest
    mio::mio
)

add_executable(brcpptest "test.cpp"  "brc/brc.hpp" "brc/brc.cpp")
set_target_properties(brcpptest PROPERTIES CXX_STANDARD 23)
target_link_libraries(brcpptest 
    PUBLIC
    doctest::doctest
    mio::mio
)