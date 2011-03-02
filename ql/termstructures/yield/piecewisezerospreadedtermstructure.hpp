/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Roland Lichters
 Copyright (C) 2006, 2008 StatPro Italia srl
 Copyright (C) 2010 Robert Philipp

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

/*! \file piecewisezerospreadedtermstructure.hpp
    \brief Piecewise-zero-spreaded term structure
*/

#ifndef quantlib_piecewise_zero_spreaded_term_structure_hpp
#define quantlib_piecewise_zero_spreaded_term_structure_hpp

#include <ql/termstructures/yield/zeroyieldstructure.hpp>
#include <ql/quote.hpp>
#include <vector>

namespace QuantLib {

  //! Term structure with an added vector of spreads on the zero-yield rate
  /*! The zero-yield spread at any given date is linearly interpolated
      between the input data.

      \note This term structure will remain linked to the original
            structure, i.e., any changes in the latter will be
            reflected in this structure as well.

      \ingroup yieldtermstructures
  */
  class PiecewiseZeroSpreadedTermStructure : public ZeroYieldStructure {
    public:
      PiecewiseZeroSpreadedTermStructure(
                                   const Handle<YieldTermStructure>&,
                                   const std::vector<Handle<Quote> >& spreads,
                                   const std::vector<Date>& dates,
                                   Compounding comp = Continuous,
                                   Frequency freq = NoFrequency,
                                   const DayCounter& dc = DayCounter());
      //! \name YieldTermStructure interface
      //@{
      DayCounter dayCounter() const;
      Natural settlementDays() const;
      Calendar calendar() const;
      const Date& referenceDate() const;
      Date maxDate() const;
      //@}
    protected:
      //! returns the spreaded zero yield rate
      Rate zeroYieldImpl(Time) const;
      void update();
    private:
      void updateTimes();
      Spread calcSpread(Time t) const;
      Handle<YieldTermStructure> originalCurve_;
      std::vector<Handle<Quote> > spreads_;
      std::vector<Date> dates_;
      std::vector<Time> times_;
      Compounding comp_;
      Frequency freq_;
      DayCounter dc_;
  };


    // inline definitions

    inline
    PiecewiseZeroSpreadedTermStructure::PiecewiseZeroSpreadedTermStructure(
                                   const Handle<YieldTermStructure>& h,
                                   const std::vector<Handle<Quote> >& spreads,
                                   const std::vector<Date>& dates,
                                   Compounding comp,
                                   Frequency freq,
                                   const DayCounter& dc)
    : originalCurve_(h), spreads_(spreads), dates_(dates),
      times_(dates_.size()), comp_(comp), freq_(freq), dc_(dc) {
        QL_REQUIRE(!spreads_.empty(), "no spreads given");
        QL_REQUIRE(spreads_.size() == dates_.size(),
                   "spread and date vector have different sizes");
        registerWith(originalCurve_);
        for (Size i = 0; i < spreads_.size(); i++)
            registerWith(spreads_[i]);
        updateTimes();
    }

    inline DayCounter PiecewiseZeroSpreadedTermStructure::dayCounter() const {
        return originalCurve_->dayCounter();
    }

    inline Calendar PiecewiseZeroSpreadedTermStructure::calendar() const {
        return originalCurve_->calendar();
    }

    inline Natural PiecewiseZeroSpreadedTermStructure::settlementDays() const {
        return originalCurve_->settlementDays();
    }

    inline const Date&
    PiecewiseZeroSpreadedTermStructure::referenceDate() const {
        return originalCurve_->referenceDate();
    }

    inline Date PiecewiseZeroSpreadedTermStructure::maxDate() const {
        return std::min(originalCurve_->maxDate(), dates_.back());
    }

    inline Rate
    PiecewiseZeroSpreadedTermStructure::zeroYieldImpl(Time t) const {
        Spread spread = calcSpread(t);
        InterestRate zeroRate = originalCurve_->zeroRate(t, comp_, freq_, true);
        InterestRate spreadedRate(zeroRate + spread,
                                  zeroRate.dayCounter(),
                                  zeroRate.compounding(),
                                  zeroRate.frequency());
        return spreadedRate.equivalentRate(Continuous, NoFrequency, t);
    }

    inline Spread
    PiecewiseZeroSpreadedTermStructure::calcSpread(Time t) const {
        if (t <= times_.front()) {
            return spreads_.front()->value();
        } else if (t >= times_.back()) {
            return spreads_.back()->value();
        } else {
            Size i;
            for (i = 0; i < times_.size(); i++)
                if (times_[i] > t) break;
            Time dt = times_[i] - times_[i-1];
            return spreads_[i]->value() * (t - times_[i-1]) / dt
                + spreads_[i-1]->value() * (times_[i] - t) / dt;
        }
    }

    inline void PiecewiseZeroSpreadedTermStructure::update() {
        updateTimes();
        ZeroYieldStructure::update();
    }

    inline void PiecewiseZeroSpreadedTermStructure::updateTimes() {
        for (Size i = 0; i < dates_.size(); i++)
            times_[i] = timeFromReference(dates_[i]);
    }

}


#endif

