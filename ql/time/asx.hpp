/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2003, 2004, 2005, 2006, 2007 StatPro Italia srl
 Copyright (C) 2004, 2005, 2006 Ferdinando Ametrano
 Copyright (C) 2006 Katiuscia Manzoni
 Copyright (C) 2015 Maddalena Zanzi

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file asx.hpp
    \brief ASX-related date functions
*/

#ifndef quantlib_asx_hpp
#define quantlib_asx_hpp

#include <ql/time/date.hpp>

namespace QuantLib {

    //! Main cycle of the Australian Securities Exchange (a.k.a. ASX) months
    struct ASX {
        enum Month { F =  1, G =  2, H =  3,
                     J =  4, K =  5, M =  6,
                     N =  7, Q =  8, U =  9,
                     V = 10, X = 11, Z = 12 };

        //! returns whether or not the given date is an ASX date
        static bool isASXdate(const Date& d,
                              bool mainCycle = true);

        //! returns whether or not the given string is an ASX code
        static bool isASXcode(const std::string& in,
                              bool mainCycle = true);

        /*! returns the ASX code for the given date
            (e.g. M5 for June 12th, 2015).

            \warning It raises an exception if the input
                     date is not an ASX date
        */
        static std::string code(const Date& asxDate);

        /*! returns the ASX date for the given ASX code
            (e.g. June 12th, 2015 for M5).

            \warning It raises an exception if the input
                     string is not an ASX code
        */
        static Date date(const std::string& asxCode,
                         const Date& referenceDate = Date());

        //! next ASX date following the given date
        /*! returns the 1st delivery date for next contract listed in the
            Australian Securities Exchange.
        */
        static Date nextDate(const Date& d = Date(),
                             bool mainCycle = true);

        //! next ASX date following the given ASX code
        /*! returns the 1st delivery date for next contract listed in the
            Australian Securities Exchange
        */
        static Date nextDate(const std::string& asxCode,
                             bool mainCycle = true,
                             const Date& referenceDate = Date());

        //! next ASX code following the given date
        /*! returns the ASX code for next contract listed in the
            Australian Securities Exchange
        */
        static std::string nextCode(const Date& d = Date(),
                                    bool mainCycle = true);

        //! next ASX code following the given code
        /*! returns the ASX code for next contract listed in the
            Australian Securities Exchange
        */
        static std::string nextCode(const std::string& asxCode,
                                    bool mainCycle = true,
                                    const Date& referenceDate = Date());
    };

}

#endif
