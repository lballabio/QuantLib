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

/*! \file fdmvppstepconditionfactory.cpp
*/

#include <ql/experimental/finitedifferences/fdmvppstartlimitstepcondition.hpp>
#include <ql/experimental/finitedifferences/fdmvppstepconditionfactory.hpp>
#include <ql/methods/finitedifferences/meshers/uniform1dmesher.hpp>
#include <utility>

namespace QuantLib {

    FdmVPPStepConditionFactory::FdmVPPStepConditionFactory(VanillaVPPOption::arguments args)
    : args_(std::move(args)) {
        QL_REQUIRE(!(   args_.nStarts       != Null<Size>()
                     && args_.nRunningHours != Null<Size>()),
                   "start and running hour limt together is not supported");

        if (   args_.nRunningHours == Null<Size>()
            && args_.nStarts == Null<Size>()) {
            type_ = Vanilla;
        }
        else if (args_.nRunningHours == Null<Size>()) {
            type_ = StartLimit;
        }
        else {
            type_ = RunningHourLimit;
        }
    }

    ext::shared_ptr<Fdm1dMesher>
    FdmVPPStepConditionFactory::stateMesher() const {
        Size nStates;
        switch (type_) {
          case Vanilla:
            nStates = 2*args_.tMinUp + args_.tMinDown;
            break;
          case StartLimit:
            nStates = FdmVPPStartLimitStepCondition::nStates(
                       args_.tMinUp,args_.tMinDown, args_.nStarts);
            break;
          default:
            QL_FAIL("vpp type is not supported");
        }

        return ext::shared_ptr<Fdm1dMesher>(
            new Uniform1dMesher(0.0, 1.0, nStates));
    }

    ext::shared_ptr<FdmVPPStepCondition> FdmVPPStepConditionFactory::build(
        const FdmVPPStepConditionMesher& mesh,
        Real fuelCostAddon,
        const ext::shared_ptr<FdmInnerValueCalculator>& fuel,
        const ext::shared_ptr<FdmInnerValueCalculator>& spark) const {

        const FdmVPPStepConditionParams params = {
            args_.heatRate, args_.pMin, args_.pMax,
            args_.tMinUp, args_.tMinDown,
            args_.startUpFuel, args_.startUpFixCost,
            fuelCostAddon
        };

        switch (type_) {
          case Vanilla:
          case StartLimit:
              return ext::shared_ptr<FdmVPPStepCondition>(
                  new FdmVPPStartLimitStepCondition(params, args_.nStarts,
                          mesh, fuel, spark));
              break;
          default:
            QL_FAIL("vpp type is not supported");
        }
    }
}
