/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Ferdinando Ametrano
 Copyright (C) 2007 François du Vignaud
 Copyright (C) 2007 Mark Joshi

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

/*! \file cmsmmdriftcalculator.hpp
    \brief Drift computation for CMS market model
*/

#ifndef quantlib_cms_drift_calculator_hpp
#define quantlib_cms_drift_calculator_hpp

#include <ql/math/matrix.hpp>
#include <vector>

namespace QuantLib {

    class CMSwapCurveState;

    //! Drift computation for CMS market models
    /*! Returns the drift \f$ \mu \Delta t \f$.
        See Mark Joshi, <i>Rapid Computation of Drifts in a
        Reduced Factor Libor Market Model</i>, Wilmott Magazine,
        May 2003.
    */
    class CMSMMDriftCalculator {
      public:
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
        Size numberOfRates_, numberOfFactors_;
        Size numeraire_, alive_;
        std::vector<Spread> displacements_;
        std::vector<Real> oneOverTaus_;
        Matrix C_, pseudo_;
        // temporary variables to be added later
        mutable std::vector<Real> tmp_;
        mutable Matrix PjPnWk_; // < Wk, P_{j}/P_n> (k, j)
        mutable Matrix wkaj_;    // < Wk , Aj/Pn> (k, j)
        mutable Matrix wkajN_;    // < Wk , Aj/PN> (k, j)

        std::vector<Size> downs_, ups_;
        Size spanningFwds_;
    };

}

#endif
