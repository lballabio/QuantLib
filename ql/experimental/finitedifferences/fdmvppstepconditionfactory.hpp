/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2012 Klaus Spanderen

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

/*! \file fdmvppstepconditionfactory.hpp
    \brief factory for VPP step conditions for FD models
*/

#ifndef quantlib_fdm_vpp_step_condition_factory_hpp
#define quantlib_fdm_vpp_step_condition_factory_hpp

#include <ql/math/array.hpp>
#include <ql/methods/finitedifferences/stepcondition.hpp>
#include <ql/experimental/finitedifferences/vanillavppoption.hpp>
#include <ql/experimental/finitedifferences/fdmvppstepcondition.hpp>

namespace QuantLib {
    class Fdm1dMesher;

    class FdmVPPStepConditionFactory {
      public:
        FdmVPPStepConditionFactory(const VanillaVPPOption::arguments& args);

        boost::shared_ptr<Fdm1dMesher> stateMesher() const;
        boost::shared_ptr<FdmVPPStepCondition> build(
            const FdmVPPStepConditionMesher& mesh,
            Real fuelCostAddon,
            const boost::shared_ptr<FdmInnerValueCalculator>& fuel,
            const boost::shared_ptr<FdmInnerValueCalculator>& spark) const;

      private:
        enum Type { Vanilla, StartLimit, RunningHourLimit } type_;

        const VanillaVPPOption::arguments args_;
    };
}

#endif
