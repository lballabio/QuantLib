/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2003, 2004, 2005, 2006, 2007 StatPro Italia srl
 Copyright (C) 2004, 2005, 2006 Ferdinando Ametrano
 Copyright (C) 2006 Katiuscia Manzoni

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

/*! \file imm.hpp
    \brief IMM-related date functions
*/

#ifndef quantlib_imm_hpp
#define quantlib_imm_hpp

#include <ql/time/date.hpp>

namespace QuantLib {

    //! Main cycle of the International %Money Market (a.k.a. %IMM) months
    struct IMM {
        enum Month { F =  1, G =  2, H =  3,
                     J =  4, K =  5, M =  6,
                     N =  7, Q =  8, U =  9,
                     V = 10, X = 11, Z = 12 };

        //! returns whether or not the given date is an IMM date
        static bool isIMMdate(const Date& d,
                              bool mainCycle = true);

        //! returns whether or not the given string is an IMM code
        static bool isIMMcode(const std::string& in,
                              bool mainCycle = true);

        /*! returns the IMM code for the given date
            (e.g. H3 for March 20th, 2013).

            \warning It raises an exception if the input
                     date is not an IMM date
        */
        static std::string code(const Date& immDate);

        /*! returns the IMM date for the given IMM code
            (e.g. March 20th, 2013 for H3).

            \warning It raises an exception if the input
                     string is not an IMM code
        */
        static Date date(const std::string& immCode,
                         const Date& referenceDate = Date());

        //! next IMM date following the given date
        /*! returns the 1st delivery date for next contract listed in the
            International Money Market section of the Chicago Mercantile
            Exchange.
        */
        static Date nextDate(const Date& d = Date(),
                             bool mainCycle = true);

        //! next IMM date following the given IMM code
        /*! returns the 1st delivery date for next contract listed in the
            International Money Market section of the Chicago Mercantile
            Exchange.
        */
        static Date nextDate(const std::string& immCode,
                             bool mainCycle = true,
                             const Date& referenceDate = Date());

        //! next IMM code following the given date
        /*! returns the IMM code for next contract listed in the
            International Money Market section of the Chicago Mercantile
            Exchange.
        */
        static std::string nextCode(const Date& d = Date(),
                                    bool mainCycle = true);

        //! next IMM code following the given code
        /*! returns the IMM code for next contract listed in the
            International Money Market section of the Chicago Mercantile
            Exchange.
        */
        static std::string nextCode(const std::string& immCode,
                                    bool mainCycle = true,
                                    const Date& referenceDate = Date());
    };

}

#endif


#ifndef id_157635f61ade8160027ff1cf8cb175a0
#define id_157635f61ade8160027ff1cf8cb175a0
inline bool test_157635f61ade8160027ff1cf8cb175a0(const int* i) {
    return i != nullptr;
}
#endif
