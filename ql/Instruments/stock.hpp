
/*
 Copyright (C) 2000, 2001, 2002 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software developed by the QuantLib Group; you can
 redistribute it and/or modify it under the terms of the QuantLib License;
 either version 1.0, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 QuantLib License for more details.

 You should have received a copy of the QuantLib License along with this
 program; if not, please email ferdinando@ametrano.net

 The QuantLib License is also available at http://quantlib.org/license.html
 The members of the QuantLib Group are listed in the QuantLib License
*/
/*! \file stock.hpp
    \brief concrete stock class

    \fullpath
    ql/Instruments/%stock.hpp
*/

// $Id$

#ifndef quantlib_stock_hpp
#define quantlib_stock_hpp

#include <ql/instrument.hpp>
#include <ql/marketelement.hpp>

namespace QuantLib {

    namespace Instruments {

        //! Simple stock class
        class Stock : public Instrument {
          public:
            Stock(const RelinkableHandle<MarketElement>& quote,
                const std::string& isinCode, const std::string& description);
            ~Stock();
          protected:
            void performCalculations() const;
          private:
            RelinkableHandle<MarketElement> quote_;
        };

    }

}


#endif
