
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
/*! \file usdlibor.hpp
    \brief %USD %Libor index

    \fullpath
    ql/Indexes/%usdlibor.hpp
*/

// $Id$

#ifndef quantlib_usd_libor_hpp
#define quantlib_usd_libor_hpp

#include <ql/Indexes/xibor.hpp>
#include <ql/Calendars/newyork.hpp>
#include <ql/DayCounters/actual360.hpp>

namespace QuantLib {

    namespace Indexes {

        //! %USD %Libor index
        class USDLibor : public Xibor {
          public:
            USDLibor(int n, TimeUnit units,
                const RelinkableHandle<TermStructure>& h)
            : Xibor("USDLibor", n, units, 2, USD,
                Calendar(Calendars::NewYork()), true, ModifiedFollowing,
                DayCounter(DayCounters::Actual360()), h) {}
        };

    }

}


#endif
