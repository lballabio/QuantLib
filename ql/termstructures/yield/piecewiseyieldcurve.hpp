/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005, 2006, 2007, 2008 StatPro Italia srl
 Copyright (C) 2007, 2008, 2009 Ferdinando Ametrano
 Copyright (C) 2007 Chris Kenyon

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

/*! \file piecewiseyieldcurve.hpp
    \brief piecewise-interpolated term structure
*/

#ifndef quantlib_piecewise_yield_curve_hpp
#define quantlib_piecewise_yield_curve_hpp

#include <ql/patterns/lazyobject.hpp>
#include <ql/termstructures/iterativebootstrap.hpp>
#include <ql/termstructures/yield/bootstraptraits.hpp>
#include <utility>

namespace QuantLib {

    //! Piecewise yield term structure
    /*! This term structure is bootstrapped on a number of interest
        rate instruments which are passed as a vector of pointers to
        RateHelper instances. Their maturities mark the boundaries of
        the interpolated segments.

        Each segment is determined sequentially starting from the
        earliest period to the latest and is chosen so that the
        instrument whose maturity marks the end of such segment is
        correctly repriced on the curve.

        \warning The bootstrapping algorithm will raise an exception if
                 any two instruments have the same maturity date.

        \ingroup yieldtermstructures

        \test
        - the correctness of the returned values is tested by
          checking them against the original inputs.
        - the observability of the term structure is tested.
    */
    template <class Traits, class Interpolator,
              template <class> class Bootstrap = IterativeBootstrap>
    class PiecewiseYieldCurve
        : public Traits::template curve<Interpolator>::type,
          public LazyObject {
      private:
        typedef typename Traits::template curve<Interpolator>::type base_curve;
        typedef PiecewiseYieldCurve<Traits,Interpolator,Bootstrap> this_curve;
      public:
        typedef Traits traits_type;
        typedef Interpolator interpolator_type;
        typedef Bootstrap<this_curve> bootstrap_type;

        //! \name Constructors
        //@{
        PiecewiseYieldCurve(
            const Date& referenceDate,
            std::vector<ext::shared_ptr<typename Traits::helper> > instruments,
            const DayCounter& dayCounter,
            const std::vector<Handle<Quote> >& jumps = {},
            const std::vector<Date>& jumpDates = {},
            const Interpolator& i = {},
            bootstrap_type bootstrap = {})
        : base_curve(referenceDate, dayCounter, jumps, jumpDates, i),
          instruments_(std::move(instruments)), accuracy_(1.0e-12),
          bootstrap_(std::move(bootstrap)) {
            bootstrap_.setup(this);
        }

        PiecewiseYieldCurve(const Date& referenceDate,
                            std::vector<ext::shared_ptr<typename Traits::helper> > instruments,
                            const DayCounter& dayCounter,
                            const Interpolator& i,
                            bootstrap_type bootstrap = bootstrap_type())
        : base_curve(
              referenceDate, dayCounter, {}, {}, i),
          instruments_(std::move(instruments)), accuracy_(1.0e-12),
          bootstrap_(std::move(bootstrap)) {
            bootstrap_.setup(this);
        }

        PiecewiseYieldCurve(const Date& referenceDate,
                            std::vector<ext::shared_ptr<typename Traits::helper> > instruments,
                            const DayCounter& dayCounter,
                            bootstrap_type bootstrap)
        : base_curve(referenceDate,
                     dayCounter),
          instruments_(std::move(instruments)), accuracy_(1.0e-12),
          bootstrap_(std::move(bootstrap)) {
            bootstrap_.setup(this);
        }

        PiecewiseYieldCurve(
            Natural settlementDays,
            const Calendar& calendar,
            std::vector<ext::shared_ptr<typename Traits::helper> > instruments,
            const DayCounter& dayCounter,
            const std::vector<Handle<Quote> >& jumps = {},
            const std::vector<Date>& jumpDates = {},
            const Interpolator& i = {},
            bootstrap_type bootstrap = {})
        : base_curve(settlementDays, calendar, dayCounter, jumps, jumpDates, i),
          instruments_(std::move(instruments)), accuracy_(1.0e-12),
          bootstrap_(std::move(bootstrap)) {
            bootstrap_.setup(this);
        }

        PiecewiseYieldCurve(Natural settlementDays,
                            const Calendar& calendar,
                            std::vector<ext::shared_ptr<typename Traits::helper> > instruments,
                            const DayCounter& dayCounter,
                            const Interpolator& i,
                            bootstrap_type bootstrap = bootstrap_type())
        : base_curve(settlementDays,
                     calendar,
                     dayCounter,
                     {},
                     {},
                     i),
          instruments_(std::move(instruments)), accuracy_(1.0e-12),
          bootstrap_(std::move(bootstrap)) {
            bootstrap_.setup(this);
        }

        PiecewiseYieldCurve(
               Natural settlementDays,
               const Calendar& calendar,
               const std::vector<ext::shared_ptr<typename Traits::helper> >&
                                                                  instruments,
               const DayCounter& dayCounter,
               const bootstrap_type& bootstrap)
        : base_curve(settlementDays, calendar, dayCounter),
          instruments_(instruments),
          accuracy_(1.0e-12), bootstrap_(bootstrap) {
            bootstrap_.setup(this);
        }
        //@}
        //! \name TermStructure interface
        //@{
        Date maxDate() const override;
        //@}
        //! \name base_curve interface
        //@{
        const std::vector<Time>& times() const;
        const std::vector<Date>& dates() const;
        const std::vector<Real>& data() const;
        std::vector<std::pair<Date, Real> > nodes() const;
        //@}
        //! \name Observer interface
        //@{
        void update() override;
        //@}
      private:
        //! \name LazyObject interface
        //@{
        void performCalculations() const override;
        //@}
        // methods
        DiscountFactor discountImpl(Time) const override;
        // data members
        std::vector<ext::shared_ptr<typename Traits::helper> > instruments_;
        Real accuracy_;

        // bootstrapper classes are declared as friend to manipulate
        // the curve data. They might be passed the data instead, but
        // it would increase the complexity---which is high enough
        // already.
        friend class Bootstrap<this_curve>;
        Bootstrap<this_curve> bootstrap_;
    };


