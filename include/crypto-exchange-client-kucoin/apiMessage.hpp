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

/// apiMessage.hpp
///
/// 0.0 - created (Denis Rozhkov <denis@rozhkoff.com>)
///

#ifndef __CRYPTO_EXCHANGE_CLIENT_KUCOIN__API_MESSAGE__H
#define __CRYPTO_EXCHANGE_CLIENT_KUCOIN__API_MESSAGE__H


#include "boost/json.hpp"

#include "crypto-exchange-client-core/core.hpp"
#include "crypto-exchange-client-core/exception.hpp"
#include "crypto-exchange-client-core/apiMessage.hpp"


namespace as::cryptox::kucoin {

	class ApiMessage : public ::as::cryptox::ApiMessage<ApiMessage> {
	};

	class ApiRequest : public ApiMessage {
	public:
		static as::t_string PlaceOrder( const as::t_stringview & directionName,
			const as::t_stringview & symbolName,
			const FixedNumber & price,
			const FixedNumber & quantity )
		{

			boost::json::object o;

			o["clientOid"] = uuidString();
			o["side"] = directionName.data();
			o["symbol"] = symbolName.data();
			o["price"] = price.toString();
			o["size"] = quantity.toString();

			return boost::json::serialize( o );
		}
	};

	class ApiResponseBullet : public ApiMessage {
	protected:
		int64_t m_pingInterval;
		int64_t m_pingTimeout;
		::as::t_string m_token;
		::as::t_string m_endpoint;

	public:
		static ApiResponseBullet deserialize( const ::as::t_string & s )
		{
			auto v = boost::json::parse( s );
			auto & o = v.get_object();

			if ( o.contains( "code" ) && "200000" == o["code"].get_string() ) {
				ApiResponseBullet result;

				auto & data = o["data"].get_object();
				auto & instanceServer =
					data["instanceServers"].get_array()[0].get_object();

				result.m_pingInterval =
					instanceServer["pingInterval"].get_int64();

				result.m_pingTimeout =
					instanceServer["pingTimeout"].get_int64();

				result.m_token.assign( data["token"].get_string() );
				result.m_endpoint.assign(
					instanceServer["endpoint"].get_string() );

				return result;
			}
			else {
				throw ::as::Exception( AS_T( "ApiResponseBullet" ) );
			}
		}

		int64_t PingInterval() const
		{
			return m_pingInterval;
		}

		int64_t PingTimeout() const
		{
			return m_pingTimeout;
		}

		::as::t_string & Token()
		{
			return m_token;
		}

		::as::t_string & Endpoint()
		{
			return m_endpoint;
		}
	};

	class ApiResponseOrders : public ApiMessage {
	protected:
		::as::t_string m_orderId;

	public:
		static ApiResponseOrders deserialize( const ::as::t_string & s )
		{
			auto v = boost::json::parse( s );
			auto & o = v.get_object();

			if ( o.contains( "code" ) && "200000" == o["code"].get_string() ) {
				ApiResponseOrders result;

				auto & data = o["data"].get_object();
				result.m_orderId.assign( data["orderId"].get_string() );

				return result;
			}
			else {
				throw ::as::Exception( AS_T( "ApiResponseOrders" ) );
			}
		}

		::as::t_string & OrderId()
		{
			return m_orderId;
		}
	};

	class ApiResponseSymbols : public ApiMessage {
	public:
		struct Pair {
			as::t_string name;
			as::t_string baseName;
			as::t_string quoteName;
			FixedNumber baseMinSize;
			FixedNumber quoteMinSize;
			FixedNumber baseIncrement;
			FixedNumber quoteIncrement;
			FixedNumber priceIncrement;
		};

	protected:
		std::vector<Pair> m_pairs;

	public:
		static ApiResponseSymbols deserialize( const ::as::t_string & s )
		{
			auto v = boost::json::parse( s );

			auto & root = v.get_object();

			if ( !root.contains( "code" ) ||
				"200000" != root["code"].get_string() ) {

				throw ::as::Exception( AS_T( "ApiResponseOrders" ) );
			}

			auto & a = root["data"].get_array();

			ApiResponseSymbols result;

			for ( size_t i = 0; i < a.size(); i++ ) {
				auto & o = a[i].get_object();

				Pair p;
				p.name = o["symbol"].get_string();
				p.baseName = o["baseCurrency"].get_string();
				p.quoteName = o["quoteCurrency"].get_string();
				p.baseMinSize = FixedNumber( o["baseMinSize"].get_string() );
				p.quoteMinSize = FixedNumber( o["quoteMinSize"].get_string() );
				p.baseIncrement =
					FixedNumber( o["baseIncrement"].get_string() );

				p.quoteIncrement =
					FixedNumber( o["quoteIncrement"].get_string() );

				p.priceIncrement =
					FixedNumber( o["priceIncrement"].get_string() );

				result.m_pairs.push_back( std::move( p ) );
			}

			return result;
		}

		const std::vector<Pair> & Pairs() const
		{
			return m_pairs;
		}
	};

}


#endif
