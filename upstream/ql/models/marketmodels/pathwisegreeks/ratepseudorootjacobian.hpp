/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*

 Copyright (C) 2008 Mark Joshi

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


#ifndef quantlib_rate_pseudo_root_hpp
#define quantlib_rate_pseudo_root_hpp

// to be removed using forward declaration


#include <ql/models/marketmodels/driftcomputation/lmmdriftcalculator.hpp>

/*! Classes for computing derivative of the map taking rates one step 
to the next with respect to a change in the pseudo-root. We do it both
numerically and analytically to provide an easy test of the analytic method.
This is useful for pathwise vegas.

Evolution is log Euler. 

One is tested against the other in MarketModelTest::testPathwiseVegas
*/

namespace QuantLib
{

    class RatePseudoRootJacobianNumerical
    {
    public:
        RatePseudoRootJacobianNumerical(const Matrix& pseudoRoot,
                                        Size aliveIndex, 
                                        Size numeraire,
                                        const std::vector<Time>& taus,
                                        const std::vector<Matrix>& pseudoBumps,
                                        const std::vector<Spread>& displacements);

        void getBumps(const std::vector<Rate>& oldRates,
            const std::vector<Real>& oneStepDFs, // redundant info but saves time to pass in since will have been needed elsewhere
            const std::vector<Rate>& newRates,   // redundant info but saves time to pass in since will have been needed elsewhere
            const std::vector<Real>& gaussians,
            Matrix& B); // B as in page 95 of the GG paper, rows should be number rates long, one row for bump

    private:

        //! this data is always the same
        Matrix pseudoRoot_;
        Size aliveIndex_; 
        std::vector<Time> taus_;
        std::vector<Matrix> pseudoBumped_;
        std::vector<Spread> displacements_; 
        Size numberBumps_;
        std::vector<LMMDriftCalculator> driftsComputers_;
        Size factors_;

        //! workspace variables
        std::vector<Real> drifts_;
        std::vector<Real> bumpedRates_;
   
    };


    

    class RatePseudoRootJacobian
    {
    public:
      RatePseudoRootJacobian(const Matrix& pseudoRoot,
                             Size aliveIndex,
                             Size numeraire,
                             const std::vector<Time>& taus,
                             const std::vector<Matrix>& pseudoBumps,
                             std::vector<Spread> displacements);

      void getBumps(const std::vector<Rate>& oldRates,
                    const std::vector<Real>& oneStepDFs, // redundant info but saves time to pass in
                                                         // since will have been needed elsewhere
                    const std::vector<Rate>& newRates,   // redundant info but saves time to pass in
                                                         // since will have been needed elsewhere
                    const std::vector<Real>& gaussians,
                    Matrix& B); // B as in page 95 of the GG paper, rows should be number rates
                                // long, one row for each bump

    private:

        //! this data does not change after construction
        Matrix pseudoRoot_;
        Size aliveIndex_; 
        std::vector<Time> taus_;
        std::vector<Matrix> pseudoBumps_;
        std::vector<Spread> displacements_; 
        Size numberBumps_;
        Size factors_;

        //! workspace variables

        std::vector<Matrix> allDerivatives_; 
    //    std::vector<Real> bumpedRates_;
        Matrix e_;
        std::vector<Real> ratios_;
   
    };

    
    class RatePseudoRootJacobianAllElements
    {
    public:
      RatePseudoRootJacobianAllElements(const Matrix& pseudoRoot,
                                        Size aliveIndex,
                                        Size numeraire,
                                        const std::vector<Time>& taus,
                                        std::vector<Spread> displacements);

      void getBumps(const std::vector<Rate>& oldRates,
                    const std::vector<Real>& oneStepDFs, // redundant info but saves time to pass in
                                                         // since will have been needed elsewhere
                    const std::vector<Rate>& newRates,   // redundant info but saves time to pass in
                                                         // since will have been needed elsewhere
                    const std::vector<Real>& gaussians,
                    std::vector<Matrix>&
                        B); // one Matrix for each rate, the elements of the matrix are the
                            // derivatives of that rate with respect to each pseudo-root element

    private:

        //! this data does not change after construction
        Matrix pseudoRoot_;
        Size aliveIndex_; 
        std::vector<Time> taus_;
        std::vector<Matrix> pseudoBumps_;
        std::vector<Spread> displacements_; 
        Size factors_;

        //! workspace

        Matrix e_;
        std::vector<Real> ratios_;
   
    };

}

#endif
