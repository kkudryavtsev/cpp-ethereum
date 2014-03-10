cmake_policy(SET CMP0015 NEW)

aux_source_directory(. SRC_LIST)

include_directories(../libethereum)
link_directories(../libethereum)

# mongo stuff
include_directories(${PROJECT_SOURCE_DIR}/include)

find_library(MONGO_LIBRARY
    mongoclient
    ${PROJECT_SOURCE_DIR}/lib
)
# /mongo stuff


add_executable(etherchain ${SRC_LIST})


if (${TARGET_PLATFORM} STREQUAL "w64")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libgcc -static-libstdc++")
	target_link_libraries(etherchain gcc)
	target_link_libraries(etherchain gdi32)
	target_link_libraries(etherchain ws2_32)
	target_link_libraries(etherchain mswsock)
	target_link_libraries(etherchain shlwapi)
	target_link_libraries(etherchain iphlpapi)
	target_link_libraries(etherchain cryptopp)
	target_link_libraries(etherchain boost_system-mt-s)
	target_link_libraries(etherchain boost_filesystem-mt-s)
	target_link_libraries(etherchain boost_thread_win32-mt-s)
	set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS)
elseif (UNIX)

else ()
	target_link_libraries(etherchain ${CRYPTOPP_LIBRARIES})
	target_link_libraries(etherchain boost_system)
	target_link_libraries(etherchain boost_filesystem)
	find_package(Threads REQUIRED)
	target_link_libraries(etherchain ${CMAKE_THREAD_LIBS_INIT})
endif ()

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	include_directories(/usr/local/include)
endif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

target_link_libraries(etherchain ethereum)
target_link_libraries(etherchain miniupnpc)
target_link_libraries(etherchain leveldb)
target_link_libraries(etherchain gmp)
target_link_libraries(etherchain ${MONGO_LIBRARY})

install( TARGETS etherchain DESTINATION bin )

