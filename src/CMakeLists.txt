
add_executable(brcpp "main.cpp" "brc/brc.hpp" "brc/brc.cpp")
set_target_properties(brcpp PROPERTIES CXX_STANDARD 23)
target_link_libraries(brcpp 
    PUBLIC
    mio::mio
    absl::node_hash_map
    phmap::phmap
)

add_executable(brcpptest "test.cpp"  "brc/brc.hpp" "brc/brc.cpp")
set_target_properties(brcpptest PROPERTIES CXX_STANDARD 23)
target_link_libraries(brcpptest 
    PUBLIC
    doctest::doctest
    mio::mio
    absl::node_hash_map
    phmap::phmap
)

add_executable(brcppbench "bench.cpp"  "brc/brc.hpp" "brc/brc.cpp")
set_target_properties(brcppbench PROPERTIES CXX_STANDARD 23)
target_link_libraries(brcppbench 
    PUBLIC
    mio::mio
    nanobench::nanobench
    absl::node_hash_map
    phmap::phmap
)