/*
 Copyright (C) 2026 David Korczynski

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

#include <ql/pricingengines/blackformula.hpp>
#include <fuzzer/FuzzedDataProvider.h>

using namespace QuantLib;

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    FuzzedDataProvider fdp(data, size);

    // Generate constrained parameters that satisfy preconditions:
    // displacement >= 0, strike + displacement >= 0, forward + displacement > 0
    // stdDev >= 0, discount > 0
    auto type = fdp.PickValueInArray({Option::Put, Option::Call});
    Real displacement = fdp.ConsumeFloatingPointInRange<Real>(0.0, 100.0);
    Real strike = fdp.ConsumeFloatingPointInRange<Real>(-displacement, 1000.0);
    Real forward = fdp.ConsumeFloatingPointInRange<Real>(-displacement + 0.01, 1000.0);
    Real stdDev = fdp.ConsumeFloatingPointInRange<Real>(0.0, 10.0);
    Real discount = fdp.ConsumeFloatingPointInRange<Real>(0.01, 2.0);
    Real expiry = fdp.ConsumeFloatingPointInRange<Real>(0.01, 30.0);

    // Choose which function group to exercise (use remaining entropy)
    int funcGroup = fdp.ConsumeIntegralInRange<int>(0, 5);

    try {
        switch (funcGroup) {
        case 0: {
            // Black formula and its derivatives
            Real price = blackFormula(type, strike, forward, stdDev, discount, displacement);
            (void)price;
            (void)blackFormulaForwardDerivative(type, strike, forward, stdDev, discount, displacement);
            (void)blackFormulaCashItmProbability(type, strike, forward, stdDev, displacement);
            (void)blackFormulaAssetItmProbability(type, strike, forward, stdDev, displacement);
            (void)blackFormulaStdDevDerivative(strike, forward, stdDev, discount, displacement);
            (void)blackFormulaVolDerivative(strike, forward, stdDev, expiry, discount, displacement);
            if (stdDev > 0.01) {
                (void)blackFormulaStdDevSecondDerivative(strike, forward, stdDev, discount, displacement);
            }
            break;
        }
        case 1: {
            // Implied stddev approximations (Corrado-Miller)
            Real price = blackFormula(type, strike, forward, stdDev, discount, displacement);
            if (price > 0.0) {
                (void)blackFormulaImpliedStdDevApproximation(type, strike, forward, price, discount, displacement);
            }
            break;
        }
        case 2: {
            // Implied stddev approximation (Radoicic-Stefanica)
            Real price = blackFormula(type, strike, forward, stdDev, discount, displacement);
            if (price > 0.0) {
                (void)blackFormulaImpliedStdDevApproximationRS(type, strike, forward, price, discount, displacement);
            }
            break;
        }
        case 3: {
            // Implied stddev (Newton-safe solver)
            Real price = blackFormula(type, strike, forward, stdDev, discount, displacement);
            if (price > 0.0 && stdDev > 0.01) {
                (void)blackFormulaImpliedStdDev(type, strike, forward, price, discount, displacement);
            }
            break;
        }
        case 4: {
            // Chambers approximation
            Real price = blackFormula(type, strike, forward, stdDev, discount, displacement);
            Real atmStdDev = fdp.ConsumeFloatingPointInRange<Real>(0.01, 5.0);
            Real atmPrice = blackFormula(type, forward, forward, atmStdDev, discount, 0.0);
            if (price > 0.0 && atmPrice > 0.0) {
                (void)blackFormulaImpliedStdDevChambers(type, strike, forward, price, atmPrice, discount, displacement);
            }
            break;
        }
        case 5: {
            // Bachelier (normal) model formulas
            Real bachelierStdDev = fdp.ConsumeFloatingPointInRange<Real>(0.0, 500.0);
            (void)bachelierBlackFormula(type, strike, forward, bachelierStdDev, discount);
            (void)bachelierBlackFormulaForwardDerivative(type, strike, forward, bachelierStdDev, discount);
            (void)bachelierBlackFormulaStdDevDerivative(strike, forward, bachelierStdDev, discount);
            (void)bachelierBlackFormulaAssetItmProbability(type, strike, forward, bachelierStdDev);

            // Bachelier implied vol (Choi and Jaeckel)
            Real bachelierPrice = bachelierBlackFormula(type, strike, forward, bachelierStdDev, discount);
            if (bachelierPrice > 0.0 && expiry > 0.0) {
                (void)bachelierBlackFormulaImpliedVolChoi(type, strike, forward, expiry, bachelierPrice, discount);
                (void)bachelierBlackFormulaImpliedVol(type, strike, forward, expiry, bachelierPrice, discount);
            }
            break;
        }
        }
    } catch (const std::exception&) {
    }

    // Also exercise Li-RS implied stddev when we have enough entropy
    if (fdp.remaining_bytes() >= 8) {
        try {
            Real price = blackFormula(type, strike, forward, stdDev, discount, displacement);
            if (price > 0.0 && stdDev > 0.01) {
                Real omega = fdp.ConsumeFloatingPointInRange<Real>(0.5, 1.5);
                (void)blackFormulaImpliedStdDevLiRS(type, strike, forward, price, discount, displacement,
                                                    Null<Real>(), omega);
            }
        } catch (const std::exception&) {
        }
    }

    return 0;
}
