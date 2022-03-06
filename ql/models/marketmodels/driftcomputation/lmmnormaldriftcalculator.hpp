/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Giorgio Facchinetti
 Copyright (C) 2007 Chiara Fornarola

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

/*! \file lmmnormaldriftcalculator.hpp
    \brief Drift computation for normal Libor market model
*/

#ifndef quantlib_lmm_normal_drift_calculator_hpp
#define quantlib_lmm_normal_drift_calculator_hpp

#include <ql/math/matrix.hpp>
#include <ql/models/marketmodels/curvestates/lmmcurvestate.hpp>
#include <vector>

namespace QuantLib {

    //! Drift computation for normal %Libor market models
    /*! Returns the drift \f$ \mu \Delta t \f$.
        See Mark Joshi, <i>Rapid Computation of Drifts in a
        Reduced Factor Libor Market Model</i>, Wilmott Magazine,
        May 2003.
    */
    class LMMNormalDriftCalculator {
      public:
        LMMNormalDriftCalculator(const Matrix& pseudo,
                                 const std::vector<Time>& taus,
                                 Size numeraire,
                                 Size alive);
        //! Computes the drifts
        void compute(const LMMCurveState& cs,
                     std::vector<Real>& drifts) const;
        void compute(const std::vector<Rate>& fwds,
                     std::vector<Real>& drifts) const;

        /*! Computes the drifts without factor reduction as in
            eqs. 2, 4 of ref. [1], modified for normal forward rates dynamic
            (uses the covariance matrix directly). */
        void computePlain(const LMMCurveState& cs,
                          std::vector<Real>& drifts) const;
        void computePlain(const std::vector<Rate>& fwds,
                          std::vector<Real>& drifts) const;

        /*! Computes the drifts with factor reduction as in
            eq. 7 of ref. [1], modified for normal forward rates dynamic
            (uses pseudo square root of the covariance matrix). */
        void computeReduced(const LMMCurveState& cs,
                            std::vector<Real>& drifts) const;
        void computeReduced(const std::vector<Rate>& fwds,
                            std::vector<Real>& drifts) const;


      private:
        Size numberOfRates_, numberOfFactors_;
        bool isFullFactor_;
        Size numeraire_, alive_;
        std::vector<Real> oneOverTaus_;
        Matrix C_, pseudo_;
        // temporary variables to be added later
        mutable std::vector<Real> tmp_;
        mutable Matrix e_;
        std::vector<Size> downs_, ups_;
    };

}

#endif


#ifndef id_a69f21dd5e9059a83e919afd95cf8287
#define id_a69f21dd5e9059a83e919afd95cf8287
inline bool test_a69f21dd5e9059a83e919afd95cf8287(int* i) { return i != 0; }
#endif
