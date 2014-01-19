cmake_policy(SET CMP0015 NEW)

aux_source_directory(. SRC_LIST)

include_directories(../../secp256k1/include)
link_directories(../../secp256k1)

include_directories(../../cryptopp562)
link_directories(../../cryptopp562)

include_directories(../libethereum)
link_directories(../libethereum)

add_executable(eth ${SRC_LIST})

target_link_libraries(eth ethereum)
target_link_libraries(eth leveldb)
target_link_libraries(eth secp256k1)
target_link_libraries(eth cryptopp)
target_link_libraries(eth gmp)