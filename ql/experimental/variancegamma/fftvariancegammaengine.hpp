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

/*! \file fftvariancegammaengine.hpp
    \brief \brief FFT engine for vanilla options under a Variance Gamma process
*/

#ifndef quantlib_fft_variancegamma_engine_hpp
#define quantlib_fft_variancegamma_engine_hpp

#include <ql/experimental/variancegamma/fftengine.hpp>
#include <ql/experimental/variancegamma/variancegammaprocess.hpp>

namespace QuantLib {

    //! FFT engine for vanilla options under a Variance Gamma process
    /*! \ingroup vanillaengines

        \test the correctness of the returned values is tested by
        comparison with known good values and the analytic approach
    */
    class FFTVarianceGammaEngine : public FFTEngine {
    public:
        explicit FFTVarianceGammaEngine(
            const ext::shared_ptr<VarianceGammaProcess>&process,
            Real logStrikeSpacing = 0.001);
        #if defined(QL_USE_STD_UNIQUE_PTR)
        std::unique_ptr<FFTEngine> clone() const override;
#else
        virtual std::auto_ptr<FFTEngine> clone() const;
        #endif
    protected:
      void precalculateExpiry(Date d) override;
      std::complex<Real> complexFourierTransform(std::complex<Real> u) const override;
      Real discountFactor(Date d) const override;
      Real dividendYield(Date d) const override;

    private:
        DiscountFactor dividendDiscount_;
        DiscountFactor riskFreeDiscount_;
        Time t_;
        Real sigma_;
        Real nu_;
        Real theta_;
    };

}


#endif



#ifndef id_f196bc1132e950795716ff324f9cf86d
#define id_f196bc1132e950795716ff324f9cf86d
inline bool test_f196bc1132e950795716ff324f9cf86d(int* i) { return i != 0; }
#endif
