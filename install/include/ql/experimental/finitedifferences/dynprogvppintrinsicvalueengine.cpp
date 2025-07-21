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

/*! \file dynprogvppintrinsicvalueengine.cpp
*/

#include <ql/experimental/finitedifferences/dynprogvppintrinsicvalueengine.hpp>
#include <ql/experimental/finitedifferences/fdmvppstepconditionfactory.hpp>
#include <ql/methods/finitedifferences/meshers/fdmmeshercomposite.hpp>
#include <ql/methods/finitedifferences/meshers/uniform1dmesher.hpp>
#include <ql/methods/finitedifferences/operators/fdmlinearoplayout.hpp>
#include <ql/methods/finitedifferences/utilities/fdminnervaluecalculator.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <utility>


namespace QuantLib {
    namespace {
        class SparkSpreadPrice : public FdmInnerValueCalculator {
          public:
            SparkSpreadPrice(Real heatRate,
                             const std::vector<Real>& fuelPrices,
                             const std::vector<Real>& powerPrices)
            : heatRate_(heatRate),
              fuelPrices_(fuelPrices),
              powerPrices_(powerPrices) {}

            Real innerValue(const FdmLinearOpIterator&, Time t) override {
                Size i = (Size) t;
                QL_REQUIRE(i < powerPrices_.size(), "invalid time");
                return powerPrices_[i] - heatRate_*fuelPrices_[i];
            }
            Real avgInnerValue(const FdmLinearOpIterator& iter, Time t) override {
                return innerValue(iter, t);
            }

          private:
            const Real heatRate_;
            const std::vector<Real>& fuelPrices_;
            const std::vector<Real>& powerPrices_;
        };


        class FuelPrice : public FdmInnerValueCalculator {
          public:
            explicit FuelPrice(const std::vector<Real>& fuelPrices)
            : fuelPrices_(fuelPrices) {}

            Real innerValue(const FdmLinearOpIterator&, Time t) override {
                Size i = (Size) t;
                QL_REQUIRE(i < fuelPrices_.size(), "invalid time");
                return fuelPrices_[(Size) t];
            }
            Real avgInnerValue(const FdmLinearOpIterator& iter, Time t) override {
                return innerValue(iter, t);
            }

          private:
            const std::vector<Real>& fuelPrices_;
        };
    }

    DynProgVPPIntrinsicValueEngine::DynProgVPPIntrinsicValueEngine(
        std::vector<Real> fuelPrices,
        std::vector<Real> powerPrices,
        Real fuelCostAddon,
        ext::shared_ptr<YieldTermStructure> rTS)
    : fuelPrices_(std::move(fuelPrices)), powerPrices_(std::move(powerPrices)),
      fuelCostAddon_(fuelCostAddon), rTS_(std::move(rTS)) {}

    void DynProgVPPIntrinsicValueEngine::calculate() const {
        const ext::shared_ptr<FdmInnerValueCalculator> fuelPrice(
            new FuelPrice(fuelPrices_));
        const ext::shared_ptr<FdmInnerValueCalculator> sparkSpreadPrice(
            new SparkSpreadPrice(arguments_.heatRate,fuelPrices_,powerPrices_));

        const FdmVPPStepConditionFactory stepConditionFactory(arguments_);

        const ext::shared_ptr<FdmMesher> mesher(
            new FdmMesherComposite(stepConditionFactory.stateMesher()));

        const FdmVPPStepConditionMesher mesh = { 0, mesher };

        const ext::shared_ptr<FdmVPPStepCondition> stepCondition(
            stepConditionFactory.build(mesh, fuelCostAddon_,
                                       fuelPrice, sparkSpreadPrice));

        Array state(mesher->layout()->dim()[0], 0.0);
        for (Size j=powerPrices_.size(); j > 0; --j) {
            stepCondition->applyTo(state, (Time) j-1);
        }

        results_.value = stepCondition->maxValue(state);
    }
}

