/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
Copyright (C) 2010 Adrian O' Neill

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

/*! \file fftvanillaengine.hpp
    \brief FFT engine for vanilla options under a Black Scholes process
*/

#ifndef quantlib_fft_vanilla_engine_hpp
#define quantlib_fft_vanilla_engine_hpp

#include <ql/experimental/variancegamma/fftengine.hpp>
#include <ql/processes/blackscholesprocess.hpp>
#include <complex>

namespace QuantLib {

    //! FFT Pricing engine vanilla options under a Black Scholes process
    /*! \ingroup vanillaengines

        \test the correctness of the returned values is tested by
        comparison with Black Scholes pricing.
    */
    class FFTVanillaEngine : public FFTEngine {
    public:
        explicit FFTVanillaEngine(
            const ext::shared_ptr<GeneralizedBlackScholesProcess>&process,
            Real logStrikeSpacing = 0.001);
        std::unique_ptr<FFTEngine> clone() const override;
    protected:
        void precalculateExpiry(Date d) override;
        std::complex<Real> complexFourierTransform(std::complex<Real> u) const override;
        Real discountFactor(Date d) const override;
        Real dividendYield(Date d) const override;

    private:
        DiscountFactor dividendDiscount_;
        DiscountFactor riskFreeDiscount_;
        Time t_;
        Real var_;
    };

}


#endif

