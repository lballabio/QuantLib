
/*
 * Copyright (C) 2000-2001 QuantLib Group
 *
 * This file is part of QuantLib.
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.org/
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
 * if not, please email quantlib-users@lists.sourceforge.net
 * The license is also available at http://quantlib.org/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.org/group.html
*/

/*! \file stock.hpp
    \fullpath Include/ql/Instruments/%stock.hpp
    \brief concrete stock class

*/

// $Id$
// $Log$
// Revision 1.12  2001/08/09 14:59:46  sigmud
// header modification
//
// Revision 1.11  2001/08/08 11:07:48  sigmud
// inserting \fullpath for doxygen
//
// Revision 1.10  2001/08/07 11:25:53  sigmud
// copyright header maintenance
//
// Revision 1.9  2001/07/25 15:47:27  sigmud
// Change from quantlib.sourceforge.net to quantlib.org
//
// Revision 1.8  2001/07/16 16:07:42  lballabio
// Market elements and stuff
//
// Revision 1.7  2001/06/26 09:20:30  marmar
// Method set price added to class stock
//
// Revision 1.6  2001/06/22 16:38:15  lballabio
// Improved documentation
//
// Revision 1.5  2001/05/28 12:52:58  lballabio
// Simplified Instrument interface
//
// Revision 1.4  2001/05/24 15:38:08  nando
// smoothing #include xx.hpp and cutting old Log messages
//

#ifndef quantlib_stock_hpp
#define quantlib_stock_hpp

#include "ql/instrument.hpp"
#include "ql/marketelement.hpp"

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
