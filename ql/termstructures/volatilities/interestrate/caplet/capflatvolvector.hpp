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
#include <ql/math/interpolations/linearinterpolation.hpp>
#include <ql/math/interpolations/cubicspline.hpp>
#include <ql/time/daycounters/thirty360.hpp>
#include <ql/time/daycounters/actual365fixed.hpp>
#include <ql/quotes/simplequote.hpp>
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
                            const DayCounter& dayCounter);        
        //! fixed reference date, floating market data
        CapVolatilityVector(const Date& settlementDate,
                            const std::vector<Period>& optionTenors,
                            const std::vector<Handle<Quote> >& volatilities,
                            const DayCounter& dayCounter);
        //! fixed reference date, fixed market data
        CapVolatilityVector(const Date& settlementDate,
                            const std::vector<Period>& optionTenors,
                            const std::vector<Volatility>& volatilities,
                            const DayCounter& dayCounter);
        //! floating reference date, fixed market data
        CapVolatilityVector(Natural settlementDays,
                            const Calendar& calendar,
                            const std::vector<Period>& optionTenors,
                            const std::vector<Volatility>& volatilities,
                            const DayCounter& dayCounter);

        // inspectors
        DayCounter dayCounter() const { return dayCounter_; }
        Date maxDate() const;
        Real minStrike() const;
        Real maxStrike() const;
        // observability
        void update();
            //TermStructure::update();
            //LazyObject::update();

        // LazyObject interface
        void performCalculations() const;

      private:
        void checkInputs(Size volatilitiesRows,
                         Size volatilitiesColumns) const;
        void registerWithMarketData();
        DayCounter dayCounter_;
        std::vector<Period> optionTenors_;
        std::vector<Time> timeLengths_;
        std::vector<Handle<Quote> > volHandles_;
        mutable std::vector<double> volatilities_;
        Interpolation interpolation_;
        void interpolate();
        Date maxDate_;
        Volatility volatilityImpl(Time length,
                                  Rate) const;
    };

    // inline definitions
    // floating reference date, floating market data
    inline CapVolatilityVector::CapVolatilityVector(
                                Natural settlementDays,
                                const Calendar& calendar,
                                const std::vector<Period>& optionTenors,
                                const std::vector<Handle<Quote> >& volatilities,
                                const DayCounter& dayCounter)
    : CapVolatilityStructure(settlementDays, calendar),
      dayCounter_(dayCounter), 
      optionTenors_(optionTenors),
      timeLengths_(optionTenors.size()), 
      volHandles_(volatilities),
      volatilities_(volatilities.size()) {
          checkInputs(optionTenors.size(), volatilities.size());
          registerWithMarketData();
          //QL_REQUIRE(optionTenors.size() == volatilities.size(),
          //         "mismatch between number of cap lengths "
          //         "and cap volatilities");
        //volatilities_[0] = volatilities[0];
        //std::copy(volatilities.begin(),volatilities.end(),volatilities_.begin()+1);
        for (Size i=0; i<volatilities_.size(); ++i)
            volatilities_[i] = volHandles_[i]->value();
        interpolate();
    }
    
    // fixed reference date, floating market data
    inline CapVolatilityVector::CapVolatilityVector(
                                const Date& settlementDate,
                                const std::vector<Period>& optionTenors,
                                const std::vector<Handle<Quote> >& volatilities,
                                const DayCounter& dayCounter)
    : CapVolatilityStructure(settlementDate),
      dayCounter_(dayCounter), 
      optionTenors_(optionTenors),
      timeLengths_(optionTenors.size()), 
      volHandles_(volatilities),
      volatilities_(volatilities.size()) {
        checkInputs(optionTenors.size(), volatilities.size());
        registerWithMarketData();
        //QL_REQUIRE(optionTenors.size() == volatilities.size(),
        //           "mismatch between number of cap lengths "
        //           "and cap volatilities");
        //volatilities_[0] = volatilities[0];
        //std::copy(volatilities.begin(),volatilities.end(),volatilities_.begin()+1);
        for (Size i=0; i<volatilities_.size(); ++i)
            volatilities_[i] = volHandles_[i]->value();
        interpolate();
    }

    // fixed reference date, fixed market data
    inline CapVolatilityVector::CapVolatilityVector(
                                const Date& settlementDate,
                                const std::vector<Period>& optionTenors,
                                const std::vector<Volatility>& volatilities,
                                const DayCounter& dayCounter)
    : CapVolatilityStructure(settlementDate),
      dayCounter_(dayCounter), 
      optionTenors_(optionTenors),
      timeLengths_(optionTenors.size()), 
      volatilities_(volatilities.size()) {
        checkInputs(optionTenors.size(), volatilities.size());
        // fill dummy handles to allow generic handle-based
        // computations later on
        for (Size i=0; i<volatilities.size(); i++) {
            volHandles_[i] = Handle<Quote>(boost::shared_ptr<Quote>(
                new SimpleQuote(volatilities[i])));
        }
        registerWithMarketData();
        //QL_REQUIRE(optionTenors.size() == volatilities.size(),
        //           "mismatch between number of cap lengths "
        //           "and cap volatilities");
        volatilities_[0] = volatilities[0];
        std::copy(volatilities.begin(),volatilities.end(),volatilities_.begin()+1);
        interpolate();
    }

    // floating reference date, fixed market data
    inline CapVolatilityVector::CapVolatilityVector(
                                Natural settlementDays,
                                const Calendar& calendar,
                                const std::vector<Period>& optionTenors,
                                const std::vector<Volatility>& volatilities,
                                const DayCounter& dayCounter)
    : CapVolatilityStructure(settlementDays,calendar),
      dayCounter_(dayCounter), 
      optionTenors_(optionTenors),
      timeLengths_(optionTenors.size()), 
      volatilities_(volatilities.size()) {
        checkInputs(optionTenors.size(), volatilities.size());
        // fill dummy handles to allow generic handle-based
        // computations later on
        for (Size i=0; i<volatilities.size(); i++) {
            volHandles_[i] = Handle<Quote>(boost::shared_ptr<Quote>(
                new SimpleQuote(volatilities[i])));
        }
        registerWithMarketData();
        //QL_REQUIRE(optionTenors.size() == volatilities.size(),
        //           "mismatch between number of cap lengths "
        //           "and cap volatilities");
        volatilities_[0] = volatilities[0];
        std::copy(volatilities.begin(),volatilities.end(),volatilities_.begin()+1);
        interpolate();
    }

    inline void CapVolatilityVector::checkInputs(Size volRows,
                                                 Size volsColumns) const {
        QL_REQUIRE(optionTenors_.size()==volRows,
            "mismatch between number of cap lenght ("
            << optionTenors_.size() << ") and number of cap volatilities ("
            << volRows << ")");
        }

    inline void CapVolatilityVector::performCalculations() const {
        //CapVolatilityVector::performCalculations();
        for (Size i=0; i<volatilities_.size(); ++i)
            volatilities_[i] = volHandles_[i]->value();
    }

    inline void CapVolatilityVector::registerWithMarketData()
    {
        for (Size i=0; i<volHandles_.size(); ++i)
            registerWith(volHandles_[i]);
    }

    inline void CapVolatilityVector::interpolate() {
        //timeLengths_[0] = 0.0;
        for (Size i=0; i<optionTenors_.size(); i++) {
            Date endDate = referenceDate() + optionTenors_[i];
            timeLengths_[i] = timeFromReference(endDate);
        }
        interpolation_ =
            CubicSpline(
                timeLengths_.begin(),
                timeLengths_.end(),
                volatilities_.begin(),
                CubicSpline::SecondDerivative,
                0.0,
                CubicSpline::SecondDerivative,
                0.0,
                false);
            //LinearInterpolation(timeLengths_.begin(),
            //                    timeLengths_.end(),
            //                    volatilities_.begin());
        interpolation_.update();
        maxDate_ = referenceDate() + optionTenors_.back();
    }

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

    inline Volatility CapVolatilityVector::volatilityImpl(Time length, 
                                                          Rate) const {
        return interpolation_(length, true);
    }

}

#endif
