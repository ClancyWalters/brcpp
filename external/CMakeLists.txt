include(FetchContent)

FetchContent_Declare(
  doctest
  GIT_REPOSITORY https://github.com/doctest/doctest.git
  GIT_TAG        v2.4.11
)
FetchContent_MakeAvailable(doctest)

FetchContent_Declare(
  mio
  GIT_REPOSITORY https://github.com/vimpunk/mio.git
  GIT_TAG        master
)
FetchContent_MakeAvailable(mio)

FetchContent_Declare(
  nanobench
  GIT_REPOSITORY https://github.com/martinus/nanobench.git
  GIT_TAG        v4.3.11
)
FetchContent_MakeAvailable(nanobench)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
FetchContent_Declare(
  abseil
  GIT_REPOSITORY https://github.com/abseil/abseil-cpp.git
  GIT_TAG        20240116.1
)
FetchContent_MakeAvailable(abseil)