/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2004, 2005, 2006 Ferdinando Ametrano
 Copyright (C) 2006 Katiuscia Manzoni
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2003, 2004, 2005, 2006, 2008 StatPro Italia srl

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

/*! \file frequency.hpp
    \brief Frequency enumeration
*/

#ifndef quantlib_frequency_hpp
#define quantlib_frequency_hpp

#include <ql/qldefines.hpp>
#include <iosfwd>

namespace QuantLib {

    //! Frequency of events
    /*! \ingroup datetime */
    enum Frequency { NoFrequency = -1,     //!< null frequency
                     Once = 0,             //!< only once, e.g., a zero-coupon
                     Annual = 1,           //!< once a year
                     Semiannual = 2,       //!< twice a year
                     EveryFourthMonth = 3, //!< every fourth month
                     Quarterly = 4,        //!< every third month
                     Bimonthly = 6,        //!< every second month
                     Monthly = 12,         //!< once a month
                     EveryFourthWeek = 13, //!< every fourth week
                     Biweekly = 26,        //!< every second week
                     Weekly = 52,          //!< once a week
                     Daily = 365,          //!< once a day
                     OtherFrequency = 999  //!< some other unknown frequency
    };

    /*! \relates Frequency */
    std::ostream& operator<<(std::ostream& out,
                             Frequency f);

}

#endif
