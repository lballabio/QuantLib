/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006, 2007 Mark Joshi

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

/*! \file lmmdriftcalculator.hpp
    \brief Drift computation for Libor market model
*/

#ifndef quantlib_lmm_drift_calculator_hpp
#define quantlib_lmm_drift_calculator_hpp

#include <ql/math/matrix.hpp>
#include <vector>

namespace QuantLib {

    class LMMCurveState;

    //! Drift computation for log-normal %Libor market models
    /*! Returns the drift \f$ \mu \Delta t \f$.
        See Mark Joshi, <i>Rapid Computation of Drifts in a
        Reduced Factor Libor Market Model</i>, Wilmott Magazine,
        May 2003.
    */
    class LMMDriftCalculator {
      public:
        LMMDriftCalculator(const Matrix& pseudo,
                           const std::vector<Spread>& displacements,
                           const std::vector<Time>& taus,
                           Size numeraire,
                           Size alive);
        //! Computes the drifts
        void compute(const LMMCurveState& cs,
                     std::vector<Real>& drifts) const;
        void compute(const std::vector<Rate>& fwds,
                     std::vector<Real>& drifts) const;

        /*! Computes the drifts without factor reduction as in
            eqs. 2, 4 of ref. [1] (uses the covariance matrix directly). */
        void computePlain(const LMMCurveState& cs,
                          std::vector<Real>& drifts) const;
        void computePlain(const std::vector<Rate>& fwds,
                          std::vector<Real>& drifts) const;

        /*! Computes the drifts with factor reduction as in eq. 7 of ref. [1]
            (uses pseudo square root of the covariance matrix). */
        void computeReduced(const LMMCurveState& cs,
                            std::vector<Real>& drifts) const;
        void computeReduced(const std::vector<Rate>& fwds,
                            std::vector<Real>& drifts) const;

      private:
        Size numberOfRates_, numberOfFactors_;
        bool isFullFactor_;
        Size numeraire_, alive_;
        std::vector<Spread> displacements_;
        std::vector<Real> oneOverTaus_;
        Matrix C_, pseudo_;
        // temporary variables to be added later
        mutable std::vector<Real> tmp_;
        mutable Matrix e_;
        std::vector<Size> downs_, ups_;
    };

}

#endif
