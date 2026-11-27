/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2013 Peter Caspers

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

#include <ql/pricingengines/capfloor/gaussian1dcapfloorengine.hpp>
#include <ql/math/interpolations/cubicinterpolation.hpp>
#include <ql/payoff.hpp>

namespace QuantLib {

    void Gaussian1dCapFloorEngine::calculate() const {

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

                // with a negative gearing, a cap on the coupon is a floor on
                // the index and vice versa; the effective strikes are
                // already adjusted by CapFloor::setupArguments
                Real gearing = arguments_.gearings[i];
                Real sign = gearing >= 0.0 ? 1.0 : -1.0;
                Real f = arguments_.nominals[i] * std::fabs(gearing);
                Date fixingDate = arguments_.fixingDates[i];
                Time fixingTime =
                    model_->termStructure()->timeFromReference(fixingDate);

                // value per unit of nominal and gearing of an option paying
                // max(omega * (L - strike), 0), with omega = 1 for a call
                // on the index and -1 for a put
                auto optionlet = [&](Rate strike, Real omega) -> Real {
                    if (fixingDate <= settlement)
                        return std::max<Real>(omega * (arguments_.forwards[i] - strike),
                                              0.0) *
                               arguments_.accrualTimes[i] *
                               model_->zerobond(paymentDate, Date(), 0.0, discountCurve_);

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
                            strike * arguments_.accrualTimes[i] *
                            model_->zerobond(paymentDate, fixingDate, z[j]);
                        p[j] = std::max<Real>(omega * (floatingLegNpv - fixedLegNpv), 0.0) /
                               model_->numeraire(fixingTime, z[j], discountCurve_);
                    }
                    CubicInterpolation payoff(
                        z.begin(), z.end(), p.begin(),
                        CubicInterpolation::Spline, true,
                        CubicInterpolation::Lagrange, 0.0,
                        CubicInterpolation::Lagrange, 0.0);
                    Real price = 0.0;
                    for (Size j = 0; j < z.size() - 1; j++) {
                        price += Gaussian1dModel::gaussianShiftedPolynomialIntegral(
                            0.0, payoff.cCoefficients()[j],
                            payoff.bCoefficients()[j],
                            payoff.aCoefficients()[j], p[j], z[j], z[j],
                            z[j + 1]);
                    }
                    if (extrapolatePayoff_) {
                        if (flatPayoffExtrapolation_) {
                            price +=
                                Gaussian1dModel::gaussianShiftedPolynomialIntegral(
                                    0.0, 0.0, 0.0, 0.0, p[z.size() - 2],
                                    z[z.size() - 2], z[z.size() - 1],
                                    100.0);
                            price +=
                                Gaussian1dModel::gaussianShiftedPolynomialIntegral(
                                    0.0, 0.0, 0.0, 0.0, p[0], z[0], -100.0,
                                    z[0]);
                        } else if (omega > 0.0) {
                            // a call pays for high rates
                            price +=
                                Gaussian1dModel::gaussianShiftedPolynomialIntegral(
                                    0.0,
                                    payoff.cCoefficients()[z.size() - 2],
                                    payoff.bCoefficients()[z.size() - 2],
                                    payoff.aCoefficients()[z.size() - 2],
                                    p[z.size() - 2], z[z.size() - 2],
                                    z[z.size() - 1], 100.0);
                        } else {
                            // a put pays for low rates
                            price +=
                                Gaussian1dModel::gaussianShiftedPolynomialIntegral(
                                    0.0, payoff.cCoefficients()[0],
                                    payoff.bCoefficients()[0],
                                    payoff.aCoefficients()[0], p[0], z[0],
                                    -100.0, z[0]);
                        }
                    }
                    return price * model_->numeraire(0.0, 0.0, discountCurve_);
                };

                if (type == CapFloor::Cap || type == CapFloor::Collar)
                    values[i] = optionlet(arguments_.capRates[i], sign) * f;
                if (type == CapFloor::Floor || type == CapFloor::Collar) {
                    Real floorlet = optionlet(arguments_.floorRates[i], -sign) * f;
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
