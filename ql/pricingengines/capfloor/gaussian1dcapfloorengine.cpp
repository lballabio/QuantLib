/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2013 Peter Caspers

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

#include <ql/pricingengines/capfloor/gaussian1dcapfloorengine.hpp>
#include <ql/math/interpolations/cubicinterpolation.hpp>
#include <ql/payoff.hpp>

namespace QuantLib {

    void Gaussian1dCapFloorEngine::calculate() const {

        for (double spread : arguments_.spreads)
            QL_REQUIRE(spread == 0.0, "Non zero spreads (" << spread << ") are not allowed.");

        Size optionlets = arguments_.startDates.size();
        std::vector<Real> values(optionlets, 0.0);
        std::vector<Real> forwards(optionlets, 0.0);
        Real value = 0.0;

        Date settlement = model_->termStructure()->referenceDate();

        CapFloor::Type type = arguments_.type;

        Array z = model_->yGrid(stddevs_, integrationPoints_);
        Array p(z.size());

        for (Size i = 0; i < optionlets; ++i) {

            Date valueDate = arguments_.startDates[i];
            Date paymentDate = arguments_.endDates[i];
            ext::shared_ptr<IborIndex> iborIndex =
                ext::dynamic_pointer_cast<IborIndex>(arguments_.indexes[i]);
            // if we do not find an ibor index with associated forwarding curve
            // we fall back on the model curve

            if (paymentDate > settlement) {

                Real f = arguments_.nominals[i] * arguments_.gearings[i];
                Date fixingDate = arguments_.fixingDates[i];
                Time fixingTime =
                    model_->termStructure()->timeFromReference(fixingDate);

                Real strike;

                if (type == CapFloor::Cap || type == CapFloor::Collar) {
                    strike = arguments_.capRates[i];
                    if (fixingDate <= settlement) {
                        values[i] =
                            std::max(arguments_.forwards[i] - strike, 0.0) * f *
                            arguments_.accrualTimes[i];
                    } else {

                        // todo add openmp support later on (as in gaussian1dswaptionengine)

                        for (Size j = 0; j < z.size(); j++) {
                            Real floatingLegNpv;
                            if (iborIndex != nullptr)
                                floatingLegNpv =
                                    arguments_.accrualTimes[i] *
                                    model_->forwardRate(fixingDate, fixingDate,
                                                        z[j], iborIndex) *
                                    model_->zerobond(paymentDate, fixingDate,
                                                     z[j], discountCurve_);
                            else
                                floatingLegNpv =
                                    (model_->zerobond(valueDate, fixingDate,
                                                      z[j]) -
                                     model_->zerobond(paymentDate, fixingDate,
                                                      z[j]));
                            Real fixedLegNpv =
                                arguments_.capRates[i] *
                                arguments_.accrualTimes[i] *
                                model_->zerobond(paymentDate, fixingDate, z[j]);
                            p[j] =
                                std::max((floatingLegNpv - fixedLegNpv), 0.0) /
                                model_->numeraire(fixingTime, z[j],
                                                  discountCurve_);
                        }
                        CubicInterpolation payoff(
                            z.begin(), z.end(), p.begin(),
                            CubicInterpolation::Spline, true,
                            CubicInterpolation::Lagrange, 0.0,
                            CubicInterpolation::Lagrange, 0.0);
                        Real price = 0.0;
                        for (Size j = 0; j < z.size() - 1; j++) {
                            price += model_->gaussianShiftedPolynomialIntegral(
                                0.0, payoff.cCoefficients()[j],
                                payoff.bCoefficients()[j],
                                payoff.aCoefficients()[j], p[j], z[j], z[j],
                                z[j + 1]);
                        }
                        if (extrapolatePayoff_) {
                            if (flatPayoffExtrapolation_) {
                                price +=
                                    model_->gaussianShiftedPolynomialIntegral(
                                        0.0, 0.0, 0.0, 0.0, p[z.size() - 2],
                                        z[z.size() - 2], z[z.size() - 1],
                                        100.0);
                                price +=
                                    model_->gaussianShiftedPolynomialIntegral(
                                        0.0, 0.0, 0.0, 0.0, p[0], z[0], -100.0,
                                        z[0]);
                            } else {
                                price +=
                                    model_->gaussianShiftedPolynomialIntegral(
                                        0.0,
                                        payoff.cCoefficients()[z.size() - 2],
                                        payoff.bCoefficients()[z.size() - 2],
                                        payoff.aCoefficients()[z.size() - 2],
                                        p[z.size() - 2], z[z.size() - 2],
                                        z[z.size() - 1], 100.0);
                            }
                        }
                        values[i] =
                            price *
                            model_->numeraire(0.0, 0.0, discountCurve_) * f;
                    }
                }
                if (type == CapFloor::Floor || type == CapFloor::Collar) {
                    strike = arguments_.floorRates[i];
                    Real floorlet;
                    if (fixingDate <= settlement) {
                        floorlet =
                            std::max(-(arguments_.forwards[i] - strike), 0.0) *
                            f * arguments_.accrualTimes[i];
                    } else {
                        for (Size j = 0; j < z.size(); j++) {
                            Real floatingLegNpv;
                            if (iborIndex != nullptr)
                                floatingLegNpv =
                                    arguments_.accrualTimes[i] *
                                    model_->forwardRate(fixingDate, fixingDate,
                                                        z[j], iborIndex) *
                                    model_->zerobond(paymentDate, fixingDate,
                                                     z[j], discountCurve_);
                            else
                                floatingLegNpv =
                                    (model_->zerobond(valueDate, fixingDate,
                                                      z[j]) -
                                     model_->zerobond(paymentDate, fixingDate,
                                                      z[j]));
                            Real fixedLegNpv =
                                arguments_.floorRates[i] *
                                arguments_.accrualTimes[i] *
                                model_->zerobond(paymentDate, fixingDate, z[j]);
                            p[j] =
                                std::max(-(floatingLegNpv - fixedLegNpv), 0.0) /
                                model_->numeraire(fixingTime, z[j],
                                                  discountCurve_);
                        }
                        CubicInterpolation payoff(
                            z.begin(), z.end(), p.begin(),
                            CubicInterpolation::Spline, true,
                            CubicInterpolation::Lagrange, 0.0,
                            CubicInterpolation::Lagrange, 0.0);
                        Real price = 0.0;
                        for (Size j = 0; j < z.size() - 1; j++) {
                            price += model_->gaussianShiftedPolynomialIntegral(
                                0.0, payoff.cCoefficients()[j],
                                payoff.bCoefficients()[j],
                                payoff.aCoefficients()[j], p[j], z[j], z[j],
                                z[j + 1]);
                        }
                        if (extrapolatePayoff_) {
                            if (flatPayoffExtrapolation_) {
                                price +=
                                    model_->gaussianShiftedPolynomialIntegral(
                                        0.0, 0.0, 0.0, 0.0, p[z.size() - 2],
                                        z[z.size() - 2], z[z.size() - 1],
                                        100.0);
                                price +=
                                    model_->gaussianShiftedPolynomialIntegral(
                                        0.0, 0.0, 0.0, 0.0, p[0], z[0], -100.0,
                                        z[0]);
                            } else {
                                price +=
                                    model_->gaussianShiftedPolynomialIntegral(
                                        0.0, payoff.cCoefficients()[0],
                                        payoff.bCoefficients()[0],
                                        payoff.aCoefficients()[0], p[0], z[0],
                                        -100.0, z[0]);
                            }
                        }
                        floorlet = price *
                                   model_->numeraire(0.0, 0.0, discountCurve_) *
                                   f;
                    }
                    if (type == CapFloor::Floor) {
                        values[i] = floorlet;
                    } else {
                        // a collar is long a cap and short a floor
                        values[i] -= floorlet;
                    }
                }

                value += values[i];
            }
        }

        results_.value = value;

        results_.additionalResults["optionletsPrice"] = values;
        results_.additionalResults["optionletsAtmForward"] = forwards;
    }

}
