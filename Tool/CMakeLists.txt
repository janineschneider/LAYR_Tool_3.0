cmake_minimum_required(VERSION 3.19.2)

SET(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic -O0 -DHAVE_STRUCT_TIMESPEC")

project(layr)
set(carve FALSE)

#OS definition for OS comparions in source code
if (WIN32)
    add_compile_definitions(WINDOWS=TRUE)
endif()

SET(HEADER
##Header files
    ##Additionals
    getworkingdir.h

    ##Stream
    bytecontainer.h
    rootcontainer.h
    transformationcontainer.h

    ##Exceptions
    customexception.h

    ##Basics
    rule.h
    outputrule.h
    parallel.h
    tree.h

    #Operators
    SequentialOperators/seqcomp.h
    SequentialOperators/or.h
    ParallelOperators/union.h
    ParallelOperators/intersection.h
    ParallelOperators/minus.h

    #rules
    Rules/DOS/dos.h
    Rules/Ext3/ext3_fsl.h
    Rules/Ext3/ext3_fl.h
    Rules/Ext3/ext3_fsl_dfxml.h
    Rules/Ext3/ext3_fl_dfxml.h
    Rules/Ext4/ext4_fsl.h
    Rules/Ext4/ext4_fl.h
    Rules/NTFS/ntfs_fsl.h
    Rules/NTFS/ntfs_fl.h
    Rules/NTFS/ntfs_fl_dfxml.h
    Rules/NTFS/ntfs_fsl_dfxml.h
    Rules/FAT32/fat32_fsl.h
    Rules/FAT32/fat32_fl.h
    Rules/Carve/carve.h
    Rules/DFXML/dfxml_writer.h
    Rules/Carve/carve.h
)

SET(SOURCES
##Source files
    ##Stream
    rootcontainer.cpp
    transformationcontainer.cpp

    ##Data
    tree.cpp

    #Opertors
    SequentialOperators/seqcomp.cpp
    SequentialOperators/or.cpp
    ParallelOperators/union.cpp
    ParallelOperators/intersection.cpp
    ParallelOperators/minus.cpp

    #rules
    Rules/DOS/dos.cpp
    Rules/Ext3/ext3_fsl.cpp
    Rules/Ext3/ext3_fl.cpp
    Rules/Ext3/ext3_fsl_dfxml.cpp
    Rules/Ext3/ext3_fl_dfxml.cpp
    Rules/Ext4/ext4_fsl.cpp
    Rules/Ext4/ext4_fl.cpp
    Rules/NTFS/ntfs_fsl.cpp
    Rules/NTFS/ntfs_fl.cpp
    Rules/NTFS/ntfs_fl_dfxml.cpp
    Rules/NTFS/ntfs_fsl_dfxml.cpp
    Rules/FAT32/fat32_fsl.cpp
    Rules/FAT32/fat32_fl.cpp
    Rules/Carve/carve.cpp
    Rules/DFXML/dfxml_writer.cpp
    Rules/Carve/carve.cpp

    #Output Tests
    #Tests/output_tests/test_dos_output.cpp
    #Tests/output_tests/test_ext3_fsl_output.cpp
    #Tests/output_tests/test_ext3_fl_output.cpp
    #Tests/output_tests/test_ext4_fsl_output.cpp
    Tests/output_tests/test_ext4_fl_output.cpp
    #Tests/output_tests/test_ntfs_fsl_output.cpp
    #Tests/output_tests/test_ntfs_fl_output.cpp
    #Tests/output_tests/test_fat32_fsl.cpp
    #Tests/output_tests/test_fat32_fl_output.cpp
    #Tests/output_tests/test_or_output.cpp
    #Tests/output_tests/test_seqComp_output.cpp
    #Tests/output_tests/test_setops_output.cpp
    #Tests/output_tests/test_carve_output.cpp

    #Google Tests
    #Tests/test_dos.cpp
    #Tests/test_ext3_fsl.cpp
    #Tests/test_ext3_fl.cpp
    #Tests/test_ext4_fsl.cpp
    #Tests/test_ext4_fl.cpp
    #Tests/test_fat32_fsl.cpp
    #Tests/test_fat32_fl.cpp
    #Tests/test_or.cpp
    #Tests/test_carve.cpp
    #Tests/test_tree.cpp
    #Tests/test_ntfs_fsl.cpp
    #Tests/test_ntfs_fl.cpp
    #Tests/test_intersection.cpp
    #Tests/test_minus.cpp
    #Tests/test_union.cpp

)


add_executable(${PROJECT_NAME} ${HEADER} ${SOURCES})

###################################################
#                GoogleTest Setup                 #
###################################################


enable_testing()
add_compile_definitions(SOURCE_DIR="${CMAKE_SOURCE_DIR}")

# Download and link GoogleTest
include(FetchContent)
FetchContent_Declare(
    googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)

include(GoogleTest)

###################################################
#                    EXAMPLES                     #
###################################################

add_executable(example_figure4
    Examples/example_figure4.cpp
    tree.cpp
    rootcontainer.cpp
    transformationcontainer.cpp

    ParallelOperators/union.cpp
    SequentialOperators/seqcomp.cpp
    SequentialOperators/or.cpp

    Rules/DOS/dos.cpp
    Rules/Ext3/ext3_fsl.cpp
    Rules/Ext3/ext3_fl.cpp
    Rules/Ext4/ext4_fsl.cpp
    Rules/Ext4/ext4_fl.cpp
    Rules/Carve/carve.cpp
    
)

###################################################
#                  GOOGLE TESTS                   #
###################################################

add_executable(address_tree_tests
    Tests/test_tree.cpp
    Tests/fixtures/tree_test_fixture.cpp
    tree.cpp
    rootcontainer.cpp
    transformationcontainer.cpp
    Rules/DOS/dos.cpp
    Rules/FAT32/fat32_fsl.cpp
    Rules/FAT32/fat32_fl.cpp
    SequentialOperators/seqcomp.cpp
    Rules/Ext3/ext3_fsl.h
    Rules/Ext3/ext3_fl.h
)
target_include_directories(address_tree_tests PRIVATE ${CMAKE_SOURCE_DIR})  
target_link_libraries(address_tree_tests gtest gtest_main pthread)
gtest_discover_tests(address_tree_tests)

add_executable(test_ext3_fsl
    Tests/test_ext3_fsl.cpp
    tree.cpp
    rootcontainer.cpp
    transformationcontainer.cpp
    Rules/Ext3/ext3_fsl.cpp
    SequentialOperators/seqcomp.cpp
)
target_include_directories(test_ext3_fsl PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(test_ext3_fsl gtest gtest_main pthread)
gtest_discover_tests(test_ext3_fsl)

add_executable(test_ext3_fl
    Tests/test_ext3_fl.cpp
    tree.cpp
    rootcontainer.cpp
    transformationcontainer.cpp
    Rules/Ext3/ext3_fl.cpp
    Rules/Ext3/ext3_fsl.cpp
    SequentialOperators/seqcomp.cpp
)
target_include_directories(test_ext3_fl PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(test_ext3_fl gtest gtest_main pthread)
gtest_discover_tests(test_ext3_fl)

add_executable(test_ext4_fsl
    Tests/test_ext4_fsl.cpp
    Tests/fixtures/tree_test_fixture.cpp
    tree.cpp
    rootcontainer.cpp
    transformationcontainer.cpp
    Rules/Ext4/ext4_fsl.cpp
    SequentialOperators/seqcomp.cpp
)
target_include_directories(test_ext4_fsl PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(test_ext4_fsl gtest gtest_main pthread)
gtest_discover_tests(test_ext4_fsl)

add_executable(test_ext4_fl
    Tests/test_ext4_fl.cpp
    Tests/fixtures/tree_test_fixture.cpp
    tree.cpp
    rootcontainer.cpp
    transformationcontainer.cpp
    Rules/Ext4/ext4_fl.cpp
    SequentialOperators/seqcomp.cpp
)
target_include_directories(test_ext4_fl PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(test_ext4_fl gtest gtest_main pthread)
gtest_discover_tests(test_ext4_fl)

add_executable(test_fat32_fsl
    Tests/test_fat32_fsl.cpp
    tree.cpp
    rootcontainer.cpp
    transformationcontainer.cpp
    Rules/FAT32/fat32_fsl.cpp
    SequentialOperators/seqcomp.cpp
)
target_include_directories(test_fat32_fsl PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(test_fat32_fsl gtest gtest_main pthread)
gtest_discover_tests(test_fat32_fsl)

add_executable(test_fat32_fl
    Tests/test_fat32_fl.cpp
    tree.cpp
    rootcontainer.cpp
    transformationcontainer.cpp
    Rules/FAT32/fat32_fl.cpp
    Rules/FAT32/fat32_fsl.cpp
    SequentialOperators/seqcomp.cpp
)
target_include_directories(test_fat32_fl PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(test_fat32_fl gtest gtest_main pthread)
gtest_discover_tests(test_fat32_fl
)

add_executable(test_ntfs_fsl
    Tests/test_ntfs_fsl.cpp
    tree.cpp
    rootcontainer.cpp
    transformationcontainer.cpp
    Rules/NTFS/ntfs_fsl.cpp
    SequentialOperators/seqcomp.cpp
)
target_include_directories(test_ntfs_fsl PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(test_ntfs_fsl gtest gtest_main pthread)
gtest_discover_tests(test_ntfs_fsl)

add_executable(test_ntfs_fl
    Tests/test_ntfs_fl.cpp 
    tree.cpp
    rootcontainer.cpp
    transformationcontainer.cpp
    Rules/NTFS/ntfs_fsl.cpp
    Rules/NTFS/ntfs_fl.cpp
    SequentialOperators/seqcomp.cpp
)
target_include_directories(test_ntfs_fl PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(test_ntfs_fl gtest gtest_main pthread)
gtest_discover_tests(test_ntfs_fl)

add_executable(test_or
    Tests/test_or.cpp
    tree.cpp
    rootcontainer.cpp
    transformationcontainer.cpp
    SequentialOperators/or.cpp
)
target_include_directories(test_or PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(test_or gtest gtest_main pthread)
gtest_discover_tests(test_or)

add_executable(test_intersection
    Tests/test_intersection.cpp
    Tests/fixtures/tree_test_fixture.cpp
    tree.cpp
    rootcontainer.cpp
    transformationcontainer.cpp
    ParallelOperators/intersection.cpp
    SequentialOperators/seqcomp.cpp 
)
target_include_directories(test_intersection PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(test_intersection gtest gtest_main pthread)
gtest_discover_tests(test_intersection)

add_executable(test_minus
    Tests/test_minus.cpp
    Tests/fixtures/tree_test_fixture.cpp
    tree.cpp
    rootcontainer.cpp
    transformationcontainer.cpp
    ParallelOperators/minus.cpp
    SequentialOperators/seqcomp.cpp 
)
target_include_directories(test_minus PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(test_minus gtest gtest_main pthread)
gtest_discover_tests(test_minus)

add_executable(test_union
    Tests/test_union.cpp
    Tests/fixtures/tree_test_fixture.cpp
    tree.cpp
    rootcontainer.cpp
    transformationcontainer.cpp
    ParallelOperators/union.cpp
    SequentialOperators/seqcomp.cpp 
)
target_include_directories(test_union PRIVATE ${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/Tests/fixtures)
target_link_libraries(test_union gtest gtest_main pthread)
gtest_discover_tests(test_union)

add_executable(test_carve
    Tests/test_carve.cpp
    Tests/fixtures/tree_test_fixture.cpp
    tree.cpp
    rootcontainer.cpp
    transformationcontainer.cpp
    Rules/Carve/carve.cpp
)
target_include_directories(test_carve PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(test_carve gtest gtest_main pthread)
gtest_discover_tests(test_carve)
