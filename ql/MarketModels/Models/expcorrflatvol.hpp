/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Ferdinando Ametrano
 Copyright (C) 2006 Chiara Fornarola
 Copyright (C) 2006 StatPro Italia srl
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

#include <ql/MarketModels/marketmodel.hpp>
#include <ql/MarketModels/evolutiondescription.hpp>
#include <ql/types.hpp>
#include <ql/Math/matrix.hpp>
#include <vector>

namespace QuantLib
{
    class ExpCorrFlatVol : public MarketModel {
      public:
        ExpCorrFlatVol(
            const Real longTermCorr,
            const Real beta,
            const std::vector<Volatility>& volatilities,
            const EvolutionDescription& evolution,
            const Size numberOfFactors,
            const std::vector<Rate>& initialRates,
            const std::vector<Spread>& displacements);
        //! \name MarketModel interface
        //@{
        const std::vector<Rate>& initialRates() const;
        const std::vector<Spread>& displacements() const;
        const EvolutionDescription& evolution() const;
        Size numberOfRates() const;
        Size numberOfFactors() const;
        Size numberOfSteps() const;
        const Matrix& pseudoRoot(Size i) const;
        const Matrix& covariance(Size i) const;
        const Matrix& totalCovariance(Size endIndex) const;
        //@}
      private:
        Size numberOfFactors_, numberOfRates_, numberOfSteps_;
        std::vector<Rate> initialRates_;
        std::vector<Spread> displacements_;
        EvolutionDescription evolution_;
        std::vector<Matrix> pseudoRoots_, covariance_, totalCovariance_;
    };

    // inline

    inline const std::vector<Rate>& ExpCorrFlatVol::initialRates() const {
        return initialRates_;
    }

    inline const std::vector<Spread>& ExpCorrFlatVol::displacements() const {
        return displacements_;
    }

    inline const EvolutionDescription& ExpCorrFlatVol::evolution() const {
        return evolution_;
    }

    inline Size ExpCorrFlatVol::numberOfRates() const {
        return initialRates_.size();
    }

    inline Size ExpCorrFlatVol::numberOfFactors() const {
        return numberOfFactors_;
    }

    inline Size ExpCorrFlatVol::numberOfSteps() const {
        return numberOfSteps_;
    }

    inline const Matrix& ExpCorrFlatVol::pseudoRoot(Size i) const {
        return pseudoRoots_[i];
    }

    inline const Matrix& ExpCorrFlatVol::covariance(Size i) const {
        return covariance_[i];
    }

    inline const Matrix& ExpCorrFlatVol::totalCovariance(Size endIndex) const {
        return totalCovariance_[endIndex];
    }

}

#endif
