/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file swaptionconstantvol.hpp
    \brief Constant swaption volatility
*/

#ifndef quantlib_swaption_constant_volatility_hpp
#define quantlib_swaption_constant_volatility_hpp

#include <ql/swaptionvolstructure.hpp>
#include <ql/Volatilities/smilesection.hpp>
#include <ql/quote.hpp>

namespace QuantLib {

    //! Constant swaption volatility, no time-strike dependence
    class SwaptionConstantVolatility : public SwaptionVolatilityStructure {
      public:
        SwaptionConstantVolatility(const Date& referenceDate,
                                   Volatility volatility,
                                   const DayCounter& dayCounter);
        SwaptionConstantVolatility(const Date& referenceDate,
                                   const Handle<Quote>& volatility,
                                   const DayCounter& dayCounter);
        SwaptionConstantVolatility(Integer settlementDays,
                                   const Calendar&,
                                   Volatility volatility,
                                   const DayCounter& dayCounter);
        SwaptionConstantVolatility(Integer settlementDays,
                                   const Calendar&,
                                   const Handle<Quote>& volatility,
                                   const DayCounter& dayCounter);
        //! \name TermStructure interface
        //@{
        DayCounter dayCounter() const { return dayCounter_; }
        //@}
        //! \name SwaptionConstantVolatility interface
        //@{
        Date maxStartDate() const;
        Time maxStartTime() const;
        Period maxLength() const;
        Time maxTimeLength() const;
        Real minStrike() const;
        Real maxStrike() const;

        //! return trivial smile section
        boost::shared_ptr<SmileSection> smileSection(
                                                 const Date& start,
                                                 const Period& length) const;

      protected:
        Volatility volatilityImpl(Time, Time, Rate) const;
        boost::shared_ptr<SmileSection> smileSection(Time start,
                                                     Time length) const;
        Volatility volatilityImpl(const Date&, const Period&, Rate) const;
        //@}
      private:
        Handle<Quote> volatility_;
        DayCounter dayCounter_;
    };


    // inline definitions

    inline SwaptionConstantVolatility::SwaptionConstantVolatility(
                                              const Date& referenceDate,
                                              Volatility volatility,
                                              const DayCounter& dayCounter)
    : SwaptionVolatilityStructure(referenceDate), dayCounter_(dayCounter) {
        volatility_.linkTo(
                       boost::shared_ptr<Quote>(new SimpleQuote(volatility)));
        registerWith(volatility_);
    }

    inline SwaptionConstantVolatility::SwaptionConstantVolatility(
                                              const Date& referenceDate,
                                              const Handle<Quote>& volatility,
                                              const DayCounter& dayCounter)
    : SwaptionVolatilityStructure(referenceDate), volatility_(volatility),
      dayCounter_(dayCounter) {
        registerWith(volatility_);
    }

    inline SwaptionConstantVolatility::SwaptionConstantVolatility(
                                              Integer settlementDays,
                                              const Calendar& calendar,
                                              Volatility volatility,
                                              const DayCounter& dayCounter)
    : SwaptionVolatilityStructure(settlementDays,calendar),
      dayCounter_(dayCounter) {
        volatility_.linkTo(
                       boost::shared_ptr<Quote>(new SimpleQuote(volatility)));
        registerWith(volatility_);
    }

    inline SwaptionConstantVolatility::SwaptionConstantVolatility(
                                              Integer settlementDays,
                                              const Calendar& calendar,
                                              const Handle<Quote>& volatility,
                                              const DayCounter& dayCounter)
    : SwaptionVolatilityStructure(settlementDays,calendar),
      volatility_(volatility), dayCounter_(dayCounter) {
        registerWith(volatility_);
    }

    inline Date SwaptionConstantVolatility::maxStartDate() const {
        return Date::maxDate();
    }

    inline Time SwaptionConstantVolatility::maxStartTime() const {
        return QL_MAX_REAL;
    }

    inline Period SwaptionConstantVolatility::maxLength() const {
        return 100*Years;
    }

    inline Time SwaptionConstantVolatility::maxTimeLength() const {
        return QL_MAX_REAL;
    }

    inline Real SwaptionConstantVolatility::minStrike() const {
        return QL_MIN_REAL;
    }

    inline Real SwaptionConstantVolatility::maxStrike() const {
        return QL_MAX_REAL;
    }

    inline Volatility SwaptionConstantVolatility::volatilityImpl(
                                                     Time, Time, Rate) const {
        return volatility_->value();
    }

    inline Volatility SwaptionConstantVolatility::volatilityImpl(
                                     const Date&, const Period&, Rate) const {
        return volatility_->value();
    }

    inline boost::shared_ptr<SmileSection>
    SwaptionConstantVolatility::smileSection(Time start, Time) const {
        const Volatility atmVol = volatility_->value();

        std::vector<Real> strikes, volatilities(2, atmVol);
        strikes.push_back(0.0);
        strikes.push_back(1.0);

        return boost::shared_ptr<SmileSection>(
                              new SmileSection(start, strikes, volatilities));
    }

    inline boost::shared_ptr<SmileSection>
    SwaptionConstantVolatility::smileSection(const Date& start,
                                             const Period&) const {
        const Volatility atmVol = volatility_->value();

        std::vector<Real> strikes, volatilities(2, atmVol);
        strikes.push_back(0.0);
        strikes.push_back(1.0);

        return boost::shared_ptr<SmileSection>(
            new SmileSection(timeFromReference(start), strikes, volatilities));
    }

}


#endif
