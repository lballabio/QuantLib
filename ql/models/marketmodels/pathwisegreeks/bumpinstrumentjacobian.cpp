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

#include <ql/models/marketmodels/pathwisegreeks/bumpinstrumentjacobian.hpp>
#include <ql/math/matrixutilities/basisincompleteordered.hpp>
#include <ql/models/marketmodels/pathwisegreeks/swaptionpseudojacobian.hpp>
#include <ql/models/marketmodels/evolutiondescription.hpp>

namespace QuantLib
{   


    VolatilityBumpInstrumentJacobian::VolatilityBumpInstrumentJacobian(const VegaBumpCollection& bumps,
        const std::vector<Swaption>& swaptions,
        const std::vector<Cap>& caps)
        : bumps_(bumps), swaptions_(swaptions), caps_(caps), computed_(false,swaptions.size()+caps.size()),
        derivatives_(swaptions.size()+caps.size(),std::vector<Real>(bumps.numberBumps())),
        bumpMatrix_(swaptions.size()+caps.size(),bumps_.numberBumps())
    {
        onePercentBumps_ = derivatives_;
        allComputed_=false;

    }

    std::vector<Real> VolatilityBumpInstrumentJacobian::derivativesVolatility(Size j) const {
        QL_REQUIRE(j < swaptions_.size()+caps_.size(), "too high index passed to VolatilityBumpInstrumentJacobian::derivativesVolatility");

        if (computed_[j])
            return derivatives_[j];

        derivatives_[j].resize(bumps_.numberBumps());
        onePercentBumps_[j].resize(bumps_.numberBumps());


        Real sizesq=0.0;
        computed_[j] = true;

        Size initj = j;

        if ( j < swaptions_.size()) // ok its a swaptions
        {
            SwaptionPseudoDerivative thisPseudo(bumps_.associatedModel(), swaptions_[j].startIndex_,swaptions_[j].endIndex_);

            for (Size k=0;  k < bumps_.numberBumps(); ++k)
            {
                Real v =0.0;

                for (Size i= bumps_.allBumps()[k].stepBegin(); i < bumps_.allBumps()[k].stepEnd(); ++i)
                {
                    const Matrix& fullDerivative = thisPseudo.volatilityDerivative(i);
                    for (Size f= bumps_.allBumps()[k].factorBegin(); f < bumps_.allBumps()[k].factorEnd(); ++f)
                        for (Size r= bumps_.allBumps()[k].rateBegin(); r < bumps_.allBumps()[k].rateEnd(); ++r)
                            v += fullDerivative[r][f];
                }

                derivatives_[j][k] =v;
                sizesq+= v*v;
            }

        }
        else // its a cap 
        {
            j-= swaptions_.size();

            
            CapPseudoDerivative thisPseudo(bumps_.associatedModel(), caps_[j].strike_, caps_[j].startIndex_,caps_[j].endIndex_,1.0); // ifrst df shouldn't make any difference

           

            for (Size k=0;  k < bumps_.numberBumps(); ++k)
            {
                Real v =0.0;

                for (Size i= bumps_.allBumps()[k].stepBegin(); i < bumps_.allBumps()[k].stepEnd(); ++i)
                {
                    const Matrix& fullDerivative = thisPseudo.volatilityDerivative(i);
                    for (Size f= bumps_.allBumps()[k].factorBegin(); f < bumps_.allBumps()[k].factorEnd(); ++f)
                        for (Size r= bumps_.allBumps()[k].rateBegin(); r < bumps_.allBumps()[k].rateEnd(); ++r)
                            v += fullDerivative[r][f];
                }

                sizesq += v*v;

                derivatives_[initj][k] =v;
            }

        }

        for (Size k=0; k < bumps_.numberBumps(); ++k)
        {
            bumpMatrix_[initj][k] = onePercentBumps_[initj][k] = 0.01 * derivatives_[initj][k]/sizesq;

        }



     return derivatives_[initj];
    }


    std::vector<Real> VolatilityBumpInstrumentJacobian::onePercentBump(Size j) const {
        derivativesVolatility(j); 
    
        return onePercentBumps_[j];
    }

    const Matrix& VolatilityBumpInstrumentJacobian::getAllOnePercentBumps() const
    {
        if (!allComputed_)
             for (Size i=0; i <swaptions_.size()+caps_.size(); ++i)
                derivativesVolatility(i);

        allComputed_ =true;


        return bumpMatrix_;
    }



    OrthogonalizedBumpFinder::OrthogonalizedBumpFinder(const VegaBumpCollection& bumps,
                                     const std::vector<VolatilityBumpInstrumentJacobian::Swaption>& swaptions,
                                     const std::vector<VolatilityBumpInstrumentJacobian::Cap>& caps,
                                     Real multiplierCutOff,
                                     Real tolerance) : 
                                                        derivativesProducer_(bumps,swaptions,caps), 
                                                        multiplierCutOff_(multiplierCutOff), 
                                                        tolerance_(tolerance)
    {
        


    }

    void OrthogonalizedBumpFinder::GetVegaBumps(std::vector<std::vector<Matrix> >& theBumps) const
    {
        OrthogonalProjections projector(derivativesProducer_.getAllOnePercentBumps(),
                                                            multiplierCutOff_,
                                                             tolerance_  );


        Size numberRestrictedBumps(projector.numberValidVectors());

        ext::shared_ptr<MarketModel> marketmodel(derivativesProducer_.getInputBumps().associatedModel());
        const EvolutionDescription& evolution(marketmodel->evolution());

        Size numberSteps = evolution.numberOfSteps();
        Size numberRates = evolution.numberOfRates();
        Size factors = marketmodel->numberOfFactors();

        theBumps.resize(numberSteps);
       // recall that the bumps: We do the outermost vector by time step and inner one by which vega.

        for (auto& theBump : theBumps)
            theBump.resize(numberRestrictedBumps);

        Matrix modelMatrix(numberRates, factors,0.0);

        for (Size i=0;  i< numberSteps; ++i)
            for (Size j=0; j < numberRestrictedBumps; ++j)
                theBumps[i][j] = modelMatrix;

        const std::vector<VegaBumpCluster>& bumpClusters(derivativesProducer_.getInputBumps().allBumps());


        Size bumpIndex =0;

        for (Size instrument=0; instrument < projector.validVectors().size(); ++instrument)
        {
            if (projector.validVectors()[instrument])
            {
                for (Size cluster =0; cluster< bumpClusters.size(); ++cluster)
                {
                    Real magnitude = projector.GetVector(instrument)[cluster];

                    for (Size step = bumpClusters[cluster].stepBegin(); step <  bumpClusters[cluster].stepEnd(); ++step)
                        for (Size rate = bumpClusters[cluster].rateBegin(); rate < bumpClusters[cluster].rateEnd(); ++rate)
                            for (Size factor = bumpClusters[cluster].factorBegin(); factor <  bumpClusters[cluster].factorEnd(); ++factor)
                                theBumps[step][bumpIndex][rate][factor] = magnitude;
                }

                ++bumpIndex;


            }

        }



    }
}
