/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Allen Kuo

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

/*! \file treecallablebondengine.hpp
    \brief Numerical lattice engines for callable/puttable bonds
*/

#ifndef quantlib_tree_callable_bond_engine_hpp
#define quantlib_tree_callable_bond_engine_hpp

#include <ql/experimental/callablebonds/callablebond.hpp>
#include <ql/pricingengines/latticeshortratemodelengine.hpp>

namespace QuantLib {

    //! Numerical lattice engine for callable fixed rate bonds
    /*! \ingroup callablebondengines */
    class TreeCallableFixedRateBondEngine
        : public LatticeShortRateModelEngine<CallableBond::arguments,
                                             CallableBond::results> {
      public:
        /*! \name Constructors
            \note the term structure is only needed when the short-rate
                  model cannot provide one itself.
        */
        //@{
        TreeCallableFixedRateBondEngine(
            const std::shared_ptr<ShortRateModel>&,
            Size timeSteps,
            Handle<YieldTermStructure> termStructure = Handle<YieldTermStructure>());
        TreeCallableFixedRateBondEngine(
            const std::shared_ptr<ShortRateModel>&,
            const TimeGrid& timeGrid,
            Handle<YieldTermStructure> termStructure = Handle<YieldTermStructure>());
        //@}
        void calculate() const override;

      private:
        void calculateWithSpread(Spread s) const;
        Handle<YieldTermStructure> termStructure_;
    };

    //! Numerical lattice engine for callable zero coupon bonds
    /*! \ingroup callablebondengines */
    class TreeCallableZeroCouponBondEngine
        : public TreeCallableFixedRateBondEngine {

      public:
        TreeCallableZeroCouponBondEngine(
                           const std::shared_ptr<ShortRateModel>& model,
                           const Size timeSteps,
                           const Handle<YieldTermStructure>& termStructure =
                                                 Handle<YieldTermStructure>())
        : TreeCallableFixedRateBondEngine(model, timeSteps, termStructure) {}

        TreeCallableZeroCouponBondEngine(
                               const std::shared_ptr<ShortRateModel>& model,
                               const TimeGrid& timeGrid,
                           const Handle<YieldTermStructure>& termStructure =
                                                 Handle<YieldTermStructure>())
        : TreeCallableFixedRateBondEngine(model, timeGrid, termStructure) {}
    };

}

#endif
