/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*

 Copyright (C) 2008 Mark Joshi

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


#ifndef quantlib_pathwise_accounting_engine_hpp
#define quantlib_pathwise_accounting_engine_hpp

#include <ql/models/marketmodels/pathwisemultiproduct.hpp>
#include <ql/models/marketmodels/pathwisediscounter.hpp>
#include <ql/math/statistics/sequencestatistics.hpp>
#include <ql/models/marketmodels/pathwisegreeks/ratepseudorootjacobian.hpp>

#include <ql/utilities/clone.hpp>
#include <ql/types.hpp>
#include <vector>

namespace QuantLib {

    class LogNormalFwdRateEuler;
    class MarketModel;


    //! Engine collecting cash flows along a market-model simulation for doing pathwise computation of Deltas
    // using Giles--Glasserman smoking adjoints method
    // note only works with displaced LMM, and requires knowledge of pseudo-roots and displacements 
    // This is tested in MarketModelTest::testPathwiseGreeks
    class PathwiseAccountingEngine 
    {
      public:
        PathwiseAccountingEngine(
            std::shared_ptr<LogNormalFwdRateEuler> evolver, // method relies heavily on LMM Euler
            const Clone<MarketModelPathwiseMultiProduct>& product,
            std::shared_ptr<MarketModel>
                pseudoRootStructure, // we need pseudo-roots and displacements
            Real initialNumeraireValue);

        void multiplePathValues(SequenceStatisticsInc& stats,
                                Size numberOfPaths);
      private:
          Real singlePathValues(std::vector<Real>& values);

        std::shared_ptr<LogNormalFwdRateEuler> evolver_;
        Clone<MarketModelPathwiseMultiProduct> product_;
        std::shared_ptr<MarketModel> pseudoRootStructure_;

        Real initialNumeraireValue_;
        Size numberProducts_;
        Size numberRates_;
        Size numberCashFlowTimes_;
        Size numberSteps_;

        std::vector<Real> currentForwards_, lastForwards_;

        bool doDeflation_;


        // workspace
        std::vector<Real> numerairesHeld_;
        std::vector<Size> numberCashFlowsThisStep_;
        std::vector<std::vector<MarketModelPathwiseMultiProduct::CashFlow> >
                                                         cashFlowsGenerated_;
        std::vector<MarketModelPathwiseDiscounter> discounters_;

        std::vector<Matrix> V_;  // one V for each product, with components for each time step and rate

   //     std::vector<std::vector<std::vector<Real> > > V_; // one V for each product, with components for each time step and rate

        Matrix LIBORRatios_; // dimensions are step and rate number
        Matrix Discounts_; // dimensions are step and rate number, goes from 0 to n. P(t_0, t_j)

        Matrix StepsDiscountsSquared_; // dimensions are step and rate number

        Matrix LIBORRates_; // dimensions are step and rate number
        Matrix partials_; // dimensions are factor and rate

        std::vector<Real> deflatorAndDerivatives_;
        
        std::vector<std::vector<Size> > numberCashFlowsThisIndex_;
        std::vector<Matrix> totalCashFlowsThisIndex_; // need product cross times cross which sensitivity

        std::vector<std::vector<Size> > cashFlowIndicesThisStep_;

    };


   //! Engine collecting cash flows along a market-model simulation for doing pathwise computation of Deltas and vegas
    // using Giles--Glasserman smoking adjoints method
    // note only works with displaced LMM, 
    // 
    // The method is intimately connected with log-normal Euler evolution 
    // 
    // We must work with discretely compounding MM account
    // To compute a vega means changing the pseudo-square root at each time step
    // So for each vega, we have a vector of matrices. So we need a vector of vectors of matrices to compute all the vegas.
    // We do the outermost vector by time step and inner one by which vega.
    // This is tested in MarketModelTest::testPathwiseVegas

    class PathwiseVegasAccountingEngine 
    {
      public:
        PathwiseVegasAccountingEngine(
            std::shared_ptr<LogNormalFwdRateEuler> evolver, // method relies heavily on LMM Euler
            const Clone<MarketModelPathwiseMultiProduct>& product,
            std::shared_ptr<MarketModel>
                pseudoRootStructure, // we need pseudo-roots and displacements
            const std::vector<std::vector<Matrix> >& VegaBumps,
            Real initialNumeraireValue);

        void multiplePathValues(std::vector<Real>& means,
                                std::vector<Real>& errors,
                                Size numberOfPaths);
      private:
          Real singlePathValues(std::vector<Real>& values);

        std::shared_ptr<LogNormalFwdRateEuler> evolver_;
        Clone<MarketModelPathwiseMultiProduct> product_;
        std::shared_ptr<MarketModel> pseudoRootStructure_;
        std::vector<Size> numeraires_;

        Real initialNumeraireValue_;
        Size numberProducts_;
        Size numberRates_;
        Size numberCashFlowTimes_;
        Size numberSteps_;
        Size numberBumps_;

        std::vector<RatePseudoRootJacobian> jacobianComputers_;

        
        bool doDeflation_;


        // workspace
        std::vector<Real> currentForwards_, lastForwards_;
        std::vector<Real> numerairesHeld_;
        std::vector<Size> numberCashFlowsThisStep_;
        std::vector<std::vector<MarketModelPathwiseMultiProduct::CashFlow> >
                                                         cashFlowsGenerated_;
        std::vector<MarketModelPathwiseDiscounter> discounters_;

