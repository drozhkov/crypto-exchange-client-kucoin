﻿#
cmake_minimum_required (VERSION 3.8)


#
project ("crypto-exchange-client-kucoin-demo")


#
##
set(LIBS
	crypto-exchange-client-kucoin
	crypto-exchange-client-core
)

##
link_directories(
	${Boost_LIBRARY_DIRS}
)

set(LIBS
	${LIBS}
	${Boost_SYSTEM_LIBRARY}
	${Boost_JSON_LIBRARY}
)

##
set(LIBS
	${LIBS}
	${OPENSSL_SSL_LIBRARY}
)

##
set(LIBS
	${LIBS}
	${OPENSSL_CRYPTO_LIBRARY}
)

##
if(NOT WIN32)
	set(LIBS
		${LIBS}
		pthread
	)
endif()

##
if(WIN32)
	set(LIBS
		${LIBS}
		bcrypt
	)
endif()


#
add_executable(${PROJECT_NAME} 
	_kucoin-demo.cpp
)


#
target_link_libraries(${PROJECT_NAME} ${LIBS})


#
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17)
