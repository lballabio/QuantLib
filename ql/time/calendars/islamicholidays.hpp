/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2026 SoftSolution srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file islamicholidays.hpp
    \brief Shared Islamic holiday date lookup tables (Eid al-Fitr, Eid al-Adha)
*/

#ifndef quantlib_islamic_holidays_hpp
#define quantlib_islamic_holidays_hpp

#include <ql/time/date.hpp>

namespace QuantLib {

    /*! For calculating Eid al-Fitr and Eid al-Adha, there are two main approaches (calendars) used in practice, and they sometimes overlap.
        Umm al-Qura calendar is used by Saudi Arabia (primary user) and some Gulf countries.
        Moon sighting calendar is used by South Asia, Central Asia, Middle East and North Africa.
    */
    namespace MoonSightingMethod {

        bool isEidAlFitr(const Date&);
        bool isEidAlAdha(const Date&);

    }

}

#endif // quantlib_islamic_holidays_hpp
