#pragma once


#include "crypto-exchange-client-core/core.hpp"


namespace as::cryptox::kucoin {

	inline const as::t_char * ApiKey()
	{
		return AS_T( "" );
	}

	inline const as::t_char * ApiSecret()
	{
		return AS_T( "api-secret" );
	}

	inline const as::t_char * ApiPassphrase()
	{
		return AS_T( "api-passphrase" );
	}

}
