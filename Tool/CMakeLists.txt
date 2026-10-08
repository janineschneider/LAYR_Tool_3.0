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

    #examples
    Examples/example_figure4.cpp
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
