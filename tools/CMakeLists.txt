# Copyright © 2025 CCP ehf.

find_package(cryptopp CONFIG REQUIRED)
find_package(CURL CONFIG REQUIRED)
find_package(ZLIB REQUIRED)

set(SRC_FILES
        include/BundleStreamIn.h
        include/BundleStreamOut.h
        include/ChunkIndex.h
        include/CompressedFileDataStreamOut.h
        include/Downloader.h
        include/FileDataStreamIn.h
        include/FileDataStreamOut.h
        include/GzipCompressionStream.h
        include/GzipDecompressionStream.h
        include/Md5ChecksumStream.h
        include/Patching.h
        include/ResourceTools.h
        include/RollingChecksum.h
        include/ScopedFile.h
        include/StatusCallback.h

        src/BundleStreamIn.cpp
        src/BundleStreamOut.cpp
        src/ChunkIndex.cpp
        src/CompressedFileDataStreamOut.cpp
        src/Downloader.cpp
        src/FileDataStreamIn.cpp
        src/FileDataStreamOut.cpp
        src/GzipCompressionStream.cpp
        src/GzipDecompressionStream.cpp
        src/Md5ChecksumStream.cpp
        src/ResourceTools.cpp
        src/ScopedFile.cpp
        src/Patching.cpp
        src/RollingChecksum.cpp
)

add_library(resources-tools STATIC ${SRC_FILES})

find_package(bsdiff CONFIG REQUIRED)

get_target_property(_SOURCES resources-tools SOURCES)
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}"
        PREFIX "Sources"
        FILES ${_SOURCES}
)

if (WIN32)
    target_compile_definitions(resources-tools PRIVATE NOMINMAX) # Do not define min/max macros.
endif ()

target_link_libraries(resources-tools PRIVATE cryptopp::cryptopp CURL::libcurl ZLIB::ZLIB static_bsdiff)

target_include_directories(resources-tools PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)


# Combine resource tools into single library
if (WIN32)

        set(RESOURCES_TOOLS_STANDALONE_LIB_NAME _resources_tools_static_standalone.lib)

        add_custom_command(TARGET resources-tools POST_BUILD
        COMMAND lib.exe 
                /OUT:${RESOURCES_TOOLS_STANDALONE_LIB_NAME}
                $<TARGET_FILE:resources-tools>
                $<TARGET_FILE:cryptopp::cryptopp>
                $<TARGET_FILE:ZLIB::ZLIB>
                $<TARGET_FILE:static_bsdiff>
                $<TARGET_FILE:CURL::libcurl_static>
                ws2_32.lib
                crypt32.lib
                /VERBOSE
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        )

elseif(APPLE)

        set(RESOURCES_TOOLS_STANDALONE_LIB_NAME _resources_tools_static_standalone.a)

        #libtool -static -o new.a old1.a old2.a
        add_custom_command(TARGET resources-tools POST_BUILD
                COMMAND libtool -static -o ${RESOURCES_TOOLS_STANDALONE_LIB_NAME}
                        $<TARGET_FILE:resources-tools>
                        $<TARGET_FILE:cryptopp::cryptopp>
                        $<TARGET_FILE:CURL::libcurl_static>
                        $<TARGET_FILE:ZLIB::ZLIB>
                        $<TARGET_FILE:static_bsdiff>
                WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                )

else()
    message(FATAL_ERROR "Unsupported platform")

endif()

add_library(resources-tools-static-standalone STATIC IMPORTED GLOBAL)
add_dependencies(resources-tools-static-standalone resources-tools)

set_target_properties(resources-tools-static-standalone
        PROPERTIES
        IMPORTED_LOCATION ${CMAKE_CURRENT_BINARY_DIR}/${RESOURCES_TOOLS_STANDALONE_LIB_NAME}
        INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_LIST_DIR}/include"
        )