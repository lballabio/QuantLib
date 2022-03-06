/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009 Chris Kenyon
 Copyright (C) 2011 Ferdinando Ametrano

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

/*! \file piecewiseyoyoptionletvolatility.hpp
    \brief piecewise yoy inflation volatility term structure
*/

#ifndef quantlib_piecewise_yoy_optionlet_volatility_hpp
#define quantlib_piecewise_yoy_optionlet_volatility_hpp

#include <ql/experimental/inflation/yoyinflationoptionletvolatilitystructure2.hpp>
#include <ql/patterns/lazyobject.hpp>
#include <ql/termstructures/iterativebootstrap.hpp>
#include <utility>

namespace QuantLib {

    //! traits for inflation-volatility bootstrap
    class YoYInflationVolatilityTraits {
      public:
        typedef BootstrapHelper<YoYOptionletVolatilitySurface> helper;

        // start of curve data
        static Date initialDate(const YoYOptionletVolatilitySurface *s) {
            return s->baseDate();
        }
        // value at reference date
        static Real initialValue(const YoYOptionletVolatilitySurface *s) {
            return s->baseLevel();  // REALLLYYYY important because
                                    // generally don't have a clue
                                    // what this should be - embodies
                                    // assumptions on early options
                                    // that are _not_ quoted
        }

        // guesses
        template <class C>
        static Real guess(Size i,
                          const C* c,
                          bool validData,
                          Size) // firstAliveHelper
        {
            if (validData) // previous iteration value
                return c->data()[i];

            if (i==1) // first pillar
                return 0.005;

            // could/should extrapolate
            return 0.002;
        }

        // constraints
        template <class C>
        static Real minValueAfter(Size i,
                                  const C* c,
                                  bool,
                                  Size) // firstAliveHelper
        {
            return std::max(0.0, c->data()[i-1] - 0.02); // vol cannot be negative
        }
        template <class C>
        static Real maxValueAfter(Size i,
                                  const C* c,
                                  bool,
                                  Size) // firstAliveHelper
        {
            return c->data()[i-1] + 0.02;
        }

        // root-finding update
        static void updateGuess(std::vector<Real> &vols,
                                Real level,
                                Size i) {
            vols[i] = level;
        }
        // upper bound for convergence loop
        static Size maxIterations() {return 25;}
    };


    //! Piecewise year-on-year inflation volatility term structure
    /*! We use a flat smile for bootstrapping at constant K.  Happily
        most of the work has already been done in the bootstrapping
        classes.  We only need to add special attention for the start
        where there is usually no data, only assumptions.
    */
    template <class Interpolator,
              template <class> class Bootstrap = IterativeBootstrap,
              class Traits = YoYInflationVolatilityTraits>
    class PiecewiseYoYOptionletVolatilityCurve
        : public InterpolatedYoYOptionletVolatilityCurve<Interpolator>,
          public LazyObject {
      private:
        typedef InterpolatedYoYOptionletVolatilityCurve<Interpolator>
                                                                   base_curve;
        typedef PiecewiseYoYOptionletVolatilityCurve<Interpolator,
                                                     Bootstrap,
                                                     Traits> this_curve;
      public:
        typedef Traits traits_type;
        typedef Interpolator interpolator_type;

        PiecewiseYoYOptionletVolatilityCurve(
            Natural settlementDays,
            const Calendar& cal,
            BusinessDayConvention bdc,
            const DayCounter& dc,
            const Period& lag,
            Frequency frequency,
            bool indexIsInterpolated,
            Rate minStrike,
            Rate maxStrike,
            Volatility baseYoYVolatility,
            std::vector<ext::shared_ptr<typename Traits::helper> > instruments,
            Real accuracy = 1.0e-12,
            const Interpolator& interpolator = Interpolator())
        : base_curve(settlementDays,
                     cal,
                     bdc,
                     dc,
                     lag,
                     frequency,
                     indexIsInterpolated,
                     minStrike,
                     maxStrike,
                     baseYoYVolatility,
                     interpolator),
          instruments_(std::move(instruments)), accuracy_(accuracy) {
            bootstrap_.setup(this);
        }

        //! \name Inflation interface
        //@{
        Date baseDate() const override;
        Date maxDate() const override;
        //@
        //! \name Inspectors
        //@{
        const std::vector<Time>& times() const override;
        const std::vector<Date>& dates() const override;
        const std::vector<Real>& data() const override;
        std::vector<std::pair<Date, Real> > nodes() const override;
        //@}
        //! \name Observer interface
        //@{
        void update() override;
        //@}
      private:
        // methods
        void performCalculations() const override;
        // data members
        std::vector<ext::shared_ptr<typename Traits::helper> > instruments_;
        Real accuracy_;

        friend class Bootstrap<this_curve>;
        friend class BootstrapError<this_curve>;
        Bootstrap<this_curve> bootstrap_;
    };


    // inline and template definitions

    template <class I, template <class> class B, class T>
    inline Date PiecewiseYoYOptionletVolatilityCurve<I,B,T>::baseDate() const {
        this->calculate();
        return base_curve::baseDate();
    }

    template <class I, template <class> class B, class T>
    inline Date PiecewiseYoYOptionletVolatilityCurve<I,B,T>::maxDate() const {
        this->calculate();
        return base_curve::maxDate();
    }

    template <class I, template <class> class B, class T>
    const std::vector<Time>&
    PiecewiseYoYOptionletVolatilityCurve<I,B,T>::times() const {
        calculate();
        return base_curve::times();
    }

    template <class I, template <class> class B, class T>
    const std::vector<Date>&
    PiecewiseYoYOptionletVolatilityCurve<I,B,T>::dates() const {
        calculate();
        return base_curve::dates();
    }

    template <class I, template <class> class B, class T>
    const std::vector<Real>&
    PiecewiseYoYOptionletVolatilityCurve<I,B,T>::data() const {
        calculate();
        return base_curve::data();
    }

    template <class I, template <class> class B, class T>
    std::vector<std::pair<Date, Real> >
    PiecewiseYoYOptionletVolatilityCurve<I,B,T>::nodes() const {
        calculate();
        return base_curve::nodes();
    }

    template <class I, template <class> class B, class T>
    void
    PiecewiseYoYOptionletVolatilityCurve<I,B,T>::performCalculations() const {
        bootstrap_.calculate();
    }

    template <class I, template <class> class B, class T>
    void PiecewiseYoYOptionletVolatilityCurve<I,B,T>::update() {
        base_curve::update();
        LazyObject::update();
    }

}

#endif


#ifndef id_85ac9db09a8f68f6639bdccb66f11102
#define id_85ac9db09a8f68f6639bdccb66f11102
inline bool test_85ac9db09a8f68f6639bdccb66f11102(int* i) { return i != 0; }
#endif
