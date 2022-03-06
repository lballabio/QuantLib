/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Ferdinando Ametrano
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

/*! \file capfloortermvolsurface.hpp
    \brief Cap/floor smile volatility surface
*/

#ifndef quantlib_cap_floor_term_vol_surface_hpp
#define quantlib_cap_floor_term_vol_surface_hpp

#include <ql/termstructures/volatility/capfloor/capfloortermvolatilitystructure.hpp>
#include <ql/math/interpolations/interpolation2d.hpp>
#include <ql/quote.hpp>
#include <ql/patterns/lazyobject.hpp>
#include <ql/time/daycounters/actual365fixed.hpp>
#include <vector>

namespace QuantLib {

    //! Cap/floor smile volatility surface
    /*! This class provides the volatility for a given cap/floor interpolating
        a volatility surface whose elements are the market term volatilities
        of a set of caps/floors with given length and given strike.
    */
    class CapFloorTermVolSurface : public LazyObject, 
                                   public CapFloorTermVolatilityStructure {
      public:
        //! floating reference date, floating market data
        CapFloorTermVolSurface(Natural settlementDays,
                               const Calendar& calendar,
                               BusinessDayConvention bdc,
                               const std::vector<Period>& optionTenors,
                               const std::vector<Rate>& strikes,
                               const std::vector<std::vector<Handle<Quote> > >&,
                               const DayCounter& dc = Actual365Fixed());
        //! fixed reference date, floating market data
        CapFloorTermVolSurface(const Date& settlementDate,
                               const Calendar& calendar,
                               BusinessDayConvention bdc,
                               const std::vector<Period>& optionTenors,
                               const std::vector<Rate>& strikes,
                               const std::vector<std::vector<Handle<Quote> > >&,
                               const DayCounter& dc = Actual365Fixed());
        //! fixed reference date, fixed market data
        CapFloorTermVolSurface(const Date& settlementDate,
                               const Calendar& calendar,
                               BusinessDayConvention bdc,
                               const std::vector<Period>& optionTenors,
                               const std::vector<Rate>& strikes,
                               const Matrix& volatilities,
                               const DayCounter& dc = Actual365Fixed());
        //! floating reference date, fixed market data
        CapFloorTermVolSurface(Natural settlementDays,
                               const Calendar& calendar,
                               BusinessDayConvention bdc,
                               const std::vector<Period>& optionTenors,
                               const std::vector<Rate>& strikes,
                               const Matrix& volatilities,
                               const DayCounter& dc = Actual365Fixed());
        //! \name TermStructure interface
        //@{
        Date maxDate() const override;
        //@}
        //! \name VolatilityTermStructure interface
        //@{
        Real minStrike() const override;
        Real maxStrike() const override;
        //@}
        //! \name LazyObject interface
        //@{
        void update() override;
        void performCalculations() const override;
        //@}
        //! \name some inspectors
        //@{
        const std::vector<Period>& optionTenors() const;
        const std::vector<Date>& optionDates() const;
        const std::vector<Time>& optionTimes() const;
        const std::vector<Rate>& strikes() const;
        //@}
      protected:
        Volatility volatilityImpl(Time t, Rate strike) const override;

      private:
        void checkInputs() const;
        void initializeOptionDatesAndTimes() const;
        void registerWithMarketData();
        void interpolate();
        
        Size nOptionTenors_;
        std::vector<Period> optionTenors_;
        mutable std::vector<Date> optionDates_;
        mutable std::vector<Time> optionTimes_;
        Date evaluationDate_;

        Size nStrikes_;
        std::vector<Rate> strikes_;

        std::vector<std::vector<Handle<Quote> > > volHandles_;
        mutable Matrix vols_;

        // make it not mutable if possible
        mutable Interpolation2D interpolation_;
    };

    // inline definitions

    inline Date CapFloorTermVolSurface::maxDate() const {
        calculate();
        return optionDateFromTenor(optionTenors_.back());
    }

    inline Real CapFloorTermVolSurface::minStrike() const {
        return strikes_.front();
    }

    inline Real CapFloorTermVolSurface::maxStrike() const {
        return strikes_.back();
    }

    inline
    Volatility CapFloorTermVolSurface::volatilityImpl(Time t,
                                                      Rate strike) const {
        calculate();
        return interpolation_(strike, t, true);
    }

    inline
    const std::vector<Period>& CapFloorTermVolSurface::optionTenors() const {
        return optionTenors_;
    }

    inline
    const std::vector<Date>& CapFloorTermVolSurface::optionDates() const {
        // what if quotes are not available?
        calculate();
        return optionDates_;
    }

    inline
    const std::vector<Time>& CapFloorTermVolSurface::optionTimes() const {
        // what if quotes are not available?
        calculate();
        return optionTimes_;
    }

    inline const std::vector<Rate>& CapFloorTermVolSurface::strikes() const {
        return strikes_;
    }
}

#endif


#ifndef id_53ee1745a8fef6300028244fa2e20d03
#define id_53ee1745a8fef6300028244fa2e20d03
inline bool test_53ee1745a8fef6300028244fa2e20d03(const int* i) {
    return i != nullptr;
}
#endif
