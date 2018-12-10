/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2011, 2012 Klaus Spanderen

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

/*! \file fdmvppstepcondition.hpp
    \brief VPP step condition for FD models
*/

#ifndef quantlib_fdm_vpp_step_condition_hpp
#define quantlib_fdm_vpp_step_condition_hpp

#include <ql/methods/finitedifferences/stepcondition.hpp>
#include <ql/shared_ptr.hpp>
#include <ql/functional.hpp>
#include <vector>

namespace QuantLib {
    class FdmMesher;
    class FdmLinearOpIterator;
    class FdmInnerValueCalculator;

    struct FdmVPPStepConditionParams {
        const Real heatRate;
        const Real pMin; const Real pMax;
        const Size tMinUp; const Size tMinDown;
        const Real startUpFuel; const Real startUpFixCost;
        const Real fuelCostAddon;
    };

    struct FdmVPPStepConditionMesher {
        const Size stateDirection;
        const ext::shared_ptr<FdmMesher> mesher;
    };

    class FdmVPPStepCondition : public StepCondition<Array> {
      public:
        FdmVPPStepCondition(
            const FdmVPPStepConditionParams& params,
            Size nStates,
            const FdmVPPStepConditionMesher& mesh,
            const ext::shared_ptr<FdmInnerValueCalculator>& gasPrice,
            const ext::shared_ptr<FdmInnerValueCalculator>& sparkSpreadPrice);

        Size nStates() const;
        void applyTo(Array& a, Time t) const;

        virtual Real maxValue(const Array& states) const = 0;

      protected:
        Real evolveAtPMin(Real sparkSpread) const;
        Real evolveAtPMax(Real sparkSpread) const;

        Real evolve(const FdmLinearOpIterator& iter, Time t) const;

        virtual Disposable<Array> changeState(
            Real gasPrice, const Array& state, Time t) const = 0;

        const Real heatRate_;
        const Real pMin_, pMax_;
        const Size tMinUp_, tMinDown_;
        const Real startUpFuel_, startUpFixCost_;
        const Real fuelCostAddon_;
        const Size stateDirection_;
        const Size nStates_;

        const ext::shared_ptr<FdmMesher> mesher_;
        const ext::shared_ptr<FdmInnerValueCalculator> gasPrice_;
        const ext::shared_ptr<FdmInnerValueCalculator> sparkSpreadPrice_;

        std::vector<ext::function<Real (Real)> > stateEvolveFcts_;
    };
}

#endif
