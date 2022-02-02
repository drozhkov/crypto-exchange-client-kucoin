/*
MIT License
Copyright (c) 2022 Denis Rozhkov <denis@rozhkoff.com>
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

/// client.hpp
///
/// 0.0 - created (Denis Rozhkov <denis@rozhkoff.com>)
///

#ifndef __CRYPTO_EXCHANGE_CLIENT_KUCOIN__CLIENT__H
#define __CRYPTO_EXCHANGE_CLIENT_KUCOIN__CLIENT__H


#include "crypto-exchange-client-core/httpClient.hpp"
#include "crypto-exchange-client-core/client.hpp"

#include "crypto-exchange-client-kucoin/apiMessage.hpp"


namespace as::cryptox::kucoin {

	class Client : public as::cryptox::Client {
	protected:
		as::t_string m_apiKey;
		as::t_string m_apiKeyVersion;
		as::t_string m_apiSecret;
		as::t_string m_apiPassphrase;

		as::t_string m_token;
		as::t_string m_connectId;
		t_timespan m_wsPingIntervalMs;

		std::thread m_wsPingThread;
		std::mutex m_wsPingSync;

	private:
		void addAuthHeaders( HttpHeaderList & headers,
			const as::t_string & path,
			as::HttpMethod httpMethod,
			const as::t_string & body = AS_T( "" ) );

	protected:
		void wsErrorHandler(
			as::WsClient &, int, const as::t_string & ) override;

		void wsHandshakeHandler( as::WsClient & ) override;
		void wsReadHandler( as::WsClient &, const char *, size_t ) override;

		void initSymbolMap() override
		{
			addSymbolMapEntry(
				AS_T( "BTC-USDT" ), as::cryptox::Symbol::BTC_USDT );
		}

		void initWsClient() override;

	public:
		Client( const as::t_string & apiKey = AS_T( "" ),
			const as::t_string & apiSecret = AS_T( "" ),
			const as::t_string & apiPassphrase = AS_T( "" ),
			const as::t_string & httpApiUrl = AS_T(
				"https://api.kucoin.com/api/v1" ),
			int apiKeyVersion = 2 )
			: as::cryptox::Client(
				  httpApiUrl, AS_T( "wss://ws-api.kucoin.com/endpoint" ) )
			, m_apiKey( apiKey )
			, m_apiSecret( apiSecret )
			, m_apiPassphrase( apiPassphrase )
			, m_apiKeyVersion( AS_TOSTRING( apiKeyVersion ) )
		{
		}

		ApiResponseBullet apiReqBulletPublic();
		ApiResponseBullet apiReqBulletPrivate();

		void run( const t_exchangeClientReadyHandler & handler ) override;

		void subscribePriceBookTicker( as::cryptox::Symbol symbol,
			const t_priceBookTickerHandler & handler ) override;
	};

}


#endif
