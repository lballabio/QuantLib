/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2003, 2004, 2005, 2006 StatPro Italia srl

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

/*! \file cadlibor.hpp
    \brief %CAD %LIBOR rate
*/

#ifndef quantlib_cad_libor_hpp
#define quantlib_cad_libor_hpp

#include <ql/indexes/ibor/libor.hpp>
#include <ql/time/calendars/canada.hpp>
#include <ql/time/daycounters/actual365fixed.hpp>
#include <ql/currencies/america.hpp>

namespace QuantLib {

    //! %CAD LIBOR rate
    /*! Canadian Dollar LIBOR discontinued as of 2013.

        Conventions are taken from a number of sources including
        OpenGamma "Interest Rate Instruments and Market Conventions
        Guide", BBG, IKON.

        \warning This is the rate fixed in London by BBA. Use CDOR if
                 you're interested in the Canadian fixing by IDA.
    */
    class CADLibor : public Libor {
      public:
        CADLibor(const Period& tenor,
                 Handle<YieldTermStructure> h = {})
        : Libor("CADLibor", tenor,
                0,
                CADCurrency(),
                Canada(),
                Actual365Fixed(), std::move(h)) {}
    };

    //! Overnight %CAD %Libor index
    class CADLiborON : public DailyTenorLibor {
      public:
        explicit CADLiborON(Handle<YieldTermStructure> h = {})
        : DailyTenorLibor("CADLibor",
                          0,
                          CADCurrency(),
                          Canada(),
                          Actual365Fixed(), std::move(h)) {}
    };

}

#endif
