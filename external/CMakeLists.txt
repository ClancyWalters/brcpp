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