    // inline definitions

    template <class C, class I, template <class> class B>
    inline Date PiecewiseYieldCurve<C,I,B>::maxDate() const {
        calculate();
        return base_curve::maxDate();
    }

    template <class C, class I, template <class> class B>
    inline const std::vector<Time>& PiecewiseYieldCurve<C,I,B>::times() const {
        calculate();
        return base_curve::times();
    }

    template <class C, class I, template <class> class B>
    inline const std::vector<Date>& PiecewiseYieldCurve<C,I,B>::dates() const {
        calculate();
        return base_curve::dates();
    }

    template <class C, class I, template <class> class B>
    inline const std::vector<Real>& PiecewiseYieldCurve<C,I,B>::data() const {
        calculate();
        return base_curve::data();
    }

    template <class C, class I, template <class> class B>
    inline std::vector<std::pair<Date, Real> >
    PiecewiseYieldCurve<C,I,B>::nodes() const {
        calculate();
        return base_curve::nodes();
    }

    template <class C, class I, template <class> class B>
    inline void PiecewiseYieldCurve<C,I,B>::update() {

        // it dispatches notifications only if (!calculated_ && !frozen_)
        LazyObject::update();

        // do not use base_curve::update() as it would always notify observers

        // TermStructure::update() update part
        if (this->moving_)
            this->updated_ = false;

    }

    template <class C, class I, template <class> class B>
    inline
    DiscountFactor PiecewiseYieldCurve<C,I,B>::discountImpl(Time t) const {
        calculate();
        return base_curve::discountImpl(t);
    }

    template <class C, class I, template <class> class B>
    inline void PiecewiseYieldCurve<C,I,B>::performCalculations() const {
        // just delegate to the bootstrapper
        bootstrap_.calculate();
    }

}

#endif
