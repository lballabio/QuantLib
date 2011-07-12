/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2011 Klaus Spanderen

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

/*! \file fdmvppstatetransitionmatrix.hpp
*/

#ifndef quantlib_fdm_vpp_step_condition_hpp
#define quantlib_fdm_vpp_step_condition_hpp

#include <ql/methods/finitedifferences/stepcondition.hpp>

#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
#include <vector>

namespace QuantLib {
    class FdmMesher;
    class FdmLinearOpIterator;
    class FdmInnerValueCalculator;

    class FdmVPPStepCondition : public StepCondition<Array> {
      public:
        FdmVPPStepCondition(
            Real heatRate,
            Real pMin, Real pMax,
            Size tMinUp, Size tMinDown,
            Real startUpFuel, Real startUpFixCost,
            Real carbonPrice,
            Size stateDirection,
            const boost::shared_ptr<FdmMesher>& mesher,
            const boost::shared_ptr<FdmInnerValueCalculator>& gasPrice,
            const boost::shared_ptr<FdmInnerValueCalculator>& sparkSpreadPrice);

        void applyTo(Array& a, Time t) const;

      private:
        Real evolveAtPMin(Real sparkSpread) const;
        Real evolveAtPMax(Real sparkSpread) const;

        Real evolve(const FdmLinearOpIterator& iter, Time t) const;
        Disposable<Array> changeState(Real gasPrice,
                                      const Array& state, Time t) const;

        const Real heatRate_;
        const Real pMin_, pMax_;
        const Size tMinUp_, tMinDown_;
        const Real startUpFuel_, startUpFixCost_;
        const Real carbonPrice_;
        const Size stateDirection_;

        const boost::shared_ptr<FdmMesher> mesher_;
        const boost::shared_ptr<FdmInnerValueCalculator> gasPrice_;
        const boost::shared_ptr<FdmInnerValueCalculator> sparkSpreadPrice_;

        std::vector<boost::function<Real (Real)> > stateEvolveFcts_;
    };
}

#endif