        std::vector<Matrix> V_;  // one V for each product, with components for each time step and rate

        Matrix LIBORRatios_; // dimensions are step and rate number
        Matrix Discounts_; // dimensions are step and rate number, goes from 0 to n. P(t_0, t_j)

        Matrix StepsDiscountsSquared_; // dimensions are step and rate number
        std::vector<Real> stepsDiscounts_;

        Matrix LIBORRates_; // dimensions are step and rate number
        Matrix partials_; // dimensions are factor and rate

        Matrix vegasThisPath_; // dimensions are product and which vega
        std::vector<Matrix> jacobiansThisPaths_; // dimensions are step, rate and factor

        std::vector<Real> deflatorAndDerivatives_;
        std::vector<Real> fullDerivatives_;
        
        std::vector<std::vector<Size> > numberCashFlowsThisIndex_;
        std::vector<Matrix> totalCashFlowsThisIndex_; // need product cross times cross which sensitivity

        std::vector<std::vector<Size> > cashFlowIndicesThisStep_;

    };

   //! Engine collecting cash flows along a market-model simulation for doing pathwise computation of Deltas and vegas
    // using Giles--Glasserman smoking adjoints method
    // note only works with displaced LMM, 
    // 
    // The method is intimately connected with log-normal Euler evolution 
    // 
    // We must work with discretely compounding MM account
    // To compute a vega means changing the pseudo-square root at each time step
    // So for each vega, we have a vector of matrices. So we need a vector of vectors of matrices to compute all the vegas.
    // We do the outermost vector by time step and inner one by which vega.
    // This implementation is different in that all the linear combinations by the bumps are done as late as possible,
    // whereas PathwiseVegasAccountingEngine does them as early as possible. 
    // This is tested in MarketModelTest::testPathwiseVegas

    class PathwiseVegasOuterAccountingEngine 
    {
      public:
        PathwiseVegasOuterAccountingEngine(
            std::shared_ptr<LogNormalFwdRateEuler> evolver, // method relies heavily on LMM Euler
            const Clone<MarketModelPathwiseMultiProduct>& product,
            std::shared_ptr<MarketModel>
                pseudoRootStructure, // we need pseudo-roots and displacements
            const std::vector<std::vector<Matrix> >& VegaBumps,
            Real initialNumeraireValue);

        //! Use to get vegas with respect to VegaBumps
        void multiplePathValues(std::vector<Real>& means,
                                std::vector<Real>& errors,
                                Size numberOfPaths);

        //! Use to get vegas with respect to pseudo-root-elements
        void multiplePathValuesElementary(std::vector<Real>& means,
                                std::vector<Real>& errors,
                                Size numberOfPaths);

      private:
          Real singlePathValues(std::vector<Real>& values);

        std::shared_ptr<LogNormalFwdRateEuler> evolver_;
        Clone<MarketModelPathwiseMultiProduct> product_;
        std::shared_ptr<MarketModel> pseudoRootStructure_;
        std::vector<std::vector<Matrix> > vegaBumps_; 
        std::vector<Size> numeraires_;

        Real initialNumeraireValue_;
        Size numberProducts_;
        Size numberRates_;
        Size numberCashFlowTimes_;
        Size numberSteps_;
        Size factors_;
        Size numberBumps_;
        Size numberElementaryVegas_;

        std::vector<RatePseudoRootJacobianAllElements> jacobianComputers_;

        
        bool doDeflation_;


        // workspace
        std::vector<Real> currentForwards_, lastForwards_;
        std::vector<Real> numerairesHeld_;
        std::vector<Size> numberCashFlowsThisStep_;
        std::vector<std::vector<MarketModelPathwiseMultiProduct::CashFlow> >
                                                         cashFlowsGenerated_;
        std::vector<MarketModelPathwiseDiscounter> discounters_;

        std::vector<Matrix> V_;  // one V for each product, with components for each time step and rate

        Matrix LIBORRatios_; // dimensions are step and rate number
        Matrix Discounts_; // dimensions are step and rate number, goes from 0 to n. P(t_0, t_j)

        Matrix StepsDiscountsSquared_; // dimensions are step and rate number
        std::vector<Real> stepsDiscounts_;

        Matrix LIBORRates_; // dimensions are step and rate number
        Matrix partials_; // dimensions are factor and rate

        std::vector<std::vector<Matrix>   > elementary_vegas_ThisPath_;  // dimensions are product, step,  rate and factor
        std::vector<std::vector<Matrix> > jacobiansThisPaths_;                      // dimensions are step, rate, rate and factor

        std::vector<Real> deflatorAndDerivatives_;
        std::vector<Real> fullDerivatives_;
        
        std::vector<std::vector<Size> > numberCashFlowsThisIndex_;
        std::vector<Matrix> totalCashFlowsThisIndex_; // need product cross times cross which sensitivity

        std::vector<std::vector<Size> > cashFlowIndicesThisStep_;
/*
        // experimental

        std::vector<std::vector<Real> > gaussians_;
        int distinguishedFactor_;
        int distinguishedRate_;
        int  distinguishedStep_;

*/
    };

}

#endif
