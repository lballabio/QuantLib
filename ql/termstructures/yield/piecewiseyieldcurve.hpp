/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005, 2006, 2007, 2008 StatPro Italia srl
 Copyright (C) 2007, 2008, 2009 Ferdinando Ametrano
 Copyright (C) 2007 Chris Kenyon
 Copyright (C) 2026 Kyrylo Protsenko

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

/*! \file piecewiseyieldcurve.hpp
    \brief piecewise-interpolated term structure
*/

#ifndef quantlib_piecewise_yield_curve_hpp
#define quantlib_piecewise_yield_curve_hpp

#include <ql/patterns/lazyobject.hpp>
#include <ql/math/matrix.hpp>
#include <ql/experimental/termstructures/jacobian/curvesensitivitypropagation.hpp>
#include <ql/termstructures/iterativebootstrap.hpp>
#include <ql/termstructures/globalbootstrap.hpp>
#include <ql/termstructures/multicurve.hpp>
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
          public LazyObject,
          public MultiCurveBootstrapProvider,
          public CurveJacobianNodeProvider {
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
        : PiecewiseYieldCurve(std::move(instruments), std::move(bootstrap),
                              referenceDate, dayCounter, jumps, jumpDates, i) {}

        PiecewiseYieldCurve(const Date& referenceDate,
                            std::vector<ext::shared_ptr<typename Traits::helper> > instruments,
                            const DayCounter& dayCounter,
                            const Interpolator& i,
                            bootstrap_type bootstrap = {})
        : PiecewiseYieldCurve(std::move(instruments), std::move(bootstrap),
                              referenceDate, dayCounter,
                              std::vector<Handle<Quote>>(), std::vector<Date>(), i) {}

        PiecewiseYieldCurve(const Date& referenceDate,
                            std::vector<ext::shared_ptr<typename Traits::helper> > instruments,
                            const DayCounter& dayCounter,
                            bootstrap_type bootstrap)
        : PiecewiseYieldCurve(std::move(instruments), std::move(bootstrap),
                              referenceDate, dayCounter) {}

        PiecewiseYieldCurve(
            Natural settlementDays,
            const Calendar& calendar,
            std::vector<ext::shared_ptr<typename Traits::helper> > instruments,
            const DayCounter& dayCounter,
            const std::vector<Handle<Quote> >& jumps = {},
            const std::vector<Date>& jumpDates = {},
            const Interpolator& i = {},
            bootstrap_type bootstrap = {})
        : PiecewiseYieldCurve(std::move(instruments), std::move(bootstrap),
                              settlementDays, calendar, dayCounter, jumps, jumpDates, i) {}

        PiecewiseYieldCurve(Natural settlementDays,
                            const Calendar& calendar,
                            std::vector<ext::shared_ptr<typename Traits::helper> > instruments,
                            const DayCounter& dayCounter,
                            const Interpolator& i,
                            bootstrap_type bootstrap = {})
        : PiecewiseYieldCurve(std::move(instruments), std::move(bootstrap),
                              settlementDays, calendar, dayCounter,
                              std::vector<Handle<Quote>>(), std::vector<Date>(), i) {}

        PiecewiseYieldCurve(
               Natural settlementDays,
               const Calendar& calendar,
               std::vector<ext::shared_ptr<typename Traits::helper> > instruments,
               const DayCounter& dayCounter,
               bootstrap_type bootstrap)
        : PiecewiseYieldCurve(std::move(instruments), std::move(bootstrap),
                              settlementDays, calendar, dayCounter) {}
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
        //! \name Jacobian
        //@{
        /*! Jacobian of helper quotes with respect to free curve nodes.
            Rows follow alive helpers and columns follow data()[1..]. For
            multi-curve dependencies, other curve nodes are fixed.
        */
        Matrix jacobian(std::vector<bool>* analyticEquations = nullptr) const;

        /*! Jacobian of free curve nodes with respect to helper quotes.
            For a stand-alone curve, this is inverse(jacobian()). For a
            MultiCurve group, columns cover all members' quotes in
            registration order. Group feedback is included.
        */
        Matrix inverseJacobian(std::vector<bool>* analyticEquations = nullptr) const;
        //@}
        //! \name Observer interface
        //@{
        void update() override;
        //@}
        const MultiCurveBootstrapContributor* multiCurveBootstrapContributor() const override {
            if constexpr (std::is_convertible_v<bootstrap_type*, MultiCurveBootstrapContributor*>) {
                return &bootstrap_;
            } else {
                return nullptr;
            }
        }

      protected:
        template <class... Args>
        PiecewiseYieldCurve(
            std::vector<ext::shared_ptr<typename Traits::helper>> instruments,
            bootstrap_type bootstrap,
            Args&&... args)
        : base_curve(std::forward<Args>(args)...), instruments_(std::move(instruments)),
          accuracy_(1.0e-12), bootstrap_(std::move(bootstrap)) {
            bootstrap_.setup(this);
        }
        //! \name LazyObject interface
        //@{
        void performCalculations() const override;
        //@}
      private:
        // methods
        detail::CurveJacobianNode makeJacobianNode(const ext::shared_ptr<YieldTermStructure>& curve) const override {
            auto self = ext::dynamic_pointer_cast<this_curve>(curve);
            QL_REQUIRE(self.get() == this,
                       "Jacobian node requested with a different curve");
            return detail::BootstrapJacobianAccess<this_curve>::makeNode(self);
        }
        detail::BootstrapJacobian calculateJacobian() const;
        DiscountFactor discountImpl(Time) const override;
        // data members
        std::vector<ext::shared_ptr<typename Traits::helper> > instruments_;
        Real accuracy_;

        // bootstrapper classes are declared as friend to manipulate
        // the curve data. They might be passed the data instead, but
        // it would increase the complexity---which is high enough
        // already.
        friend class Bootstrap<this_curve>;
        // access needed for cross-curve Jacobians
        template <class> friend struct detail::BootstrapJacobianAccess;
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

    template <class Traits, class Interpolator, template <class> class Bootstrap>
    detail::BootstrapJacobian
    PiecewiseYieldCurve<Traits, Interpolator, Bootstrap>::calculateJacobian() const {
        return detail::bootstrapEquationJacobian<Traits>(
            this, instruments_, this->times_, this->data_, this->interpolation_,
            !this->jumpDates().empty());
    }

    template <class Traits, class Interpolator, template <class> class Bootstrap>
    Matrix PiecewiseYieldCurve<Traits, Interpolator, Bootstrap>::jacobian(
                                       std::vector<bool>* analyticEquations) const {
        calculate();
        detail::BootstrapJacobian result = calculateJacobian();
        if (analyticEquations != nullptr)
            *analyticEquations = result.analyticEquations;
        return std::move(result.matrix);
    }

    template <class Traits, class Interpolator, template <class> class Bootstrap>
    Matrix PiecewiseYieldCurve<Traits, Interpolator, Bootstrap>::inverseJacobian(
                                       std::vector<bool>* analyticEquations) const {
        calculate();
        if constexpr (detail::hasJacobianGroup<bootstrap_type>) {
            auto group = bootstrap_.jacobianGroup();
            if (!group.members.empty()) {
                // differentiate known dependent wrappers numerically
                detail::CurveCrossJacobianContext context;
                context.addNumericallyPropagatedCurves(group.dependents);
                context.assumeUnlistedCurvesIndependent();
                QL_REQUIRE(group.target < group.members.size(),
                           "invalid target curve in Jacobian group");
                detail::CurveJacobianBlocks blocks =
                    detail::curveJacobianBlocks(group.members, context);
                if (analyticEquations != nullptr) {
                    analyticEquations->clear();
                    for (const auto& flags : blocks.analyticQuotes)
                        analyticEquations->insert(analyticEquations->end(),
                                                  flags.begin(), flags.end());
                }
                return detail::inverseCurveJacobianRows(blocks, group.target);
            }
        }
        return detail::inverseBootstrapEquationJacobian(
            jacobian(analyticEquations));
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
