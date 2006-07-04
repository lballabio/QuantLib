/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Katiuscia Manzoni, Chiara Fornarola & Nicola Jean


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


#ifndef quantlib_exponential_correlation_hpp
#define quantlib_exponential_correlation_hpp

#include <ql/MarketModels/pseudoroot.hpp>

namespace QuantLib {

    class ExponentialCorrelation: public PseudoRoot{
    public:
        ExponentialCorrelation(
            Real asyntothicCorr,
            Real beta,
            const std::vector<Volatility>& volatilities,
            const std::vector<Time>& tenors,
            const std::vector<Time>& evolutionTimes,
            const Size numberOfFactors,
            const Array& initialRates,
            const Array& displacements,
            )
        const Array& initialRates() const ;// passed by the calibrator
        const Array& displacements() const ;// passed by the calibrator
        Size numberOfRates() const ;// n 
        Size numberOfFactors() const ;//F, A rank 
        const Matrix& pseudoRoot(Size i) const;
    private:
      
       Real asyntothicCorr_;
       Real beta_;
       std::vector<Volatility> volatilities_;
       std::vector<Time> tenors_;
       std::vector<Time> evolutionTimes_;
       Size numberOfFactors_;
       Array initialRates_;
       Array displacements_;
       Matrix covariance_;
       std::vector<Matrix> pseudoRoots_;
    }
}
#endif