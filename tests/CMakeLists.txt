INCLUDE_DIRECTORIES("../src")

include(FetchContent)

FetchContent_Declare(unity
		GIT_REPOSITORY "https://github.com/ThrowTheSwitch/Unity"
		GIT_TAG "v2.5.1")
FetchContent_MakeAvailable(unity)

add_executable(testarray test_array.c)
target_link_libraries(testarray unity)
