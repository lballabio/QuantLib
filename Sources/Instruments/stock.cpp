
/*
 * Copyright (C) 2000-2001 QuantLib Group
 *
 * This file is part of QuantLib.
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.sourceforge.net/
 *
 * QuantLib is free software and you are allowed to use, copy, modify, merge,
 * publish, distribute, and/or sell copies of it under the conditions stated
 * in the QuantLib License.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the license for more details.
 *
 * You should have received a copy of the license along with this file;
 * if not, contact ferdinando@ametrano.net
 * The license is also available at http://quantlib.sourceforge.net/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.sourceforge.net/Authors.txt
*/

/*! \file stock.cpp
    \brief concrete stock class

    $Id$
*/

// $Source$
// $Log$
// Revision 1.2  2001/07/16 16:07:42  lballabio
// Market elements and stuff
//
// Revision 1.1  2001/06/26 09:20:30  marmar
// Method set price added to class stock
//


#include "ql/Instruments/stock.hpp"

namespace QuantLib {

    namespace Instruments {

        Stock::Stock(const RelinkableHandle<MarketElement>& quote, 
            const std::string& isinCode, const std::string& description)
        : Instrument(isinCode,description), quote_(quote) {
            quote_.registerObserver(this);
        }

        Stock::~Stock() {
            quote_.unregisterObserver(this);
        }
        
        void Stock::performCalculations() const {
            QL_REQUIRE(!quote_.isNull(), 
                "null quote set for "+isinCode()+" stock");
            isExpired_ = false;
            NPV_ = quote_->value();
        }

    }

}

