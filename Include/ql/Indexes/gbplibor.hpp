
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

/*! \file gbplibor.hpp
    \brief %GBP %Libor index

    $Id$
*/

// $Source$
// $Log$
// Revision 1.2  2001/06/15 13:52:07  lballabio
// Reworked indexes
//
// Revision 1.1  2001/06/12 15:05:33  lballabio
// Renamed Libor to GBPLibor and LiborManager to XiborManager
//

#ifndef quantlib_gbp_libor_hpp
#define quantlib_gbp_libor_hpp

#include "ql/Indexes/xibor.hpp"
#include "ql/Calendars/london.hpp"
#include "ql/DayCounters/actual365.hpp"

namespace QuantLib {

    namespace Indexes {

        //! %GBP %Libor index
        class GBPLibor : public Xibor {
          public:
            GBPLibor(int n, TimeUnit units, 
                const RelinkableHandle<TermStructure>& h)
            : Xibor("GBPLibor", n, units, GBP, 
                Handle<Calendar>(new Calendars::London),
                true, ModifiedFollowing, 
                Handle<DayCounter>(new DayCounters::Actual365), h) {}
        };

    }

}


#endif
