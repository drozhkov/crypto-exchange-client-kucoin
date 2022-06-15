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

#include "boost/json.hpp"

#include "crypto-exchange-client-core/logger.hpp"
#include "crypto-exchange-client-core/exception.hpp"

#include "crypto-exchange-client-kucoin/client.hpp"
#include "crypto-exchange-client-kucoin/wsMessage.hpp"


namespace as::cryptox::kucoin {

	void Client::addAuthHeaders( HttpHeaderList & headers,
		const ::as::t_string & path,
		::as::HttpMethod httpMethod,
		const ::as::t_string & body )
	{

		auto ts = AS_TOSTRING( UnixTs<std::chrono::milliseconds>() );
		auto data = ts + HttpsClient::MethodName( httpMethod ) + path + body;
		auto s = ::as::hmacSha256( m_apiSecret, data );
		auto sign = toBase64( t_buffer( s.data(), s.size() ) );

		headers.add( AS_T( "KC-API-KEY" ), m_apiKey );
		headers.add( AS_T( "KC-API-SIGN" ), sign );
		headers.add( AS_T( "KC-API-TIMESTAMP" ), ts );

		auto p = as::hmacSha256( m_apiSecret, m_apiPassphrase );
		auto passphrase = toBase64( t_buffer( p.data(), p.size() ) );
		headers.add( AS_T( "KC-API-PASSPHRASE" ), passphrase );
		headers.add( AS_T( "KC-API-KEY-VERSION" ), m_apiKeyVersion );

		if ( !body.empty() ) {
			headers.add( AS_T( "Content-Type" ), AS_T( "application/json" ) );
		}
	}

	void Client::wsErrorHandler(
		WsClient & client, int code, const as::t_string & message )
	{
	}

	void Client::wsHandshakeHandler( WsClient & client )
	{
		client.readAsync();
	}

	bool Client::wsReadHandler(
		WsClient & client, const char * data, size_t size )
	{

		try {
			std::string s( data, size );
			std::cout << s << std::endl;

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
					t.symbol = toSymbol( m->SymbolName().c_str() );
					t.askPrice = std::move( m->AskPrice() );
					t.askQuantity = std::move( m->AskSize() );
					t.bidPrice = std::move( m->BidPrice() );
					t.bidQuantity = std::move( m->BidSize() );

					callSymbolHandler(
						t.symbol, m_priceBookTickerHandlerMap, t );
				}

				break;

				case WsMessage::TypeIdOrderUpdate: {
					auto m =
						static_cast<WsMessageOrderUpdate *>( message.get() );

					as::cryptox::t_order_update u;
					u.orderId = std::move( m->OrderId() );

					AS_CALL( m_orderUpdateHandler, *this, u );
				}

				break;
			}
		}
		catch ( ... ) {
		}

		return true;
	}

	ApiResponseBullet Client::apiReqBulletPublic()
	{
		auto url = m_httpApiUrl.addPath( AS_T( "bullet-public" ) );
		auto res = m_httpClient.post( url, HttpHeaderList() );

		return ApiResponseBullet::deserialize( res );
	}

	ApiResponseBullet Client::apiReqBulletPrivate()
	{
		auto url = m_httpApiUrl.addPath( AS_T( "bullet-private" ) );

		HttpHeaderList headers;
		addAuthHeaders( headers, url.Path(), HttpMethod::POST );

		auto res = m_httpClient.post( url, headers );

		return ApiResponseBullet::deserialize( res );
	}

	ApiResponseSymbols Client::apiReqSymbols()
	{
		auto url = m_httpApiUrl.addPath( AS_T( "symbols" ) );
		auto res = m_httpClient.get( url, HttpHeaderList() );

		return ApiResponseSymbols::deserialize( res );
	}

	void Client::initSymbolMap()
	{
		as::cryptox::Client::initSymbolMap();

		auto apiRes = apiReqSymbols();

		m_pairList.resize( apiRes.Pairs().size() + 2 );
		m_pairList[0] = as::cryptox::Pair( as::cryptox::Coin::_undef,
			as::cryptox::Coin::_undef,
			AS_T( "undefined" ) );

		size_t index = 1;

		for ( const auto & p : apiRes.Pairs() ) {
			std::cout << p.name << std::endl;

			as::cryptox::Coin quote = toCoin( p.quoteName.c_str() );
			as::cryptox::Coin base = toCoin( p.baseName.c_str() );

			as::cryptox::Pair pair( base,
				quote,
				p.name,
				p.baseMinSize,
				p.quoteMinSize,
				p.baseIncrement,
				p.quoteIncrement,
				p.priceIncrement );

			m_pairList[index] = pair;

			addSymbolMapEntry(
				p.name, static_cast<as::cryptox::Symbol>( index ) );

			index++;
		}
	}

	void Client::initWsClient()
	{
		std::lock_guard<std::mutex> lock( m_wsPingSync );

		ApiResponseBullet apiRes =
			m_apiKey.empty() ? apiReqBulletPublic() : apiReqBulletPrivate();

		m_wsPingIntervalMs = apiRes.PingInterval();
		m_wsTimeoutMs = m_wsPingIntervalMs + apiRes.PingTimeout();
		m_token = std::move( apiRes.Token() );

		as::Url::parse(
			m_wsApiUrl, apiRes.Endpoint() + AS_T( "?token=" ) + m_token );

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
		as::cryptox::Client::run( handler );

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
			std::string( "/market/ticker:" ) + toName( symbol ) );

		m_wsClient->write( buffer.c_str(), buffer.length() );
	}

	void Client::subscribeOrderUpdate( const t_orderUpdateHandler & handler )
	{
		as::cryptox::Client::subscribeOrderUpdate( handler );
		auto buffer =
			WsMessage::Subscribe( "/spotMarket/tradeOrders", false, true );

		m_wsClient->write( buffer.c_str(), buffer.length() );
	}

	t_order Client::placeOrder( Direction direction,
		as::cryptox::Symbol symbol,
		const FixedNumber & price,
		const FixedNumber & quantity )
	{

		as::t_string body = ApiRequest::PlaceOrder(
			toName( direction ), toName( symbol ), price, quantity );

		auto url = m_httpApiUrl.addPath( AS_T( "orders" ) );

		HttpHeaderList headers;
		addAuthHeaders( headers, url.Path(), HttpMethod::POST, body );

		auto res = m_httpClient.post( url, headers, body );
		// std::cout << res << std::endl;

		auto resOrders = ApiResponseOrders::deserialize( res );

		t_order result;
		result.id = std::move( resOrders.OrderId() );

		return result;
	}

}
