cmake_policy(SET CMP0015 NEW)

aux_source_directory(. SRC_LIST)

include_directories(../libethereum)
link_directories(../libethereum)

add_executable(eth ${SRC_LIST})

if (${TARGET_PLATFORM} STREQUAL "w64")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libgcc -static-libstdc++")
	target_link_libraries(eth gcc)
	target_link_libraries(eth gdi32)
	target_link_libraries(eth ws2_32)
	target_link_libraries(eth mswsock)
	target_link_libraries(eth shlwapi)
	target_link_libraries(eth iphlpapi)
	target_link_libraries(eth cryptopp)
	target_link_libraries(eth boost_system-mt-s)
	target_link_libraries(eth boost_filesystem-mt-s)
	target_link_libraries(eth boost_thread_win32-mt-s)
	set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS)
elseif (UNIX)

else ()
	target_link_libraries(eth ${CRYPTOPP_LIBRARIES})
	target_link_libraries(eth boost_system)
	target_link_libraries(eth boost_filesystem)
	find_package(Threads REQUIRED)
	target_link_libraries(eth ${CMAKE_THREAD_LIBS_INIT})
endif ()

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	include_directories(/usr/local/include)
endif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

target_link_libraries(eth ethereum)
target_link_libraries(eth miniupnpc)
target_link_libraries(eth leveldb)
target_link_libraries(eth gmp)

install( TARGETS eth DESTINATION bin )

