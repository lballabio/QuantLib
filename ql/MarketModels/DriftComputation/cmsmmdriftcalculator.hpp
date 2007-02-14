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
    \brief Drift computation for Market Model
*/

#ifndef quantlib_cms_drift_calculator_hpp
#define quantlib_cms_drift_calculator_hpp

#include <ql/Math/matrix.hpp>
#include <ql/MarketModels/curvestate.hpp>
#include <ql/MarketModels/CurveStates/cmswapcurvestate.hpp>
#include <vector>

namespace QuantLib {

    //! Drift computation for Market Models
    /*! \ingroup Market Models */

    class CMSMMDriftCalculator {
      public:
        /*! Returns the drift \f$ \mu \Delta t \f$.
            See Mark Joshi, "\it {Rapid Computation of Drifts in a 
            Reduced Factor Libor Market Model}", Wilmott Magazine, 
            May 2003.
        */
        CMSMMDriftCalculator(const Matrix& pseudo,
                        const std::vector<Spread>& displacements,
                        const std::vector<Time>& taus,
                        Size numeraire,
                        Size alive,
                        Size spanningFwds);
        //! Computes the drifts
        void compute(const CMSwapCurveState& cs,
                     std::vector<Real>& drifts) const;
      private:
        Size dim_, factors_;
        bool isFullFactor_;
        Size numeraire_, alive_;
        std::vector<Spread> displacements_;
        std::vector<Real> oneOverTaus_;
        Matrix C_, pseudo_;
        // temporary variables to be added later
        mutable std::vector<Real> tmp_;
        mutable Matrix PjPnWk_; // < Wk, P_{j}/P_n> (k, j)
        mutable Matrix wkaj_;    // < Wk , Aj/Pn> (k, j)
        std::vector<Size> downs_, ups_;
        Size spanningFwds_;
    };

}

#endif
