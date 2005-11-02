/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000-2005 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file cadlibor.hpp
    \brief %CAD %LIBOR rate
*/

#ifndef quantlib_cad_libor_hpp
#define quantlib_cad_libor_hpp

#include <ql/Indexes/libor.hpp>
#include <ql/Calendars/unitedkingdom.hpp>
#include <ql/Calendars/toronto.hpp>
#include <ql/DayCounters/actual360.hpp>
#include <ql/Currencies/america.hpp>

namespace QuantLib {

    //! %CAD LIBOR rate
    /*! Canadian Dollar LIBOR fixed by BBA.

        See <http://www.bba.org.uk/bba/jsp/polopoly.jsp?d=225&a=1414>.

        \warning This is the rate fixed in London by BBA. Use CDOR if
                 you're interested in the Canadian fixing by IDA.
    */
    class CADLibor : public Libor {
      public:
        CADLibor(Integer n, TimeUnit units,
                 const Handle<YieldTermStructure>& h,
                 const DayCounter& dc = Actual360())
        : Libor("CADLibor", n, units, 2, CAD(),
                UnitedKingdom(UnitedKingdom::Exchange), Toronto(),
                ModifiedFollowing, dc, h) {}
    };

}


#endif
