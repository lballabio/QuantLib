
/*
 Copyright (C) 2004 Decillion Pty(Ltd)

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file currency.hpp
    \brief Currency base
*/

#ifndef quantlib_gbp_currency_hpp
#define quantlib_gbp_currency_hpp

#include <ql/currency.hpp>

namespace QuantLib {

    class GBPCurrency : public Currency {
    private:
	class Impl : public CurrencyImpl {
	public:
	    std::string name() const { return "British Pound"; };
	    std::string mnemonic() const { return "GBP"; };
	    std::string numeric() const { return "N/A"; };
	    std::string symbol() const { return ""; };
	    bool isRounding() const { return false; }
	    Rounding prefRounding() const { return Rounding(); }
	    Currency nextCurrency() const { return GBPCurrency(); }
	    Currency lastCurrency() const { return GBPCurrency(); }
	};
    public:
	GBPCurrency();
    };
    
}

#endif
