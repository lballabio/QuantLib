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

/*! \file piecewisezeroinflationcurve.hpp
    \brief Piecewise zero-inflation term structure
*/

#ifndef quantlib_piecewise_zero_inflation_curve_hpp
#define quantlib_piecewise_zero_inflation_curve_hpp

#include <ql/functional.hpp>
#include <ql/patterns/lazyobject.hpp>
#include <ql/termstructures/inflation/inflationtraits.hpp>
#include <ql/termstructures/iterativebootstrap.hpp>
#include <utility>

namespace QuantLib {

    //! Piecewise zero-inflation term structure
    template <class Interpolator,
              template <class> class Bootstrap = IterativeBootstrap,
              class Traits = ZeroInflationTraits>
    class PiecewiseZeroInflationCurve
        : public InterpolatedZeroInflationCurve<Interpolator>,
          public LazyObject {
      private:
        typedef InterpolatedZeroInflationCurve<Interpolator> base_curve;
        typedef PiecewiseZeroInflationCurve<Interpolator,Bootstrap,Traits>
                                                                   this_curve;
        typedef std::function<Date()> BaseDateFunc;
      public:
        typedef Traits traits_type;
        typedef Interpolator interpolator_type;

        //! \name Constructors
        //@{
        PiecewiseZeroInflationCurve(
            const Date& referenceDate,
            Date baseDate,
            Frequency frequency,
            const DayCounter& dayCounter,
            std::vector<ext::shared_ptr<typename Traits::helper> > instruments,
            const ext::shared_ptr<Seasonality>& seasonality = {},
            Real accuracy = 1.0e-14,
            const Interpolator& i = Interpolator())
        : base_curve(referenceDate,
                     baseDate,
                     frequency,
                     dayCounter,
                     seasonality,
                     i),
          instruments_(std::move(instruments)), accuracy_(accuracy) {
            bootstrap_.setup(this);
        }

        PiecewiseZeroInflationCurve(
            const Date& referenceDate,
            BaseDateFunc baseDateFunc,
            Frequency frequency,
            const DayCounter& dayCounter,
            std::vector<ext::shared_ptr<typename Traits::helper> > instruments,
            const ext::shared_ptr<Seasonality>& seasonality = {},
            Real accuracy = 1.0e-14,
            const Interpolator& i = Interpolator())
        : base_curve(referenceDate,
                     Date(),
                     frequency,
                     dayCounter,
                     seasonality,
                     i),
          instruments_(std::move(instruments)), accuracy_(accuracy),
          baseDateFunc_(std::move(baseDateFunc)) {
            bootstrap_.setup(this);
        }
        //@}

        //! \name Inflation interface
        //@{
        Date baseDate() const override;
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
        Rate zeroRateImpl(Time t) const override;
        // data members
        std::vector<ext::shared_ptr<typename Traits::helper> > instruments_;
        Real accuracy_;
        BaseDateFunc baseDateFunc_;

        friend class Bootstrap<this_curve>;
        Bootstrap<this_curve> bootstrap_;
    };


    // inline and template definitions

    template <class I, template <class> class B, class T>
    inline Date PiecewiseZeroInflationCurve<I,B,T>::baseDate() const {
        if (baseDateFunc_)
            this->calculate();
        return base_curve::baseDate();
    }

    template <class I, template <class> class B, class T>
    inline Date PiecewiseZeroInflationCurve<I,B,T>::maxDate() const {
        this->calculate();
        return base_curve::maxDate();
    }

    template <class I, template <class> class B, class T>
    const std::vector<Time>& PiecewiseZeroInflationCurve<I,B,T>::times() const {
        calculate();
        return base_curve::times();
    }

    template <class I, template <class> class B, class T>
    const std::vector<Date>& PiecewiseZeroInflationCurve<I,B,T>::dates() const {
        calculate();
        return base_curve::dates();
    }

    template <class I, template <class> class B, class T>
    const std::vector<Real>& PiecewiseZeroInflationCurve<I,B,T>::data() const {
        calculate();
        return base_curve::rates();
    }

    template <class I, template <class> class B, class T>
    std::vector<std::pair<Date, Real> >
    PiecewiseZeroInflationCurve<I,B,T>::nodes() const {
        calculate();
        return base_curve::nodes();
    }

    template <class I, template <class> class B, class T>
    void PiecewiseZeroInflationCurve<I,B,T>::performCalculations() const {
        if (baseDateFunc_)
            const_cast<this_curve*>(this)->baseDate_ = baseDateFunc_();
        bootstrap_.calculate();
    }

    template <class I, template <class> class B, class T>
    Rate PiecewiseZeroInflationCurve<I,B,T>::zeroRateImpl(Time t) const {
        calculate();
        return base_curve::zeroRateImpl(t);
    }

    template <class I, template<class> class B, class T>
    void PiecewiseZeroInflationCurve<I,B,T>::update() {
        base_curve::update();
        LazyObject::update();
    }

}

#endif
