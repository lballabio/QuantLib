/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Katiuscia Manzoni
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2003, 2004, 2005 StatPro Italia srl

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

/*! \file capflatvolvector.hpp
    \brief Cap/floor at-the-money flat volatility vector
*/

#ifndef quantlib_cap_volatility_vector_hpp
#define quantlib_cap_volatility_vector_hpp

#include <ql/termstructures/capvolstructures.hpp>
#include <ql/math/interpolation.hpp>
#include <ql/quote.hpp>
#include <vector>

namespace QuantLib {

    //! Cap/floor at-the-money term-volatility vector
    /*! This class provides the at-the-money volatility for a given
        cap by interpolating a volatility vector whose elements are
        the market volatilities of a set of caps/floors with given
        length.

        \todo either add correct copy behavior or inhibit copy. Right
              now, a copied instance would end up with its own copy of
              the length vector but an interpolation pointing to the
              original ones.
    */
    class CapVolatilityVector : public CapVolatilityStructure {
      public:
        //! floating reference date, floating market data
        CapVolatilityVector(Natural settlementDays,
                            const Calendar& calendar,
                            const std::vector<Period>& optionTenors,
                            const std::vector<Handle<Quote> >& volatilities,
                            const DayCounter& dc = Actual365Fixed());        
        //! fixed reference date, floating market data
        CapVolatilityVector(const Date& settlementDate,
                            const Calendar& calendar,
                            const std::vector<Period>& optionTenors,
                            const std::vector<Handle<Quote> >& volatilities,
                            const DayCounter& dc = Actual365Fixed());
        //! fixed reference date, fixed market data
        CapVolatilityVector(const Date& settlementDate,
                            const Calendar& calendar,
                            const std::vector<Period>& optionTenors,
                            const std::vector<Volatility>& volatilities,
                            const DayCounter& dc = Actual365Fixed());
        //! floating reference date, fixed market data
        CapVolatilityVector(Natural settlementDays,
                            const Calendar& calendar,
                            const std::vector<Period>& optionTenors,
                            const std::vector<Volatility>& volatilities,
                            const DayCounter& dc = Actual365Fixed());
        //! \name TermStructure interface
        //@{
        Date maxDate() const;
        //@}
        //! \name CapVolatilityStructure interface
        //@{
        Real minStrike() const;
        Real maxStrike() const;
        //@}
        // observability
        void update();
        // LazyObject interface
        void performCalculations() const;

      private:
        void checkInputs(Size volatilitiesRows) const;
        void registerWithMarketData();
        std::vector<Period> optionTenors_;
        std::vector<Time> optionTimes_;
        std::vector<Handle<Quote> > volHandles_;
        mutable std::vector<Volatility> volatilities_;
        Interpolation interpolation_;
        void interpolate();
        Date maxDate_;
        Volatility volatilityImpl(Time length,
                                  Rate) const;
    };

    // inline definitions

    inline Date CapVolatilityVector::maxDate() const {
        return referenceDate()+optionTenors_.back();
    }

    inline Real CapVolatilityVector::minStrike() const {
        return QL_MIN_REAL;
    }

    inline Real CapVolatilityVector::maxStrike() const {
        return QL_MAX_REAL;
    }

    inline void CapVolatilityVector::update() {
        CapVolatilityStructure::update();
        interpolate();
    }

    inline Volatility CapVolatilityVector::volatilityImpl(Time t, 
                                                          Rate) const {
        return interpolation_(t, true);
    }

}

#endif
