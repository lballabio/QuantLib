
/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

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

/*! \file stock.hpp
    \brief concrete stock class
*/

#ifndef quantlib_stock_hpp
#define quantlib_stock_hpp

#include <ql/instrument.hpp>
#include <ql/marketelement.hpp>

namespace QuantLib {

    //! Simple stock class
    class Stock : public Instrument {
      public:
        Stock(const RelinkableHandle<Quote>& quote);
        bool isExpired() const { return false; }
      protected:
        void performCalculations() const;
      private:
        RelinkableHandle<Quote> quote_;
    };

}


#endif
