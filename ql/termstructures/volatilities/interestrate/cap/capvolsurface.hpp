/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Katiuscia Manzoni

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

/*! \file capvolsurface.hpp
    \brief Cap/floor smile volatility surface
*/

#ifndef quantlib_cap_volatility_surface_hpp
#define quantlib_cap_volatility_surface_hpp

#include <ql/termstructures/capvolstructures.hpp>
#include <ql/math/interpolations/interpolation2d.hpp>
#include <ql/quote.hpp>
#include <vector>

namespace QuantLib {

    //! Cap/floor smile volatility surface
    /*! This class provides the volatility for a given cap by 
        interpolating a volatility surface whose elements are the 
        market term volatilities of a set of caps/floors with given
        length and given strike.
    */
    class CapVolatilitySurface : public CapVolatilityStructure {
      public:
        //! floating reference date, floating market data
        CapVolatilitySurface(Natural settlementDays,
                             const Calendar& calendar,
                             const std::vector<Period>& optionTenors,
                             const std::vector<Rate>& strikes,
                             const std::vector<std::vector<Handle<Quote> > >&,
                             const DayCounter& dc = Actual365Fixed());
        //! fixed reference date, floating market data
        CapVolatilitySurface(const Date& settlementDate,
                             const Calendar& calendar,
                             const std::vector<Period>& optionTenors,
                             const std::vector<Rate>& strikes,
                             const std::vector<std::vector<Handle<Quote> > >&,
                             const DayCounter& dc = Actual365Fixed());
        //! fixed reference date, fixed market data
        CapVolatilitySurface(const Date& settlementDate,
                             const Calendar& calendar,
                             const std::vector<Period>& optionTenors,
                             const std::vector<Rate>& strikes,
                             const Matrix& volatilities,
                             const DayCounter& dc = Actual365Fixed());
        //! floating reference date, fixed market data
        CapVolatilitySurface(Natural settlementDays,
                             const Calendar& calendar,
                             const std::vector<Period>& optionTenors,
                             const std::vector<Rate>& strikes,
                             const Matrix& volatilities,
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
        // (to be) LazyObject interface
        void performCalculations() const;

      private:
        void checkInputs(Size volatilitiesRows,
                         Size volatilitiesColumns) const;
        void registerWithMarketData();
        std::vector<Period> optionTenors_;
        std::vector<Time> optionTimes_;
        std::vector<Rate> strikes_;
        std::vector<std::vector<Handle<Quote> > > volHandles_;
        mutable Matrix volatilities_;
        Interpolation2D interpolation_;
        void interpolate();
        Date maxDate_;
        Volatility volatilityImpl(Time length,
                                  Rate strike) const;
    };

    // inline definitions

    inline Date CapVolatilitySurface::maxDate() const {
        return referenceDate()+optionTenors_.back();
    }

    inline Real CapVolatilitySurface::minStrike() const {
        return strikes_.front();
    }

    inline Real CapVolatilitySurface::maxStrike() const {
        return strikes_.back();
    }

    inline void CapVolatilitySurface::update() {
        CapVolatilityStructure::update();
        interpolate();
    }

    inline Volatility CapVolatilitySurface::volatilityImpl(Time t,
                                                           Rate strike) const {
        return interpolation_(strike, t);
    }

}

#endif
