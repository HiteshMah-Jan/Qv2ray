macro(add_android_full_lib libname)
    message("Adding ${libname}")
    list(APPEND ANDROID_EXTRA_LIBS ${libname})
endmacro()

QVLOG(CMAKE_ANDROID_ARCH)
if(CMAKE_BUILD_TYPE EQUAL "DEBUG")
    set (SSL_ROOT_PATH "${CMAKE_SOURCE_DIR}/3rdparty/android_openssl/no-asm/latest")
else()
    set (SSL_ROOT_PATH "${CMAKE_SOURCE_DIR}/3rdparty/android_openssl/latest")
endif()

add_android_full_lib("${SSL_ROOT_PATH}/arm/libcrypto_1_1.so")
add_android_full_lib("${SSL_ROOT_PATH}/arm/libssl_1_1.so")
add_android_full_lib("${SSL_ROOT_PATH}/arm64/libcrypto_1_1.so")
add_android_full_lib("${SSL_ROOT_PATH}/arm64/libssl_1_1.so")
add_android_full_lib("${SSL_ROOT_PATH}/x86/libcrypto_1_1.so")
add_android_full_lib("${SSL_ROOT_PATH}/x86/libssl_1_1.so")
add_android_full_lib("${SSL_ROOT_PATH}/x86_64/libcrypto_1_1.so")
add_android_full_lib("${SSL_ROOT_PATH}/x86_64/libssl_1_1.so")