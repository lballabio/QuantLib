/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
Copyright (C) 2010 Adrian O' Neill

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

#include <ql/experimental/variancegamma/fftvanillaengine.hpp>
#include <ql/exercise.hpp>
#include <ql/termstructures/volatility/equityfx/blackconstantvol.hpp>
#include <complex>

namespace QuantLib {

    FFTVanillaEngine::FFTVanillaEngine(
        const ext::shared_ptr<GeneralizedBlackScholesProcess>& process, Real logStrikeSpacing)
        : FFTEngine(process, logStrikeSpacing)
    {
    }

    std::unique_ptr<FFTEngine> FFTVanillaEngine::clone() const
    {
        ext::shared_ptr<GeneralizedBlackScholesProcess> process =
            ext::dynamic_pointer_cast<GeneralizedBlackScholesProcess>(process_);
        return std::unique_ptr<FFTEngine>(new FFTVanillaEngine(process, lambda_));
    }

    void FFTVanillaEngine::precalculateExpiry(Date d)
    {
        ext::shared_ptr<GeneralizedBlackScholesProcess> process =
            ext::dynamic_pointer_cast<GeneralizedBlackScholesProcess>(process_);

        dividendDiscount_ =
            process->dividendYield()->discount(d);
        riskFreeDiscount_ =
            process->riskFreeRate()->discount(d);

        DayCounter rfdc  = process->riskFreeRate()->dayCounter();
        t_ = rfdc.yearFraction(process->riskFreeRate()->referenceDate(), d);

        ext::shared_ptr<BlackConstantVol> constVol = ext::dynamic_pointer_cast<BlackConstantVol>
            (*(process->blackVolatility()));
        QL_REQUIRE(constVol, "Constant volatility required");
        Real vol = constVol->blackVol(0.0, 0.0);
        var_ = vol*vol;
    }

    std::complex<Real> FFTVanillaEngine::complexFourierTransform(std::complex<Real> u) const
    {
        std::complex<Real> i1(0, 1);

        Real s = process_->x0();

        std::complex<Real> phi = std::exp(i1 * u * (std::log(s) - (var_ * t_) / 2.0) 
            - (var_ * u * u * t_) / 2.0); 
        phi = phi * std::pow(dividendDiscount_/ riskFreeDiscount_, i1 * u);
        return phi;
    }

    Real FFTVanillaEngine::discountFactor(Date d) const
    {
        ext::shared_ptr<GeneralizedBlackScholesProcess> process =
            ext::dynamic_pointer_cast<GeneralizedBlackScholesProcess>(process_);
        return process->riskFreeRate()->discount(d);
    }

    Real FFTVanillaEngine::dividendYield(Date d) const
    {
        ext::shared_ptr<GeneralizedBlackScholesProcess> process =
            ext::dynamic_pointer_cast<GeneralizedBlackScholesProcess>(process_);
        return process->dividendYield()->discount(d);
    }

}
