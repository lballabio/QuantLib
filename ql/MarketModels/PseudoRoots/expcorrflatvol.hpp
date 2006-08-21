/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Ferdinando Ametrano
 Copyright (C) 2006 Chiara Fornarola
 Copyright (C) 2006 Nicola Jean
 Copyright (C) 2006 Katiuscia Manzoni


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


#ifndef quantlib_exp_corr_flat_vol_hpp
#define quantlib_exp_corr_flat_vol_hpp

#include <ql/MarketModels/pseudoroot.hpp>
#include <ql/MarketModels/evolutiondescription.hpp>
#include <ql/types.hpp>
#include <ql/Math/matrix.hpp>
#include <vector>

namespace QuantLib
{
    class ExpCorrFlatVol : public PseudoRoot {
      public:
        ExpCorrFlatVol(
            const Real longTermCorr,
            const Real beta,
            const std::vector<Volatility>& volatilities,
            const EvolutionDescription& evolution,
            const Size numberOfFactors,
            const std::vector<Rate>& initialRates,
            const std::vector<Rate>& displacements);
        //! \name PseudoRoot interface
        //@{
        const std::vector<Rate>& initialRates() const;
        const std::vector<Rate>& displacements() const;
        Size numberOfRates() const;
        Size numberOfFactors() const; 
        const Matrix& pseudoRoot(Size i) const;
        //@}
      private:
        Real longTermCorr_;
        Real beta_;
        std::vector<Volatility> volatilities_;
        std::vector<Time> rateTimes_, evolutionTimes_;
        Size numberOfFactors_;
        std::vector<Rate> initialRates_;
        std::vector<Rate> displacements_;
        std::vector<Matrix> pseudoRoots_;
    };

    // inline

    inline const std::vector<Rate>& ExpCorrFlatVol::initialRates() const {
        return initialRates_;
    }

    inline const std::vector<Rate>& ExpCorrFlatVol::displacements() const {
        return displacements_;
    }

    inline Size ExpCorrFlatVol::numberOfRates() const {
        return initialRates_.size();
    }

    inline Size ExpCorrFlatVol::numberOfFactors() const {
        return numberOfFactors_;
    }

    inline const Matrix& ExpCorrFlatVol::pseudoRoot(Size i) const {
        return pseudoRoots_[i];
    }

}

#endif
