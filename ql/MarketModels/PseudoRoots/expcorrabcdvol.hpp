/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Ferdinando Ametrano
 Copyright (C) 2006 Mark Joshi
 Copyright (C) 2005, 2006 Klaus Spanderen

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


#ifndef quantlib_exp_corr_abcd_vol_hpp
#define quantlib_exp_corr_abcd_vol_hpp

#include <ql/MarketModels/PseudoRoots/abcd.hpp>
#include <ql/MarketModels/pseudoroot.hpp>
#include <ql/MarketModels/evolutiondescription.hpp>
#include <ql/types.hpp>
#include <ql/Math/matrix.hpp>
#include <vector>

namespace QuantLib
{
    class ExpCorrAbcdVol : public PseudoRoot {
      public:
        ExpCorrAbcdVol(
            Real a,
            Real b,
            Real c,
            Real d,
            const std::vector<Real>& ks,
            Real longTermCorr,
            Real beta,
            const EvolutionDescription& evolution,
            const Size numberOfFactors,
            const std::vector<Rate>& initialRates,
            const std::vector<Rate>& displacements);
        const std::vector<Rate>& initialRates() const ;// passed by the calibrator
        const std::vector<Rate>& displacements() const ;// passed by the calibrator
        Size numberOfRates() const ;// n
        Size numberOfFactors() const ;//F, A rank
        // number of steps method?
        const Matrix& pseudoRoot(Size i) const ;
      private:
        Real a_, b_, c_, d_;
        std::vector<Real> ks_;
        Real longTermCorr_;
        Real beta_;
        std::vector<Time> rateTimes_, evolutionTimes_;
        Size numberOfFactors_;
        std::vector<Rate> initialRates_;
        std::vector<Rate> displacements_;
        std::vector<Matrix> pseudoRoots_;

    };

    // inline

    inline const std::vector<Rate>& ExpCorrAbcdVol::initialRates() const {
        return initialRates_;
    }

    inline const std::vector<Rate>& ExpCorrAbcdVol::displacements() const {
        return displacements_;
    }

    inline Size ExpCorrAbcdVol::numberOfRates() const {
        return initialRates_.size();
    }

    inline Size ExpCorrAbcdVol::numberOfFactors() const {
        return numberOfFactors_;
    }

    inline const Matrix& ExpCorrAbcdVol::pseudoRoot(Size i) const {
        return pseudoRoots_[i];
    }

}

#endif
