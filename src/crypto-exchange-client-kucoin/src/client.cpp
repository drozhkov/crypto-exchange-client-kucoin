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

/// client.cpp
///
/// 0.0 - created (Denis Rozhkov <denis@rozhkoff.com>)
///

#include <iostream>

#include "boost/json.hpp"

#include "crypto-exchange-client-kucoin/client.hpp"
#include "crypto-exchange-client-kucoin/wsMessage.hpp"


namespace as::cryptox::kucoin {

	void Client::wsErrorHandler(
		WsClient & client, int code, const as::t_string & message )
	{
	}

	void Client::wsHandshakeHandler( WsClient & client )
	{
		client.readAsync();
	}

	void Client::wsReadHandler(
		WsClient & client, const char * data, size_t size )
	{

		try {
			// std::string s( data, size );
			// std::cout << s << std::endl;

			auto message = WsMessage::deserialize( data, size );

			switch ( message->TypeId() ) {
				case WsMessage::TypeIdWelcome: {
					auto m = static_cast<WsMessageWelcome *>( message.get() );
					m_connectId = m->Id();

					AS_CALL( m_clientReadyHandler, *this );
				}

				break;

				case WsMessage::TypeIdPriceBookTicker: {
					auto m = static_cast<WsMessagePriceBookTicker *>(
						message.get() );

					as::cryptox::t_price_book_ticker t;
					t.symbol = Symbol( m->SymbolName().c_str() );
					t.askPrice = std::move( m->AskPrice() );
					t.askQuantity = std::move( m->AskSize() );
					t.bidPrice = std::move( m->BidPrice() );
					t.bidQuantity = std::move( m->BidSize() );

					callSymbolHandler(
						t.symbol, m_priceBookTickerHandlerMap, t );
				}

				break;
			}
		}
		catch ( ... ) {
		}
	}

	void Client::apiReqBulletPublic()
	{
		auto res =
			m_httpClient.post( m_httpApiUrl.addPath( AS_T( "bullet-public" ) ),
				HttpHeaderList(),
				"" );

		auto v = boost::json::parse( res );
		auto & o = v.get_object();

		if ( o.contains( "code" ) && "200000" == o["code"].get_string() ) {
			auto & data = o["data"].get_object();
			auto & instanceServer =
				data["instanceServers"].get_array()[0].get_object();

			m_wsPingIntervalMs = instanceServer["pingInterval"].get_int64();
			m_wsTimeoutMs =
				m_wsPingIntervalMs + instanceServer["pingTimeout"].get_int64();

			m_token.assign( data["token"].get_string() );

			as::Url::parse( m_wsApiUrl,
				const_cast<const boost::json::value &>(
					instanceServer["endpoint"] )
						.get_string()
						.c_str() +
					std::string( "?token=" ) + m_token );
		}
		else {
			throw std::logic_error( "bullet-public" );
		}
	}

	void Client::initWsClient()
	{
		std::lock_guard<std::mutex> lock( m_wsPingSync );

		apiReqBulletPublic();

		as::cryptox::Client::initWsClient();

		if ( !m_wsPingThread.joinable() ) {
			m_wsPingThread = std::thread( [this] {
				while ( true ) {
					std::this_thread::sleep_for(
						std::chrono::milliseconds( m_wsPingIntervalMs ) );

					auto buffer = WsMessage::Ping();

					{
						std::lock_guard<std::mutex> lock( m_wsPingSync );
						// not 'ping' frame!?
						m_wsClient->writeAsync(
							buffer.c_str(), buffer.length() );
					}
				}
			} );
		}
	}

	void Client::run( const t_exchangeClientReadyHandler & handler )
	{
		m_clientReadyHandler = handler;

		while ( true ) {
			initWsClient();
			m_wsClient->run();
		}
	}

	void Client::subscribePriceBookTicker(
		as::cryptox::Symbol symbol, const t_priceBookTickerHandler & handler )
	{

		as::cryptox::Client::subscribePriceBookTicker( symbol, handler );
		auto buffer = WsMessage::Subscribe(
			std::string( "/market/ticker:" ) + SymbolName( symbol ) );

		m_wsClient->writeAsync( buffer.c_str(), buffer.length() );
	}

}
