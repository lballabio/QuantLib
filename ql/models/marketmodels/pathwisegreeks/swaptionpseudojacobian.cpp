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



#include <ql/models/marketmodels/pathwisegreeks/swaptionpseudojacobian.hpp>
#include <ql/models/marketmodels/curvestates/lmmcurvestate.hpp>
#include <ql/models/marketmodels/evolutiondescription.hpp>
#include <ql/models/marketmodels/swapforwardmappings.hpp>
namespace QuantLib
{

    SwaptionPseudoDerivative::SwaptionPseudoDerivative(boost::shared_ptr<MarketModel> inputModel,
        Size startIndex,
        Size endIndex)
    {
        std::vector<Real> subRateTimes(inputModel->evolution().rateTimes().begin()+startIndex, 
            inputModel->evolution().rateTimes().begin()+endIndex+1);

        std::vector<Real> subForwards(inputModel->initialRates().begin()+startIndex,inputModel->initialRates().begin()+endIndex);

        LMMCurveState cs(subRateTimes);
        cs.setOnForwardRates(subForwards);

        Matrix zed(SwapForwardMappings::coterminalSwapZedMatrix(cs,inputModel->displacements()[0]));
        Size factors = inputModel->numberOfFactors();


        //first compute variance and implied vol

        variance_=0.0;
        Size index=0;

        while (inputModel->evolution().firstAliveRate()[index] <= startIndex)
        {   
            const Matrix& thisPseudo = inputModel->pseudoRoot(index);

            Real thisVariance_ =0.0;
            for (Size j=startIndex; j < endIndex; ++j)
                for (Size k=startIndex; k < endIndex; ++k)
                    for (Size f=0; f < factors; ++f)
                        thisVariance_+= zed[0][j-startIndex]*thisPseudo[j][f]*thisPseudo[k][f]*zed[0][k-startIndex];

            variance_ += thisVariance_;

            ++index;
        }

        Size stopIndex = index;

        expiry_ = subRateTimes[0];

        impliedVolatility_ = sqrt(variance_/expiry_);

        Real scale = 0.5*(1.0/expiry_)/impliedVolatility_;

        Size numberRates = inputModel->evolution().numberOfRates();

        Matrix thisDerivative(numberRates, factors,0.0);
        Matrix nullDerivative(numberRates, factors,0.0);

        index =0;

        while (index < stopIndex)
        {
            const Matrix& thisPseudo = inputModel->pseudoRoot(index);

            for (Size rate=startIndex; rate<endIndex; ++rate)
            {
                Size zIndex = rate -startIndex;
                for (Size f =0; f < factors; ++f)
                {
                    double sum=0.0;
                    for (Size rate2 = startIndex; rate2<endIndex; ++rate2)
                    {
                        Size zIndex2 = rate2-startIndex;
                        sum += zed[0][zIndex2] * thisPseudo[rate2][f];
                    }
                    sum *= 2.0*zed[0][zIndex];

                    thisDerivative[rate][f] =sum;

                }
            }

            varianceDerivatives_.push_back(thisDerivative);

            for ( Size rate=startIndex; rate<endIndex; ++rate)
                for (Size f =0; f < factors; ++f)
                    thisDerivative[rate][f] *= scale;

            volatilityDerivatives_.push_back(thisDerivative);     

            ++index;
        }

        for (; index < inputModel->evolution().numberOfSteps(); ++index)
        {
            varianceDerivatives_.push_back(nullDerivative);
            volatilityDerivatives_.push_back(nullDerivative);

        }





    }

    const Matrix& SwaptionPseudoDerivative::varianceDerivative(Size i) const
    {
        return varianceDerivatives_[i];
    }

    const Matrix& SwaptionPseudoDerivative::volatilityDerivative(Size i) const
    {
        return volatilityDerivatives_[i];
    }

    Real SwaptionPseudoDerivative::impliedVolatility() const
    {
        return impliedVolatility_;
    }
    Real SwaptionPseudoDerivative::variance() const
    {
        return variance_;
    }
    Real SwaptionPseudoDerivative::expiry() const
    {
        return expiry_;
    }

}
