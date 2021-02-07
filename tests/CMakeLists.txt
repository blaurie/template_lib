INCLUDE_DIRECTORIES("../src")

include(FetchContent)

FetchContent_Declare(unity
		GIT_REPOSITORY "https://github.com/ThrowTheSwitch/Unity"
		GIT_TAG "v2.5.1")
FetchContent_MakeAvailable(unity)


add_executable(testarray test_array.c)
target_link_libraries(testarray unity)

add_executable(testflatmapzm test_flatmap_zero_mem.c)
target_link_libraries(testflatmapzm unity)

add_executable(testflatmapnzm test_flatmap_no_zero_mem.c)
target_link_libraries(testflatmapnzm unity)

add_executable(testhashalgo test_hash_algorithm.c)
target_link_libraries(testhashalgo unity)

add_executable(testutility test_utility.c)
target_link_libraries(testutility unity)
