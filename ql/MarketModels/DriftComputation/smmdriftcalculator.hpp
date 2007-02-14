/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Mark Joshi

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

/*! \file driftcalculator.hpp
    \brief Drift computation for Coterminal Swap Market Model
*/

#ifndef quantlib_smm_drift_calculator_hpp
#define quantlib_smm_drift_calculator_hpp

#include <ql/Math/matrix.hpp>
#include <ql/MarketModels/CurveStates/coterminalswapcurvestate.hpp>
#include <vector>

namespace QuantLib {
    class CurveState;
    //! Drift computation for Coterminal Swap Market Models
    /*! \ingroup Market Models */
    class SMMDriftCalculator {
      public:
        /*! Returns the drift \f$ \mu \Delta t \f$.
            See Mark Joshi, Lorenzo Liesch, "\it {Effective
            Implementation Of Generic Market Models}".
        */
        SMMDriftCalculator(const Matrix& pseudo,
                           const std::vector<Spread>& displacements,
                           const std::vector<Time>& taus,
                           Size numeraire,
                           Size alive);
        //! Computes the drifts
        void compute(const CoterminalSwapCurveState& cs,
                     std::vector<Real>& drifts) const;
      private:
        Size nRates_, nFactors_;
        bool isFullFactor_;
        Size numeraire_, alive_;
        std::vector<Spread> displacements_;
        std::vector<Real> oneOverTaus_;
        Matrix C_, pseudo_;
        // temporary variables to be added later
        mutable std::vector<Real> tmp_;
        mutable Matrix wkaj_;  // < W(k) | A(j)/P(n) >
        mutable Matrix wkpj_; // < W(k) | P(j)/P(n) >
        mutable Matrix wkajshifted_;
        //std::vector<Size> downs_, ups_;
    };

}

#endif
