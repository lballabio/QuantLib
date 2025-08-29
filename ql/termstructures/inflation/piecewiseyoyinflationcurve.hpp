/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Chris Kenyon
 Copyright (C) 2007, 2008 StatPro Italia srl
 Copyright (C) 2011 Ferdinando Ametrano

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

/*! \file piecewiseyoyinflationcurve.hpp
    \brief Piecewise year-on-year inflation term structure
*/

#ifndef quantlib_piecewise_yoy_inflation_curve_hpp
#define quantlib_piecewise_yoy_inflation_curve_hpp

#include <ql/patterns/lazyobject.hpp>
#include <ql/termstructures/inflation/inflationtraits.hpp>
#include <ql/termstructures/iterativebootstrap.hpp>
#include <utility>

namespace QuantLib {

    //! Piecewise year-on-year inflation term structure
    template <class Interpolator,
              template <class> class Bootstrap = IterativeBootstrap,
              class Traits = YoYInflationTraits>
    class PiecewiseYoYInflationCurve:
        public InterpolatedYoYInflationCurve<Interpolator>,
        public LazyObject {
      private:
        typedef InterpolatedYoYInflationCurve<Interpolator> base_curve;
        typedef PiecewiseYoYInflationCurve<Interpolator,Bootstrap,Traits>
                                                                   this_curve;
      public:
        typedef Traits traits_type;
        typedef Interpolator interpolator_type;

        //! \name Constructors
        //@{

        PiecewiseYoYInflationCurve(
            const Date& referenceDate,
            Date baseDate,
            Rate baseYoYRate,
            Frequency frequency,
            const DayCounter& dayCounter,
            std::vector<ext::shared_ptr<typename Traits::helper> > instruments,
            const ext::shared_ptr<Seasonality>& seasonality = {},
            Real accuracy = 1.0e-12,
            const Interpolator& i = Interpolator())
        : base_curve(referenceDate,
                     baseDate,
                     baseYoYRate,
                     frequency,
                     dayCounter,
                     seasonality,
                     i),
          instruments_(std::move(instruments)), accuracy_(accuracy) {
            bootstrap_.setup(this);
        }

        /*! \deprecated Use the overload without indexIsInterpolated.
                        Deprecated in version 1.37.
        */
        [[deprecated("Use the overload without indexIsInterpolated")]]
        PiecewiseYoYInflationCurve(
            const Date& referenceDate,
            Date baseDate,
            Rate baseYoYRate,
            Frequency frequency,
            bool indexIsInterpolated,
            const DayCounter& dayCounter,
            std::vector<ext::shared_ptr<typename Traits::helper> > instruments,
            const ext::shared_ptr<Seasonality>& seasonality = {},
            Real accuracy = 1.0e-12,
            const Interpolator& i = Interpolator())
        : PiecewiseYoYInflationCurve(referenceDate, baseDate, baseYoYRate, frequency,
                                     dayCounter, instruments, seasonality, accuracy, i) {
            QL_DEPRECATED_DISABLE_WARNING
            this->indexIsInterpolated_ = indexIsInterpolated;
            QL_DEPRECATED_ENABLE_WARNING
        }
        //@}

        //! \name Inflation interface
        //@{
        Date maxDate() const override;
        //@
        //! \name Inspectors
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
        // methods
        void performCalculations() const override;
        Rate yoyRateImpl(Time t) const override;
        // data members
        std::vector<ext::shared_ptr<typename Traits::helper> > instruments_;
        Real accuracy_;

        friend class Bootstrap<this_curve>;
        Bootstrap<this_curve> bootstrap_;
    };


    // inline and template definitions

    template <class I, template <class> class B, class T>
    inline Date PiecewiseYoYInflationCurve<I,B,T>::maxDate() const {
        this->calculate();
        return base_curve::maxDate();
    }

    template <class I, template <class> class B, class T>
    const std::vector<Time>& PiecewiseYoYInflationCurve<I,B,T>::times() const {
        calculate();
        return base_curve::times();
    }

    template <class I, template <class> class B, class T>
    const std::vector<Date>& PiecewiseYoYInflationCurve<I,B,T>::dates() const {
        calculate();
        return base_curve::dates();
    }

    template <class I, template <class> class B, class T>
    const std::vector<Real>& PiecewiseYoYInflationCurve<I,B,T>::data() const {
        calculate();
        return base_curve::data();
    }

    template <class I, template <class> class B, class T>
    std::vector<std::pair<Date, Real> >
    PiecewiseYoYInflationCurve<I,B,T>::nodes() const {
        calculate();
        return base_curve::nodes();
    }

    template <class I, template <class> class B, class T>
    void PiecewiseYoYInflationCurve<I,B,T>::performCalculations() const {
        bootstrap_.calculate();
    }

    template <class I, template <class> class B, class T>
    Rate PiecewiseYoYInflationCurve<I,B,T>::yoyRateImpl(Time t) const {
        calculate();
        return base_curve::yoyRateImpl(t);
    }

    template <class I, template <class> class B, class T>
    void PiecewiseYoYInflationCurve<I,B,T>::update() {
        base_curve::update();
        LazyObject::update();
    }

}

#endif
