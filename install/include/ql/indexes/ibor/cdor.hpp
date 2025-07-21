/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005 StatPro Italia srl

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

/*! \file cdor.hpp
    \brief %CDOR rate
*/

#ifndef quantlib_cdor_hpp
#define quantlib_cdor_hpp

#include <ql/indexes/iborindex.hpp>
#include <ql/time/calendars/canada.hpp>
#include <ql/time/daycounters/actual365fixed.hpp>
#include <ql/currencies/america.hpp>

namespace QuantLib {

    //! %CDOR rate
    /*! Canadian Dollar Offered Rate fixed by IDA.

        Conventions are taken from a number of sources including
        OpenGamma "Interest Rate Instruments and Market Conventions
        Guide", BBG, IKON.

        \warning This is the rate fixed in Canada by IDA. Use CADLibor
                 if you're interested in the London fixing by BBA.
    */
    class Cdor : public IborIndex {
      public:
        Cdor(const Period& tenor,
             const Handle<YieldTermStructure>& h = {})
        : IborIndex("CDOR", tenor, 0, CADCurrency(),
                    Canada(), ModifiedFollowing, false,
                    Actual365Fixed(), h) {}
    };

}


#endif
