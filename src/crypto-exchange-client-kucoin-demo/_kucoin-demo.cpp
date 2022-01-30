#include <iostream>
#include <exception>

#include "crypto-exchange-client-core/httpClient.hpp"

#include "crypto-exchange-client-kucoin/client.hpp"


int main()
{
	try {
		as::cryptox::kucoin::Client client;
		client.run( []( as::cryptox::Client & c ) {
			std::cout << "ready" << std::endl;

			c.subscribePriceBookTicker( as::cryptox::Symbol::ALL,
				[]( as::cryptox::Client & c,
					as::cryptox::t_price_book_ticker & t ) {
					std::cout << "price book ticker: " << t.askPrice.Value()
							  << '/' << t.askQuantity.Value() << " - "
							  << t.bidPrice.Value() << '/'
							  << t.bidQuantity.Value() << std::endl;
				} );
		} );
	}
	catch ( const std::exception & x ) {
		std::cerr << x.what() << std::endl;
	}
	catch ( ... ) {
		std::cerr << "error" << std::endl;
	}

	return 0;
}
