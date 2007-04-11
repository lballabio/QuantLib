/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 François du Vignaud
 Copyright (C) 2007 Chiara Fornarola
 Copyright (C) 2007 Katiuscia Manzoni

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

#include <ql/marketmodels/models/swapfromfracorrelationstructure.hpp>
#include <ql/marketmodels/evolutiondescription.hpp>
#include <ql/utilities/disposable.hpp>
#include <ql/marketmodels/curvestate.hpp>
#include <ql/marketmodels/swapforwardmappings.hpp>
#include <ql/math/pseudosqrt.hpp>

namespace QuantLib {

    SwapFromFRACorrelationStructure::SwapFromFRACorrelationStructure(
            const Matrix& fraCorrelation,
            const CurveState& curveState,
            const EvolutionDescription& evolution,
            const Size numberOfFactors) :
    fraCorrelationMatrix_(evolution.numberOfRates()),
    pseudoRoots_(evolution.numberOfRates()),
    numberOfFactors_(numberOfFactors), evolution_(evolution) {

        Size nbRates = evolution.numberOfRates();
        QL_REQUIRE(nbRates==curveState.numberOfRates(),
                   "mismatch between number of rates in evolution (" << nbRates <<
                   ") and curveState (" << curveState.numberOfRates() << ")");
        QL_REQUIRE(nbRates==fraCorrelation.rows(),
                   "mismatch between number of rates (" << nbRates <<
                   ") and fraCorrelation rows (" << fraCorrelation.rows() << ")");
        QL_REQUIRE(nbRates==fraCorrelation.columns(),
                   "mismatch between number of rates (" << nbRates <<
                   ") and fraCorrelation columns (" << fraCorrelation.columns() << ")");
        QL_REQUIRE(numberOfFactors<=fraCorrelation.rows(),
                   "number of factors (" << numberOfFactors <<
                   ") must be less than correlation rows (" << fraCorrelation.rows() << ")");
        QL_REQUIRE(fraCorrelation.rows()==fraCorrelation.columns(),
            "correlation matrix is not square: " << fraCorrelation.rows() <<
                   " rows and " << fraCorrelation.columns() << " columns");

        //1.Reduced-factor pseudo-root matrices for each time step
        const Spread displacement = 0;
        Real componentRetainedPercentage = 1.0;
        Matrix jacobian =
                SwapForwardMappings::coterminalSwapZedMatrix(
                    curveState, displacement);
        for (Size k=0; k<fraCorrelationMatrix_.size();++k){
            //reducing rank
            Disposable<Matrix> fraPseudoRoot = rankReducedSqrt(fraCorrelation,
                                                numberOfFactors_,
                                                componentRetainedPercentage,
                                                SalvagingAlgorithm::None);
            // converting to swap correlation
            Disposable<Matrix> swapPseudoRoot = jacobian*fraPseudoRoot;
            // rescaling swapPseudoRoot
            for (Size i = 0; i< swapPseudoRoot.rows(); ++i){
                Real sum = 0;
                for (Size j = 0; j< swapPseudoRoot.columns(); ++j){
                    sum+= swapPseudoRoot[i][j]*swapPseudoRoot[i][j];
                }
                sum = std::sqrt(sum);
                for (Size j = 0; j< swapPseudoRoot.columns(); ++j){
                    swapPseudoRoot[i][j] /= sum;
                }
             }
            pseudoRoots_[k] = swapPseudoRoot;
        }
    }

    //0.add yield curve and displacements in the costructor
    //1.FRA correlation matrix (todo: later)
    //2.Reduced-factor pseudo-root matrices for each time step
    //3.Compute Z matrix
    //4.Normalize 2.*3.

    const EvolutionDescription&
    SwapFromFRACorrelationStructure::evolution() const {
        return evolution_;
    }

    Size SwapFromFRACorrelationStructure::numberOfFactors() const {
        return numberOfFactors_;
    }

    const Matrix& SwapFromFRACorrelationStructure::pseudoRoot(Size i) const {
        QL_REQUIRE(i<pseudoRoots_.size(),
                   "index (" << i << ") must be less than pseudoRoots size ("
                   << pseudoRoots_.size() << ")");
        return pseudoRoots_[i];
    }

}


/*Matrix swapCorrelation;
            swapCorrelation = jacobian*fraCorrelationMatrix_[k]*transpose(jacobian);
            Real componentRetainedPercentage = 1.0;
            Matrix& pseudoRoot = pseudoRoots_[k];
            pseudoRoot = rankReducedSqrt(  swapCorrelation,
                                                numberOfFactors_,
                                                componentRetainedPercentage,
                                                SalvagingAlgorithm::None);
            for (Size i = 0; i< pseudoRoots_.rows(); ++i){
                Real sum = 0;
                for (Size j = 0; j< pseudoRoots_.columns(); ++j){
                    sum+= pseudoRoot[i][j]*pseudoRoot[i][j];
                }
                pseudoRoot[i] /= std::sqrt(sum);
             }*/
