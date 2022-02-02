#include <iostream>
#include <exception>

#include "crypto-exchange-client-kucoin/client.hpp"

#include "api-secret.hpp"


int main()
{
	try {
		as::cryptox::kucoin::Client client( as::cryptox::kucoin::ApiKey(),
			as::cryptox::kucoin::ApiSecret(),
			as::cryptox::kucoin::ApiPassphrase() );

		client.run( []( as::cryptox::Client & c ) {
			std::cout << "ready" << std::endl;

			c.subscribePriceBookTicker( as::cryptox::Symbol::ALL,
				[]( as::cryptox::Client & c,
					as::cryptox::t_price_book_ticker & t ) {
					std::cout << "price book ticker: " << t.askPrice.toString()
							  << '/' << t.askQuantity.toString() << " - "
							  << t.bidPrice.toString() << '/'
							  << t.bidQuantity.toString() << std::endl;
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
