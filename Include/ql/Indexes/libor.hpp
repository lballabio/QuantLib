
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

/*! \file libor.hpp
    \brief %Libor index

    $Id$
*/

// $Source$
// $Log$
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

#ifndef quantlib_libor_hpp
#define quantlib_libor_hpp

#include "ql/Indexes/xibor.hpp"
#include "ql/Calendars/london.hpp"
#include "ql/DayCounters/actual365.hpp"

namespace QuantLib {

    namespace Indexes {

        //! Libor index
        class Libor : public Xibor {
          public:
            Libor(const RelinkableHandle<TermStructure>& h)
            : Xibor("Libor", GBP, Handle<Calendar>(new Calendars::London),
                    true, ModifiedFollowing, 
                    Handle<DayCounter>(new DayCounters::Actual365), h) {}
        };

    }

}


#endif
