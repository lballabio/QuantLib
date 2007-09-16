/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Ferdinando Ametrano
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

#include <ql/termstructures/volatilities/interestrate/cap/capfloorvolatilitystructure.hpp>
#include <ql/math/interpolation.hpp>
#include <ql/quote.hpp>
#include <ql/patterns/lazyobject.hpp>
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
    class CapVolatilityVector : public CapFloorVolatilityStructure,
                                public LazyObject  {
      public:
        //! floating reference date, floating market data
        CapVolatilityVector(Natural settlementDays,
                            const Calendar& calendar,
                            const std::vector<Period>& optionTenors,
                            const std::vector<Handle<Quote> >& volatilities,
                            BusinessDayConvention bdc = Following,
                            const DayCounter& dc = Actual365Fixed());        
        //! fixed reference date, floating market data
        CapVolatilityVector(const Date& settlementDate,
                            const Calendar& calendar,
                            const std::vector<Period>& optionTenors,
                            const std::vector<Handle<Quote> >& volatilities,
                            BusinessDayConvention bdc = Following,
                            const DayCounter& dc = Actual365Fixed());
        //! fixed reference date, fixed market data
        CapVolatilityVector(const Date& settlementDate,
                            const Calendar& calendar,
                            const std::vector<Period>& optionTenors,
                            const std::vector<Volatility>& volatilities,
                            BusinessDayConvention bdc = Following,
                            const DayCounter& dc = Actual365Fixed());
        //! floating reference date, fixed market data
        CapVolatilityVector(Natural settlementDays,
                            const Calendar& calendar,
                            const std::vector<Period>& optionTenors,
                            const std::vector<Volatility>& volatilities,
                            BusinessDayConvention bdc = Following,
                            const DayCounter& dc = Actual365Fixed());
        //! \name TermStructure interface
        //@{
        Date maxDate() const;
        //@}
        //! \name CapFloorVolatilityStructure interface
        //@{
        Real minStrike() const;
        Real maxStrike() const;
        //@}
        //! \name LazyObject interface
        //@{
        void update();
        void performCalculations() const;
        //@}
        //! \name some inspectors
        //@{
        const std::vector<Period>& optionTenors() const;
        const std::vector<Time>& optionTimes() const;
        const std::vector<Rate>& strikes() const;
        //@}
      private:
        void checkInputs(Size volatilitiesRows) const;
        void registerWithMarketData();
        std::vector<Period> optionTenors_;
        mutable std::vector<Time> optionTimes_;
        std::vector<Handle<Quote> > volHandles_;
        mutable std::vector<Volatility> volatilities_;
        mutable Interpolation interpolation_;
        void interpolate() const;
        Volatility volatilityImpl(Time length,
                                  Rate) const;
    };

    // inline definitions

    inline Date CapVolatilityVector::maxDate() const {
        calculate();
        return optionDateFromTenor(optionTenors_.back());
    }

    inline Real CapVolatilityVector::minStrike() const {
        return QL_MIN_REAL;
    }

    inline Real CapVolatilityVector::maxStrike() const {
        return QL_MAX_REAL;
    }

    inline void CapVolatilityVector::update() {
        //TermStructure::update();
        //LazyObject::update();
        CapFloorVolatilityStructure::update();
        interpolate();
    }

    inline Volatility CapVolatilityVector::volatilityImpl(Time t, 
                                                          Rate) const {
        calculate();
        return interpolation_(t, true);
    }

    inline const std::vector<Period>& CapVolatilityVector::optionTenors() const {
        return optionTenors_;
    }

    inline const std::vector<Time>& CapVolatilityVector::optionTimes() const {
        return optionTimes_;
    }

}

#endif
