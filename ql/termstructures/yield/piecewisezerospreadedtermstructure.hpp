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

/*! \file piecewisezerospreadedtermstructure.hpp
    \brief Piecewise-zero-spreaded term structure
*/

#ifndef quantlib_piecewise_zero_spreaded_term_structure_hpp
#define quantlib_piecewise_zero_spreaded_term_structure_hpp

#include <ql/math/interpolations/linearinterpolation.hpp>
#include <ql/quote.hpp>
#include <ql/termstructures/yield/zeroyieldstructure.hpp>
#include <ql/termstructures/yield/interpolatedpiecewisespreadcurve.hpp>
#include <utility>
#include <vector>

namespace QuantLib {

  //! Yield curve with an added vector of spreads on the zero-yield rate
  /*! The zero-yield spread at any given date is interpolated
      between the input data.

      \note This term structure will remain linked to the original
            structure, i.e., any changes in the latter will be
            reflected in this structure as well.

      \ingroup yieldtermstructures
  */

  template <class Interpolator>
  class InterpolatedPiecewiseZeroSpreadedTermStructure : public ZeroYieldStructure,
                                                         public InterpolatedPiecewiseSpreadCurve<Interpolator> {
    public:
      InterpolatedPiecewiseZeroSpreadedTermStructure(Handle<YieldTermStructure>,
                                                     std::vector<Handle<Quote> > spreads,
                                                     const std::vector<Date>& dates,
                                                     Compounding comp = Continuous,
                                                     Frequency freq = NoFrequency,
                                                     DayCounter dc = DayCounter(),
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
      void update() override;

    private:
      void updateInterpolation();
      Real calcSpread(Time t) const;
      Handle<YieldTermStructure> originalCurve_;
      std::vector<Date> dates_;
      Compounding comp_;
      Frequency freq_;
      DayCounter dc_;
  };

    //! Piecewise zero-spreaded yield curve based on linear interpolation of zero rates
    /*! \ingroup yieldtermstructures */

    typedef InterpolatedPiecewiseZeroSpreadedTermStructure<Linear> PiecewiseZeroSpreadedTermStructure;


    // inline definitions

    template <class T>
    inline InterpolatedPiecewiseZeroSpreadedTermStructure<
        T>::InterpolatedPiecewiseZeroSpreadedTermStructure(Handle<YieldTermStructure> h,
                                                           std::vector<Handle<Quote> > spreads,
                                                           const std::vector<Date>& dates,
                                                           Compounding comp,
                                                           Frequency freq,
                                                           DayCounter dc,
                                                           const T& factory)
    : InterpolatedPiecewiseSpreadCurve<T>(spreads, dates, h->referenceDate(), h->dayCounter(), factory),
      originalCurve_(std::move(h)), dates_(dates), comp_(comp), freq_(freq),
      dc_(std::move(dc))  {
        registerWith(originalCurve_);
    }

    template <class T>
    inline DayCounter InterpolatedPiecewiseZeroSpreadedTermStructure<T>::dayCounter() const {
        return originalCurve_->dayCounter();
    }

    template <class T>
    inline Calendar InterpolatedPiecewiseZeroSpreadedTermStructure<T>::calendar() const {
        return originalCurve_->calendar();
    }

    template <class T>
    inline Natural InterpolatedPiecewiseZeroSpreadedTermStructure<T>::settlementDays() const {
        return originalCurve_->settlementDays();
    }

    template <class T>
    inline const Date&
    InterpolatedPiecewiseZeroSpreadedTermStructure<T>::referenceDate() const {
        return originalCurve_->referenceDate();
    }

    template <class T>
    inline Date InterpolatedPiecewiseZeroSpreadedTermStructure<T>::maxDate() const {
        return std::min(originalCurve_->maxDate(), dates_.back());
    }

    template <class T>
    inline Rate
    InterpolatedPiecewiseZeroSpreadedTermStructure<T>::zeroYieldImpl(Time t) const {
        Spread spread = InterpolatedPiecewiseSpreadCurve<T>::calcSpread(t);
        InterestRate zeroRate = originalCurve_->zeroRate(t, comp_, freq_, true);
        InterestRate spreadedRate(zeroRate + spread,
                                  zeroRate.dayCounter(),
                                  zeroRate.compounding(),
                                  zeroRate.frequency());
        return spreadedRate.equivalentRate(Continuous, NoFrequency, t);
    }

    template <class T>
    inline void InterpolatedPiecewiseZeroSpreadedTermStructure<T>::update() {
        if (!originalCurve_.empty()) {
            InterpolatedPiecewiseSpreadCurve<T>::updateInterpolation();
            ZeroYieldStructure::update();
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

