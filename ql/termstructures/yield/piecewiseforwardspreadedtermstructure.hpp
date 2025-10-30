/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Roland Lichters
 Copyright (C) 2006, 2008, 2014 StatPro Italia srl
 Copyright (C) 2010 Robert Philipp

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file piecewiseforwardspreadedtermstructure.hpp
    \brief Piecewise-forward-spreaded term structure
*/

#ifndef quantlib_piecewise_forward_spreaded_term_structure_hpp
#define quantlib_piecewise_forward_spreaded_term_structure_hpp

#include <ql/math/interpolations/linearinterpolation.hpp>
#include <ql/quote.hpp>
#include <ql/termstructures/yield/forwardstructure.hpp>
#include <utility>
#include <vector>

namespace QuantLib {

  //! Term structure with an added vector of spreads on the instantaneous forward rate
  /*! The forward rate spread at any given date is interpolated
      between the input data.

      \note This term structure will remain linked to the original
            structure, i.e., any changes in the latter will be
            reflected in this structure as well.

      \ingroup yieldtermstructures
  */

  template <class Interpolator>
  class InterpolatedPiecewiseForwardSpreadedTermStructure : public ForwardRateStructure {
    public:
      InterpolatedPiecewiseForwardSpreadedTermStructure(Handle<YieldTermStructure>,
                                                     std::vector<Handle<Quote>> spreads,
                                                     std::vector<Date> dates,
                                                     Interpolator factory = Interpolator());

      [[deprecated("Use the constructor without DayCounter")]]
      InterpolatedPiecewiseForwardSpreadedTermStructure(Handle<YieldTermStructure>,
                                                     std::vector<Handle<Quote>> spreads,
                                                     std::vector<Date> dates,
                                                     DayCounter dc,
                                                     Interpolator factory = Interpolator());
      //! \name YieldTermStructure interface
      //@{
      DayCounter dayCounter() const override;
      Natural settlementDays() const override;
      Calendar calendar() const override;
      const Date& referenceDate() const override;
      Date maxDate() const override;
      //@}
    protected:
      //! returns the spreaded zero yield rate
      Rate zeroYieldImpl(Time) const override;
      Rate forwardImpl(Time) const override;
      void update() override;

    private:
      void updateInterpolation();
      Real calcSpread(Time t) const;
      Real calcSpreadPrimitive(Time t) const;
      Handle<YieldTermStructure> originalCurve_;
      std::vector<Handle<Quote> > spreads_;
      std::vector<Date> dates_;
      std::vector<Time> times_;
      std::vector<Spread> spreadValues_;
      Compounding comp_;
      Frequency freq_;
      Interpolator factory_;
      Interpolation interpolator_;
  };

    // inline definitions

    #ifndef __DOXYGEN__

    template <class T>
    inline InterpolatedPiecewiseForwardSpreadedTermStructure<
        T>::InterpolatedPiecewiseForwardSpreadedTermStructure(Handle<YieldTermStructure> h,
                                                           std::vector<Handle<Quote>> spreads,
                                                           std::vector<Date> dates,
                                                           T factory)
    : originalCurve_(std::move(h)), spreads_(std::move(spreads)), dates_(std::move(dates)),
    times_(dates_.size()), spreadValues_(dates_.size()), factory_(std::move(factory)) {
        QL_REQUIRE(!spreads_.empty(), "no spreads given");
        QL_REQUIRE(spreads_.size() == dates_.size(),
                   "spread and date vector have different sizes");
        registerWith(originalCurve_);
        for (auto& spread : spreads_)
            registerWith(spread);
        if (!originalCurve_.empty())
            updateInterpolation();
    }

    template <class T>
    inline InterpolatedPiecewiseForwardSpreadedTermStructure<
        T>::InterpolatedPiecewiseForwardSpreadedTermStructure(Handle<YieldTermStructure> h,
                                                           std::vector<Handle<Quote>> spreads,
                                                           std::vector<Date> dates,
                                                           DayCounter dc,
                                                           T factory)
    : InterpolatedPiecewiseForwardSpreadedTermStructure(
        std::move(h), std::move(spreads), std::move(dates), std::move(factory)
    ) {}

    #endif

    template <class T>
    inline DayCounter InterpolatedPiecewiseForwardSpreadedTermStructure<T>::dayCounter() const {
        return originalCurve_->dayCounter();
    }

    template <class T>
    inline Calendar InterpolatedPiecewiseForwardSpreadedTermStructure<T>::calendar() const {
        return originalCurve_->calendar();
    }

    template <class T>
    inline Natural InterpolatedPiecewiseForwardSpreadedTermStructure<T>::settlementDays() const {
        return originalCurve_->settlementDays();
    }

    template <class T>
    inline const Date&
    InterpolatedPiecewiseForwardSpreadedTermStructure<T>::referenceDate() const {
        return originalCurve_->referenceDate();
    }

    template <class T>
    inline Date InterpolatedPiecewiseForwardSpreadedTermStructure<T>::maxDate() const {
        return std::min(originalCurve_->maxDate(), dates_.back());
    }

    template <class T>
    inline Rate
    InterpolatedPiecewiseForwardSpreadedTermStructure<T>::zeroYieldImpl(Time t) const {
        Spread spreadPrimitive = calcSpreadPrimitive(t);
        InterestRate zeroRate = originalCurve_->zeroRate(t, Continuous, NoFrequency, true);
        return zeroRate + spreadPrimitive;
    }

    template <class T>
    inline Rate
    InterpolatedPiecewiseForwardSpreadedTermStructure<T>::forwardImpl(Time t) const {
        Spread spread = calcSpread(t);
        Rate forwardRate = originalCurve_->forwardRate(t, t, Continuous, NoFrequency, true);
        return forwardRate + spread;
    }

    template <class T>
    inline Spread
    InterpolatedPiecewiseForwardSpreadedTermStructure<T>::calcSpread(Time t) const {
        if (t <= times_.front()) {
            return spreads_.front()->value();
        } else if (t >= times_.back()) {
            return spreads_.back()->value();
        } else {
            return interpolator_(t, true);
        }
    }

    template <class T>
    inline Spread
    InterpolatedPiecewiseForwardSpreadedTermStructure<T>::calcSpreadPrimitive(Time t) const {
        if (t == 0.0)
            return calcSpread(0.0);

        Real integral;
        if (t <= this->times_.back()) {
            integral = this->interpolator_.primitive(t, true);
        } else {
            integral = this->interpolator_.primitive(this->times_.back(), true)
                     + this->spreads_.back()->value() * (t - this->times_.back());
        }
        return integral/t;
    }

    template <class T>
    inline void InterpolatedPiecewiseForwardSpreadedTermStructure<T>::update() {
        if (!originalCurve_.empty()) {
            updateInterpolation();
            YieldTermStructure::update();
        } else {
            /* The implementation inherited from YieldTermStructure
               asks for our reference date, which we don't have since
               the original curve is still not set. Therefore, we skip
               over that and just call the base-class behavior. */
            // NOLINTNEXTLINE(bugprone-parent-virtual-call)
            TermStructure::update();
        }
    }

    template <class T>
    inline void InterpolatedPiecewiseForwardSpreadedTermStructure<T>::updateInterpolation() {
        for (Size i = 0; i < dates_.size(); i++) {
            times_[i] = timeFromReference(dates_[i]);
            spreadValues_[i] = spreads_[i]->value();
        }
        interpolator_ = factory_.interpolate(times_.begin(),
                                             times_.end(),
                                             spreadValues_.begin());
    }

}


#endif
