/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Giorgio Facchinetti

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

/*! \file spreadedcapletvolstructure.hpp
    \brief Spreaded caplet volatility
*/

#ifndef quantlib_spreaded_caplet_volstructure_h
#define quantlib_spreaded_caplet_volstructure_h

#include <ql/voltermstructures/interestrate/caplet/optionletvolatilitystructure.hpp>
#include <ql/quote.hpp>

namespace QuantLib {

    class SpreadedOptionletVol : public OptionletVolatilityStructure {
      public:
        SpreadedOptionletVol(
            const Handle<OptionletVolatilityStructure>& underlyingVolStructure,
            const Handle<Quote>& spread);

      protected:
        // All virtual methods of base classes must be forwarded
        //! \name TermStructure interface
        //@{
        DayCounter dayCounter() const;
        Date maxDate() const;
        Time maxTime() const;
        const Date& referenceDate() const;
        Calendar calendar() const;
        Natural settlementDays() const;
        //@}
        //! \name OptionletVolatilityStructure interface
        //@{
        Rate minStrike() const;
        Rate maxStrike() const;
        BusinessDayConvention businessDayConvention() const;

        Volatility volatilityImpl(Time length,
                                  Rate strike) const;
        //@} 

    private:
        const Handle<OptionletVolatilityStructure> underlyingVolStructure_;
        const Handle<Quote> spread_;

    };

}

#endif
