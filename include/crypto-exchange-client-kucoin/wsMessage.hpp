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

/// wsMessage.hpp
///
/// 0.0 - created (Denis Rozhkov <denis@rozhkoff.com>)
///

#ifndef __CRYPTO_EXCHANGE_CLIENT_KUCOIN__WS_MESSAGE__H
#define __CRYPTO_EXCHANGE_CLIENT_KUCOIN__WS_MESSAGE__H


#include "boost/json.hpp"

#include "crypto-exchange-client-core/core.hpp"
#include "crypto-exchange-client-core/wsMessage.hpp"

#include "crypto-exchange-client-kucoin/apiMessage.hpp"


namespace as::cryptox::kucoin {

	class WsMessage : public as::cryptox::WsMessage {
	public:
		static const as::cryptox::t_api_message_type_id TypeIdWelcome = 100;
		static const as::cryptox::t_api_message_type_id TypeIdPriceBookTicker =
			101;

	protected:
		virtual void deserialize( boost::json::object & o ) = 0;

	public:
		WsMessage( t_api_message_type_id typeId )
			: as::cryptox::WsMessage( typeId )
		{
		}

		static std::shared_ptr<as::cryptox::ApiMessageBase> deserialize(
			const char * data, size_t size );

		static std::string Ping()
		{
			boost::json::object o;
			o["id"] = ApiMessage::RequestId();
			o["type"] = "ping";

			return boost::json::serialize( o );
		}

		static std::string Subscribe(
			const std::string & topicName, bool shouldSendResponse = false )
		{

			boost::json::object o;
			o["id"] = ApiMessage::RequestId();
			o["type"] = "subscribe";
			o["topic"] = topicName;
			o["response"] = shouldSendResponse;

			return boost::json::serialize( o );
		}
	};

	class WsMessageWelcome : public WsMessage {
	protected:
		std::string m_id;

	protected:
		void deserialize( boost::json::object & o ) override;

	public:
		WsMessageWelcome()
			: WsMessage( TypeIdWelcome )
		{
		}

		const std::string & Id() const
		{
			return m_id;
		}
	};

	class WsMessagePriceBookTicker : public WsMessage {
	protected:
		std::string m_symbolName;
		as::FixedNumber m_askPrice;
		as::FixedNumber m_askSize;
		as::FixedNumber m_bidPrice;
		as::FixedNumber m_bidSize;

	protected:
		void deserialize( boost::json::object & o ) override;

	public:
		WsMessagePriceBookTicker()
			: WsMessage( TypeIdPriceBookTicker )
		{
		}

		const std::string & SymbolName() const
		{
			return m_symbolName;
		}

		as::FixedNumber & AskPrice()
		{
			return m_askPrice;
		}

		as::FixedNumber & AskSize()
		{
			return m_askSize;
		}

		as::FixedNumber & BidPrice()
		{
			return m_bidPrice;
		}

		as::FixedNumber & BidSize()
		{
			return m_bidSize;
		}
	};

}


#endif
