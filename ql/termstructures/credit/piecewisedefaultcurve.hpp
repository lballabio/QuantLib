/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008, 2016 Jose Aparicio
 Copyright (C) 2008 Chris Kenyon
 Copyright (C) 2008 Roland Lichters
 Copyright (C) 2008 StatPro Italia srl

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

/*! \file piecewisedefaultcurve.hpp
    \brief piecewise-interpolated default-probability structure
*/

#ifndef quantlib_piecewise_default_curve_hpp
#define quantlib_piecewise_default_curve_hpp

#include <ql/models/shortrate/onefactormodel.hpp>
#include <ql/patterns/lazyobject.hpp>
#include <ql/quote.hpp>
#include <ql/termstructures/credit/probabilitytraits.hpp>
#include <ql/termstructures/iterativebootstrap.hpp>
#include <utility>

namespace QuantLib {

    //! Piecewise default-probability term structure
    /*! This term structure is bootstrapped on a number of credit
        instruments which are passed as a vector of handles to
        DefaultProbabilityHelper instances. Their maturities mark the
        boundaries of the interpolated segments.

        Each segment is determined sequentially starting from the
        earliest period to the latest and is chosen so that the
        instrument whose maturity marks the end of such segment is
        correctly repriced on the curve.

        \warning The bootstrapping algorithm will raise an exception if
                 any two instruments have the same maturity date.
    */
    template <class Traits, class Interpolator,
              template <class> class Bootstrap = IterativeBootstrap>
    class PiecewiseDefaultCurve
        : public Traits::template curve<Interpolator>::type,
          public LazyObject {
      private:
        typedef typename Traits::template curve<Interpolator>::type base_curve;
        typedef PiecewiseDefaultCurve<Traits,Interpolator,Bootstrap> this_curve;
      public:
        typedef Traits traits_type;
        typedef Interpolator interpolator_type;
        typedef Bootstrap<this_curve> bootstrap_type;

        //! \name Constructors
        //@{
        PiecewiseDefaultCurve(
            const Date& referenceDate,
            std::vector<ext::shared_ptr<typename Traits::helper> > instruments,
            const DayCounter& dayCounter,
            const std::vector<Handle<Quote> >& jumps = std::vector<Handle<Quote> >(),
            const std::vector<Date>& jumpDates = std::vector<Date>(),
            const Interpolator& i = Interpolator(),
            bootstrap_type bootstrap = bootstrap_type())
        : base_curve(referenceDate, dayCounter, jumps, jumpDates, i),
          instruments_(std::move(instruments)), accuracy_(1.0e-12),
          bootstrap_(std::move(bootstrap)) {
            bootstrap_.setup(this);
        }

        PiecewiseDefaultCurve(
               const Date& referenceDate,
               const std::vector<ext::shared_ptr<typename Traits::helper> >&
                                                                  instruments,
               const DayCounter& dayCounter,
               const Interpolator& i,
               const bootstrap_type& bootstrap = bootstrap_type())
        : base_curve(referenceDate, dayCounter,
                     std::vector<Handle<Quote> >(), std::vector<Date>(), i),
          instruments_(instruments), accuracy_(1.0e-12), bootstrap_(bootstrap) {
            bootstrap_.setup(this);
        }

        PiecewiseDefaultCurve(const Date& referenceDate,
                              std::vector<ext::shared_ptr<typename Traits::helper> > instruments,
                              const DayCounter& dayCounter,
                              bootstrap_type bootstrap)
        : base_curve(referenceDate,
                     dayCounter,
                     std::vector<Handle<Quote> >(),
                     std::vector<Date>(),
                     Interpolator()),
          instruments_(std::move(instruments)), accuracy_(1.0e-12),
          bootstrap_(std::move(bootstrap)) {
            bootstrap_.setup(this);
        }

        PiecewiseDefaultCurve(
            Natural settlementDays,
            const Calendar& calendar,
            std::vector<ext::shared_ptr<typename Traits::helper> > instruments,
            const DayCounter& dayCounter,
            const std::vector<Handle<Quote> >& jumps = std::vector<Handle<Quote> >(),
            const std::vector<Date>& jumpDates = std::vector<Date>(),
            const Interpolator& i = Interpolator(),
            bootstrap_type bootstrap = bootstrap_type())
        : base_curve(settlementDays, calendar, dayCounter, jumps, jumpDates, i),
          instruments_(std::move(instruments)), accuracy_(1.0e-12),
          bootstrap_(std::move(bootstrap)) {
            bootstrap_.setup(this);
        }

        PiecewiseDefaultCurve(
               Natural settlementDays,
               const Calendar& calendar,
               const std::vector<ext::shared_ptr<typename Traits::helper> >&
                                                                  instruments,
               const DayCounter& dayCounter,
               const Interpolator& i,
               const bootstrap_type& bootstrap = bootstrap_type())
        : base_curve(settlementDays, calendar, dayCounter,
                     std::vector<Handle<Quote> >(), std::vector<Date>(), i),
          instruments_(instruments), accuracy_(1.0e-12), bootstrap_(bootstrap) {
            bootstrap_.setup(this);
        }

        PiecewiseDefaultCurve(
               Natural settlementDays,
               const Calendar& calendar,
               const std::vector<ext::shared_ptr<typename Traits::helper> >&
                                                                  instruments,
               const DayCounter& dayCounter,
               const bootstrap_type& bootstrap)
        : base_curve(settlementDays, calendar, dayCounter,
                     std::vector<Handle<Quote> >(), std::vector<Date>(),
                     Interpolator()),
          instruments_(instruments), accuracy_(1.0e-12), bootstrap_(bootstrap) {
            bootstrap_.setup(this);
        }

        /* AffineHazardRate Traits constructor case. Other constructors of
        base_curve would fail and this would fail for other cases of Traits.
        This is a case of substitution failure, it might be preferred
        to specialization of the class.
        The way the methods are used in the bootstrapping means the target
        term structure is the deterministic TS to be added to the model
        passed in order to reproduce instrument market prices.

        \todo Implement the remaining signatures
        */
        PiecewiseDefaultCurve(
            const Date& referenceDate,
            const std::vector<ext::shared_ptr<typename Traits::helper> >& instruments,
            const DayCounter& dayCounter,
            const ext::shared_ptr<OneFactorAffineModel>& model,
            const Interpolator& i = Interpolator(),
            const bootstrap_type& bootstrap = bootstrap_type())
        : base_curve(referenceDate,
                     dayCounter,
                     model,
                     std::vector<Handle<Quote> >(),
                     std::vector<Date>(),
                     i),
          instruments_(instruments), accuracy_(1.0e-12), bootstrap_(bootstrap) {
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
        Probability survivalProbabilityImpl(Time) const override;
        Real defaultDensityImpl(Time) const override;
        #if defined(__clang__)
        #pragma clang diagnostic push
        #pragma clang diagnostic ignored "-Winconsistent-missing-override"
        #if (defined(__APPLE__) && __clang_major__ > 12) || (!defined(__APPLE__) && __clang_major__ > 10)
        #pragma clang diagnostic ignored "-Wsuggest-override"
        #endif
        #endif
        #if defined(__GNUC__) && (((__GNUC__ == 5) && (__GNUC_MINOR__ >= 1)) || (__GNUC__ > 5))
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wsuggest-override"
        #endif
        Real hazardRateImpl(Time) const; // NOLINT(modernize-use-override, cppcoreguidelines-explicit-virtual-functions)
                                         // (sometimes this method is not virtual,
                                         //  depending on the base class)
        #if defined(__clang__)
        #pragma clang diagnostic pop
        #endif
        #if defined(__GNUC__) && (((__GNUC__ == 5) && (__GNUC_MINOR__ >= 1)) || (__GNUC__ > 5))
        #pragma GCC diagnostic pop
        #endif
        // data members
        std::vector<ext::shared_ptr<typename Traits::helper> > instruments_;
        Real accuracy_;

        // bootstrapper classes are declared as friend to manipulate
        // the curve data. They might be passed the data instead, but
        // it would increase the complexity---which is high enough
        // already.
        friend class Bootstrap<this_curve>;
        friend class BootstrapError<this_curve>;
        Bootstrap<this_curve> bootstrap_;
    };


    // inline definitions

    template <class C, class I, template <class> class B>
    inline Date PiecewiseDefaultCurve<C,I,B>::maxDate() const {
        calculate();
        return base_curve::maxDate();
    }

    template <class C, class I, template <class> class B>
    inline const std::vector<Time>&
    PiecewiseDefaultCurve<C,I,B>::times() const {
        calculate();
        return base_curve::times();
    }

    template <class C, class I, template <class> class B>
    inline const std::vector<Date>&
    PiecewiseDefaultCurve<C,I,B>::dates() const {
        calculate();
        return base_curve::dates();
    }

    template <class C, class I, template <class> class B>
    inline const std::vector<Real>&
    PiecewiseDefaultCurve<C,I,B>::data() const {
        calculate();
        return this->data_;
    }

    template <class C, class I, template <class> class B>
    inline std::vector<std::pair<Date, Real> >
    PiecewiseDefaultCurve<C,I,B>::nodes() const {
        calculate();
        return base_curve::nodes();
    }

    template <class C, class I, template <class> class B>
    inline void PiecewiseDefaultCurve<C,I,B>::update() {
        // it dispatches notifications only if (!calculated_ && !frozen_)
        LazyObject::update();

        // do not use base_curve::update() as it would always notify observers

        // TermStructure::update() update part
        if (this->moving_)
            this->updated_ = false;
    }

    template <class C, class I, template <class> class B>
    inline Probability
    PiecewiseDefaultCurve<C,I,B>::survivalProbabilityImpl(Time t) const {
        calculate();
        return base_curve::survivalProbabilityImpl(t);
    }

    template <class C, class I, template <class> class B>
    inline Real PiecewiseDefaultCurve<C,I,B>::defaultDensityImpl(Time t) const {
        calculate();
        return base_curve::defaultDensityImpl(t);
    }

    template <class C, class I, template <class> class B>
    inline Real PiecewiseDefaultCurve<C,I,B>::hazardRateImpl(Time t) const {
        calculate();
        return base_curve::hazardRateImpl(t);
    }

    template <class C, class I, template <class> class B>
    inline void PiecewiseDefaultCurve<C,I,B>::performCalculations() const {
        // just delegate to the bootstrapper
        bootstrap_.calculate();
    }

}

#endif
