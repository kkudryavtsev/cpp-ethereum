cmake_policy(SET CMP0015 NEW)

aux_source_directory(. SRC_LIST)

include_directories(..)
link_directories(../libethcore)
link_directories(../libethereum)

add_executable(json_rpc ${SRC_LIST})


target_link_libraries(json_rpc ${CRYPTOPP_LIBRARIES})
target_link_libraries(json_rpc boost_system)
target_link_libraries(json_rpc boost_filesystem)
find_package(Threads REQUIRED)
target_link_libraries(json_rpc ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(json_rpc jsonrpc)

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
                        include_directories(/usr/local/include)
endif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

target_link_libraries(json_rpc ethereum)
target_link_libraries(json_rpc miniupnpc)
target_link_libraries(json_rpc leveldb)
target_link_libraries(json_rpc gmp)

install( TARGETS json_rpc DESTINATION bin )
