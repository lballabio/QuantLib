
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

/*! \file usdlibor.hpp
    \fullpath Include/ql/Indexes/%usdlibor.hpp
    \brief %USD %Libor index

*/

// $Id$
// $Log$
// Revision 1.14  2001/08/09 14:59:46  sigmud
// header modification
//
// Revision 1.13  2001/08/08 11:07:48  sigmud
// inserting \fullpath for doxygen
//
// Revision 1.12  2001/08/07 11:25:53  sigmud
// copyright header maintenance
//
// Revision 1.11  2001/07/25 15:47:27  sigmud
// Change from quantlib.sourceforge.net to quantlib.org
//
// Revision 1.10  2001/06/15 13:52:07  lballabio
// Reworked indexes
//
// Revision 1.9  2001/06/12 09:36:37  lballabio
// name() now returns the class name
//
// Revision 1.8  2001/06/05 09:35:13  lballabio
// Updated docs to use Doxygen 1.2.8
//
// Revision 1.7  2001/06/01 16:50:16  lballabio
// Term structure on deposits and swaps
//
// Revision 1.6  2001/05/29 15:12:48  lballabio
// Reintroduced RollingConventions (and redisabled default extrapolation on PFF curve)
//
// Revision 1.5  2001/05/29 09:24:06  lballabio
// Using relinkable handle to term structure
//
// Revision 1.4  2001/05/24 15:38:08  nando
// smoothing #include xx.hpp and cutting old Log messages
//

#ifndef quantlib_usd_libor_hpp
#define quantlib_usd_libor_hpp

#include "ql/Indexes/xibor.hpp"
#include "ql/Calendars/newyork.hpp"
#include "ql/DayCounters/actual360.hpp"

namespace QuantLib {

    namespace Indexes {

        //! %USD %Libor index
        class USDLibor : public Xibor {
          public:
            USDLibor(int n, TimeUnit units, 
                const RelinkableHandle<TermStructure>& h)
            : Xibor("USDLibor", n, units, USD, 
                Handle<Calendar>(new Calendars::NewYork), 
                true, ModifiedFollowing, 
                Handle<DayCounter>(new DayCounters::Actual360), h) {}
        };

    }

}


#endif
