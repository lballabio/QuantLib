
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

/*! \file stock.hpp
    \brief concrete stock class

    $Source$
    $Name$
    $Log$
    Revision 1.1  2001/04/09 14:05:48  nando
    all the *.hpp moved below the Include/ql level

    Revision 1.2  2001/04/06 18:46:20  nando
    changed Authors, Contributors, Licence and copyright header

    Revision 1.1  2001/04/04 11:07:22  nando
    Headers policy part 1:
    Headers should have a .hpp (lowercase) filename extension
    All *.h renamed to *.hpp

    Revision 1.4  2001/01/17 14:37:55  nando
    tabs removed

    Revision 1.3  2000/12/14 12:32:30  lballabio
    Added CVS tags in Doxygen file documentation blocks

*/

#ifndef quantlib_stock_h
#define quantlib_stock_h

#include "ql/qldefines.hpp"
#include "ql/instrument.hpp"

namespace QuantLib {

    namespace Instruments {

        class Stock : public PricedInstrument {
          public:
            Stock() {}
            Stock(const std::string& isinCode, const std::string& description)
            : PricedInstrument(isinCode,description) {}
            // modifiers
            void setPrice(double price) { PricedInstrument::setPrice(price); theNPV = price; }
            // inspectors
            bool useTermStructure() const { return false; }
            bool useSwaptionVolatility() const { return false; }
            bool useForwardVolatility() const { return false; }
          private:
            // methods
            bool needsFinalCalculations() const { return true; }
            void performFinalCalculations() const { price(); } // just check that it works
        };

    }

}


#endif
