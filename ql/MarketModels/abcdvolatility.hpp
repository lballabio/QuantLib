/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 


 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/


#ifndef quantlib_abcd_volatility_hpp
#define quantlib_abcd_volatility_hpp

#include <ql/MarketModels/pseudoroot.hpp>
#include <ql/types.hpp>
#include <ql/Math/matrix.hpp>
#include <vector>

namespace QuantLib
{
    class AbcdVolatility : public PseudoRoot {
    public:
        AbcdVolatility(
            Real a,
            Real b,
            Real c,
            Real d,
            const std::vector<Real>& ks,
            Real longTermCorr,
            Real beta,
            const Array& rateTimes,
            const Array& evolutionTimes,
            const Size numberOfFactors,
            const Array& initialRates,
            const Array& displacements);
        const Array& initialRates() const ;// passed by the calibrator
        const Array& displacements() const ;// passed by the calibrator
        Size numberOfRates() const ;// n 
        Size numberOfFactors() const ;//F, A rank 
        // number of steps method?
        const Matrix& pseudoRoot(Size i) const ;

    private:
        Real a_, b_, c_, d_;
        std::vector<Real> ks_;
        Real longTermCorr_;
        Real beta_;
        Array rateTimes_, evolutionTimes_;
        Size numberOfFactors_;
        Array initialRates_;
        Array displacements_;
        Matrix covariance_;
        std::vector<Matrix> pseudoRoots_;
    };
}

#endif
