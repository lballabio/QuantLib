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

#include <ql/voltermstructures/interestrate/capfloor/capfloortermvolatilitystructure.hpp>
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
    class CapFloorTermVolCurve : public CapFloorTermVolatilityStructure,
                                  public LazyObject  {
      public:
        //! floating reference date, floating market data
        CapFloorTermVolCurve(Natural settlementDays,
                              const Calendar& calendar,
                              const std::vector<Period>& optionTenors,
                              const std::vector<Handle<Quote> >& vols,
                              BusinessDayConvention bdc = Following,
                              const DayCounter& dc = Actual365Fixed());        
        //! fixed reference date, floating market data
        CapFloorTermVolCurve(const Date& settlementDate,
                              const Calendar& calendar,
                              const std::vector<Period>& optionTenors,
                              const std::vector<Handle<Quote> >& vols,
                              BusinessDayConvention bdc = Following,
                              const DayCounter& dc = Actual365Fixed());
        //! fixed reference date, fixed market data
        CapFloorTermVolCurve(const Date& settlementDate,
                              const Calendar& calendar,
                              const std::vector<Period>& optionTenors,
                              const std::vector<Volatility>& vols,
                              BusinessDayConvention bdc = Following,
                              const DayCounter& dc = Actual365Fixed());
        //! floating reference date, fixed market data
        CapFloorTermVolCurve(Natural settlementDays,
                              const Calendar& calendar,
                              const std::vector<Period>& optionTenors,
                              const std::vector<Volatility>& vols,
                              BusinessDayConvention bdc = Following,
                              const DayCounter& dc = Actual365Fixed());
        //! \name TermStructure interface
        //@{
        Date maxDate() const;
        //@}
        //! \name CapFloorTermVolatilityStructure interface
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
        const std::vector<Date>& optionDates() const;
        const std::vector<Time>& optionTimes() const;
        //@}
      protected:
        Volatility volatilityImpl(Time length,
                                  Rate) const;
      private:
        void checkInputs() const;
        void initializeOptionDatesAndTimes();
        void registerWithMarketData();
        void interpolate();

        Size nOptionTenors_;
        std::vector<Period> optionTenors_;
        mutable std::vector<Date> optionDates_;
        mutable std::vector<Time> optionTimes_;
        Date evaluationDate_;

        std::vector<Handle<Quote> > volHandles_;
        mutable std::vector<Volatility> vols_;

        // make it not mutable if possible
        mutable Interpolation interpolation_;
    };

    // inline definitions

    inline Date CapFloorTermVolCurve::maxDate() const {
        calculate();
        return optionDateFromTenor(optionTenors_.back());
    }

    inline Real CapFloorTermVolCurve::minStrike() const {
        return QL_MIN_REAL;
    }

    inline Real CapFloorTermVolCurve::maxStrike() const {
        return QL_MAX_REAL;
    }

    inline
    Volatility CapFloorTermVolCurve::volatilityImpl(Time t, 
                                                     Rate) const {
        calculate();
        return interpolation_(t, true);
    }

    inline
    const std::vector<Period>& CapFloorTermVolCurve::optionTenors() const {
        return optionTenors_;
    }

    inline
    const std::vector<Date>& CapFloorTermVolCurve::optionDates() const {
        // what if quotes are not available?
        calculate();
        return optionDates_;
    }

    inline
    const std::vector<Time>& CapFloorTermVolCurve::optionTimes() const {
        // what if quotes are not available?
        calculate();
        return optionTimes_;
    }

}

#endif
