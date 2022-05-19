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

/// wsMessage.cpp
///
/// 0.0 - created (Denis Rozhkov <denis@rozhkoff.com>)
///

#include "crypto-exchange-client-kucoin/wsMessage.hpp"


namespace as::cryptox::kucoin {

	std::shared_ptr<::as::cryptox::ApiMessageBase> WsMessage::deserialize(
		const char * data, size_t size )
	{

		auto v = boost::json::parse( { data, size } );
		auto & o = v.get_object();

		if ( !o.contains( "type" ) ) {
			return s_unknown;
		}

		WsMessage * r = nullptr;

		auto & typeName = o["type"].get_string();

		if ( "welcome" == typeName ) {
			r = new WsMessageWelcome;
		}
		if ( "message" == typeName ) {
			auto & topicName = o["topic"].get_string();

			if ( topicName.starts_with( "/market/ticker:" ) ) {
				r = new WsMessagePriceBookTicker;
			}
			else if ( topicName.starts_with( "/spotMarket/tradeOrders" ) ) {
				r = new WsMessageOrderUpdate;
			}
		}

		if ( nullptr == r ) {
			return s_unknown;
		}

		r->deserialize( o );

		return std::shared_ptr<::as::cryptox::WsMessage>( r );
	}

	//

	void WsMessageWelcome::deserialize( boost::json::object & o )
	{
		m_id.assign( o["id"].get_string() );
	}

	//

	void WsMessagePriceBookTicker::deserialize( boost::json::object & o )
	{
		auto & topicName = o["topic"].get_string();

		if ( "/market/ticker:all" == topicName ) {
			m_symbolName.assign( o["subject"].get_string() );
		}
		else {
			// TODO
		}

		auto & data = o["data"].get_object();
		m_askPrice.Value( data["bestAsk"].get_string() );
		m_askSize.Value( data["bestAskSize"].get_string() );
		m_bidPrice.Value( data["bestBid"].get_string() );
		m_bidSize.Value( data["bestBidSize"].get_string() );
	}

	//

	void WsMessageOrderUpdate::deserialize( boost::json::object & o )
	{
		auto & data = o["data"].get_object();
		m_orderId.assign( data["orderId"].get_string() );
	}

}
