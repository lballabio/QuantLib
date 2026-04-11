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

#include <ql/math/interpolations/linearinterpolation.hpp>
#include <ql/math/interpolations/loginterpolation.hpp>
#include <ql/math/interpolations/cubicinterpolation.hpp>
#include <ql/math/interpolations/backwardflatinterpolation.hpp>
#include <ql/math/interpolations/forwardflatinterpolation.hpp>
#include <ql/math/interpolations/lagrangeinterpolation.hpp>
#include <fuzzer/FuzzedDataProvider.h>
#include <algorithm>
#include <vector>

using namespace QuantLib;

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    FuzzedDataProvider fdp(data, size);

    try {
        // Consume a variable number of (x, y) data points from fuzz input.
        // Each point is 2 doubles = 16 bytes, so the fuzzer controls both
        // the number of points and their values.
        auto numPoints = fdp.ConsumeIntegralInRange<size_t>(3, 50);
        auto interpType = fdp.ConsumeIntegralInRange<int>(0, 5);

        std::vector<Real> x(numPoints), y(numPoints);

        // Consume x values and sort them (interpolation requires sorted x)
        for (size_t i = 0; i < numPoints; ++i)
            x[i] = fdp.ConsumeFloatingPointInRange<Real>(-1000.0, 1000.0);
        std::sort(x.begin(), x.end());

        // Ensure strictly increasing x values
        for (size_t i = 1; i < numPoints; ++i) {
            if (x[i] <= x[i - 1])
                x[i] = x[i - 1] + 0.01;
        }

        // Consume y values (unconstrained for most interpolations)
        for (size_t i = 0; i < numPoints; ++i)
            y[i] = fdp.ConsumeFloatingPointInRange<Real>(-1e6, 1e6);

        // For log interpolation, y must be positive
        if (interpType == 1) {
            for (size_t i = 0; i < numPoints; ++i)
                y[i] = std::abs(y[i]) + 0.001;
        }

        // Build the interpolation
        ext::shared_ptr<Interpolation> interp;
        switch (interpType) {
        case 0:
            interp = ext::make_shared<LinearInterpolation>(
                x.begin(), x.end(), y.begin());
            break;
        case 1:
            interp = ext::make_shared<LogLinearInterpolation>(
                x.begin(), x.end(), y.begin());
            break;
        case 2:
            interp = ext::make_shared<CubicNaturalSpline>(
                x.begin(), x.end(), y.begin());
            break;
        case 3:
            interp = ext::make_shared<BackwardFlatInterpolation>(
                x.begin(), x.end(), y.begin());
            break;
        case 4:
            interp = ext::make_shared<ForwardFlatInterpolation>(
                x.begin(), x.end(), y.begin());
            break;
        case 5:
            interp = ext::make_shared<LagrangeInterpolation>(
                x.begin(), x.end(), y.begin());
            break;
        }

        interp->enableExtrapolation();

        // Evaluate the interpolation at fuzz-controlled query points
        auto numQueries = fdp.ConsumeIntegralInRange<size_t>(1, 30);
        for (size_t i = 0; i < numQueries; ++i) {
            Real xq = fdp.ConsumeFloatingPointInRange<Real>(
                x.front() - 10.0, x.back() + 10.0);
            (void)(*interp)(xq);
            // Exercise derivative computation where supported
            if (interpType <= 2) {
                (void)interp->derivative(xq);
                (void)interp->secondDerivative(xq);
            }
            (void)interp->primitive(xq);
        }

    } catch (const std::exception&) {
    }
    return 0;
}
