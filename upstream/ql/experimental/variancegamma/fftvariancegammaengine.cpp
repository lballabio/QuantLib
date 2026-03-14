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

#include <ql/experimental/variancegamma/fftvariancegammaengine.hpp>
#include <ql/exercise.hpp>

namespace QuantLib {

    FFTVarianceGammaEngine::FFTVarianceGammaEngine(
        const ext::shared_ptr<VarianceGammaProcess>& process, Real logStrikeSpacing)
        : FFTEngine(process, logStrikeSpacing)
    {
    }

    std::unique_ptr<FFTEngine> FFTVarianceGammaEngine::clone() const
    {
        ext::shared_ptr<VarianceGammaProcess> process =
            ext::dynamic_pointer_cast<VarianceGammaProcess>(process_);
        return std::unique_ptr<FFTEngine>(new FFTVarianceGammaEngine(process, lambda_));
    }

    void FFTVarianceGammaEngine::precalculateExpiry(Date d)
    {
        ext::shared_ptr<VarianceGammaProcess> process =
            ext::dynamic_pointer_cast<VarianceGammaProcess>(process_);

        dividendDiscount_ =
            process->dividendYield()->discount(d);
        riskFreeDiscount_ =
            process->riskFreeRate()->discount(d);

        DayCounter rfdc  = process->riskFreeRate()->dayCounter();
        t_ = rfdc.yearFraction(process->riskFreeRate()->referenceDate(), d);

        sigma_ = process->sigma();
        nu_ = process->nu();
        theta_ = process->theta();
    }

    std::complex<Real> FFTVarianceGammaEngine::complexFourierTransform(std::complex<Real> u) const
    {
        Real s = process_->x0();

        std::complex<Real> i1(0, 1);

        Real omega = std::log(1.0 - theta_ * nu_ - sigma_*sigma_ * nu_ / 2.0) / nu_;
        std::complex<Real> phi = std::exp(i1 * u * (std::log(s) + omega * t_)) 
            * std::pow(dividendDiscount_/ riskFreeDiscount_, i1 * u);
        phi = phi * (std::pow((1.0 - i1 * theta_ * nu_ * u + sigma_*sigma_ * nu_ * u*u / 2.0), (-t_ / nu_)));

        return phi;
    }

    Real FFTVarianceGammaEngine::discountFactor(Date d) const
    {
        ext::shared_ptr<VarianceGammaProcess> process =
            ext::dynamic_pointer_cast<VarianceGammaProcess>(process_);
        return process->riskFreeRate()->discount(d);
    }

    Real FFTVarianceGammaEngine::dividendYield(Date d) const
    {
        ext::shared_ptr<VarianceGammaProcess> process =
            ext::dynamic_pointer_cast<VarianceGammaProcess>(process_);
        return process->dividendYield()->discount(d);
    }

}
