/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2013, 2015 Peter Caspers

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

#include <ql/pricingengines/swaption/basketgeneratingengine.hpp>
#include <ql/rebatedexercise.hpp>
#include <ql/math/optimization/levenbergmarquardt.hpp>
#include <ql/math/optimization/simplex.hpp>
#include <ql/models/shortrate/calibrationhelpers/swaptionhelper.hpp>
#include <ql/termstructures/volatility/swaption/swaptionvolcube.hpp>
#include <ql/quotes/simplequote.hpp>
#include <cmath>

using std::exp;
using std::fabs;

namespace QuantLib {

    std::vector<ext::shared_ptr<BlackCalibrationHelper>>
    BasketGeneratingEngine::calibrationBasket(
        const ext::shared_ptr<Exercise>& exercise,
        const ext::shared_ptr<SwapIndex>& standardSwapBase,
        const ext::shared_ptr<SwaptionVolatilityStructure>& swaptionVolatility,
        const CalibrationBasketType basketType) const {

        QL_REQUIRE(
            !standardSwapBase->forwardingTermStructure().empty(),
            "standard swap base forwarding term structure must not be empty.");
        QL_REQUIRE(
            !standardSwapBase->exogenousDiscount() ||
                !standardSwapBase->discountingTermStructure().empty(),
            "standard swap base discounting term structure must not be empty.");

        std::vector<ext::shared_ptr<BlackCalibrationHelper> > result;

        Date today = Settings::instance().evaluationDate();
        Size minIdxAlive = static_cast<Size>(
            std::upper_bound(exercise->dates().begin(), exercise->dates().end(),
                             today) -
            exercise->dates().begin());

        ext::shared_ptr<RebatedExercise> rebEx =
            ext::dynamic_pointer_cast<RebatedExercise>(exercise);

        for (Size i = minIdxAlive; i < exercise->dates().size(); i++) {

            Date expiry = exercise->date(i);
            Real rebate = 0.0;
            Date rebateDate = expiry;
            if (rebEx != nullptr) {
                rebate = rebEx->rebate(i);
                rebateDate = rebEx->rebatePaymentDate(i);
            }

            ext::shared_ptr<SwaptionHelper> helper;

            switch (basketType) {

            case Naive: {
                Real swapLength = swaptionVolatility->dayCounter().yearFraction(
                    standardSwapBase->valueDate(expiry), underlyingLastDate());
                ext::shared_ptr<SmileSection> sec =
                    swaptionVolatility->smileSection(
                        expiry,
                        static_cast<Size>(std::lround(swapLength * 12.0)) * Months,
                        true);
                Real atmStrike = sec->atmLevel();
                Real atmVol;
                if (atmStrike == Null<Real>())
                    atmVol = sec->volatility(0.03);
                else
                    atmVol = sec->volatility(atmStrike);
                Real shift = sec->shift();

                helper = ext::make_shared<SwaptionHelper>(
                    expiry, underlyingLastDate(),
                    Handle<Quote>(ext::make_shared<SimpleQuote>(atmVol)),
                    standardSwapBase->iborIndex(),
                    standardSwapBase->fixedLegTenor(),
                    standardSwapBase->dayCounter(),
                    standardSwapBase->iborIndex()->dayCounter(),
                    standardSwapBase->exogenousDiscount()
                        ? standardSwapBase->discountingTermStructure()
                        : standardSwapBase->forwardingTermStructure(),
                    BlackCalibrationHelper::RelativePriceError, Null<Real>(), 1.0,
                    swaptionVolatility->volatilityType() ,shift);

                break;
            }

            case MaturityStrikeByDeltaGamma: {

                // determine the npv, first and second order derivatives at
                // $y=0$ of the underlying swap

                const Real h = 0.0001; // finite difference step in $y$, make
                                       // this a parameter of the engine ?
                Real zSpreadDsc =
                    oas_.empty() ? Real(1.0)
                                 : exp(-oas_->value() *
                                       onefactormodel_->termStructure()
                                           ->dayCounter()
                                           .yearFraction(expiry, rebateDate));

                Real npvm = underlyingNpv(expiry, -h) +
                            rebate *
                                onefactormodel_->zerobond(rebateDate, expiry,
                                                          -h, discountCurve_) *
                                zSpreadDsc;
                Real npv = underlyingNpv(expiry, 0.0) +
                           rebate * onefactormodel_->zerobond(
                                        rebateDate, expiry, 0, discountCurve_) *
                               zSpreadDsc;
                Real npvp = underlyingNpv(expiry, h) +
                            rebate *
                                onefactormodel_->zerobond(rebateDate, expiry, h,
                                                          discountCurve_) *
                                zSpreadDsc;

                Real delta = (npvp - npvm) / (2.0 * h);
                Real gamma = (npvp - 2.0 * npv + npvm) / (h * h);

                QL_REQUIRE(npv * npv + delta * delta + gamma * gamma > 0.0,
                           "(npv,delta,gamma) must have a positive norm");

                // debug output
                // std::cout << "EXOTIC npv " << npv << " delta " << delta
                //           << " gamma " << gamma << std::endl;
                // Real xtmp = -5.0;
                // std::cout
                //     << "********************************************EXERCISE "
                //     << expiry << " ******************" << std::endl;
                // std::cout << "globalExoticNpv;";
                // while (xtmp <= 5.0 + QL_EPSILON) {
                //     std::cout << underlyingNpv(expiry, xtmp) << ";";
                //     xtmp += 0.1;
                // }
                // std::cout << std::endl;
                // end debug output

                // play safe, we restrict the maximum maturity so to easily fit
                // in the date class restriction
                Real maxMaturity =
                    swaptionVolatility->dayCounter().yearFraction(
                        expiry, Date::maxDate() - 365);

                ext::shared_ptr<MatchHelper> matchHelper_;
                matchHelper_ = ext::make_shared<MatchHelper>(
                    underlyingType(), npv, delta, gamma, *onefactormodel_,
                    standardSwapBase, expiry, maxMaturity, h);

                // Optimize
                Array initial = initialGuess(expiry);
                QL_REQUIRE(initial.size() == 3,
                           "initial guess must have size 3 (but is "
                               << initial.size() << ")");

                EndCriteria ec(1000, 200, 1E-8, 1E-8, 1E-8); // make these
                                                             // criteria and the
                                                             // optimizer itself
                                                             // parameters of
                                                             // the method ?
                Constraint constraint = NoConstraint();
                Problem p(*matchHelper_, constraint, initial);
                LevenbergMarquardt lm;

                EndCriteria::Type ret = lm.minimize(p, ec);
                QL_REQUIRE(ret != EndCriteria::None &&
                               ret != EndCriteria::Unknown &&
                               ret != EndCriteria::MaxIterations,
                           "optimizer returns error (" << ret << ")");
                Array solution = p.currentValue();

                Real maturity = fabs(solution[1]);

                Size years = (Size)std::floor(maturity);
                maturity -= (Real)years;
                maturity *= 12.0;
                Size months = (Size)std::floor(maturity + 0.5);
                if (years == 0 && months == 0)
                    months = 1; // ensure a maturity of at least one months
                // maturity -= (Real)months; maturity *= 365.25;
                // Size days = (Size)std::floor(maturity);

                Period matPeriod =
                    years * Years + months * Months; //+days*Days;

                ext::shared_ptr<SmileSection> sec =
                    swaptionVolatility->smileSection(expiry, matPeriod, true);
                Real shift = sec->shift();

                // we have to floor the strike of the calibration instrument,
                // see warning in the header
                solution[2] = std::max(
                    solution[2], 0.00001 - shift); // floor at 0.1bp - shift

                // also the calibrated nominal may be zero, so we floor it, too
                solution[0] =
                    std::max(solution[0], 0.000001); // float at 0.01bp

                Real vol = sec->volatility(solution[2]);

                helper = ext::make_shared<SwaptionHelper>(
                    expiry, matPeriod,
                    Handle<Quote>(ext::make_shared<SimpleQuote>(
                                      vol)),
                    standardSwapBase->iborIndex(),
                    standardSwapBase->fixedLegTenor(),
                    standardSwapBase->dayCounter(),
                    standardSwapBase->iborIndex()->dayCounter(),
                    standardSwapBase->exogenousDiscount()
                        ? standardSwapBase->discountingTermStructure()
                        : standardSwapBase->forwardingTermStructure(),
                    BlackCalibrationHelper::RelativePriceError, solution[2],
                    fabs(solution[0]), swaptionVolatility->volatilityType(), shift);
                break;
            }

            default:
                QL_FAIL("Calibration basket type not known (" << basketType
                                                              << ")");
            }

            result.push_back(helper);
        }

        return result;
    }
}
