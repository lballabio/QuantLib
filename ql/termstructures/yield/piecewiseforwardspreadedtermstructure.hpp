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
 <http://quantlib.org/license.shtml>.

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
#include <ql/termstructures/yield/interpolatedpiecewisespreadcurve.hpp>
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
  class InterpolatedPiecewiseForwardSpreadedTermStructure : public ForwardRateStructure,
                                                            public InterpolatedPiecewiseSpreadCurve<Interpolator> {
    public:
      InterpolatedPiecewiseForwardSpreadedTermStructure(Handle<YieldTermStructure>,
                                                     std::vector<Handle<Quote> > spreads,
                                                     const std::vector<Date>& dates,
                                                     const Interpolator& factory = Interpolator());
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
      Handle<YieldTermStructure> originalCurve_;
      std::vector<Date> dates_;
      DayCounter dc_;
  };

    // inline definitions

    template <class T>
    inline InterpolatedPiecewiseForwardSpreadedTermStructure<
        T>::InterpolatedPiecewiseForwardSpreadedTermStructure(Handle<YieldTermStructure> h,
                                                           std::vector<Handle<Quote> > spreads,
                                                           const std::vector<Date>& dates,
                                                           const T& factory)
    : InterpolatedPiecewiseSpreadCurve<T>(spreads, dates, h->referenceDate(), h->dayCounter(), factory),
     originalCurve_(std::move(h)), dates_(dates) {
        registerWith(originalCurve_);
    }

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
        Spread spreadPrimitive = InterpolatedPiecewiseSpreadCurve<T>::calcSpreadPrimitive(t);
        InterestRate zeroRate = originalCurve_->zeroRate(t, Continuous, NoFrequency, true);
        return zeroRate + spreadPrimitive;
    }

    template <class T>
    inline Rate
    InterpolatedPiecewiseForwardSpreadedTermStructure<T>::forwardImpl(Time t) const {
        Spread spread = InterpolatedPiecewiseSpreadCurve<T>::calcSpread(t);
        Rate forwardRate = originalCurve_->forwardRate(t, t, Continuous, NoFrequency, true);
        return forwardRate + spread;
    }

    template <class T>
    inline void InterpolatedPiecewiseForwardSpreadedTermStructure<T>::update() {
        if (!originalCurve_.empty()) {
            InterpolatedPiecewiseSpreadCurve<T>::updateInterpolation();
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

}


#endif
