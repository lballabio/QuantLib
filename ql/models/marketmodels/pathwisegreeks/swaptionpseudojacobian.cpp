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
#include <ql/pricingengines/blackformula.hpp>
#include <ql/math/solvers1d/brent.hpp>


namespace QuantLib
{

    SwaptionPseudoDerivative::SwaptionPseudoDerivative(
        const ext::shared_ptr<MarketModel>& inputModel, Size startIndex, Size endIndex) {
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

        while (index <  inputModel->evolution().numberOfSteps() && inputModel->evolution().firstAliveRate()[index] <= startIndex)
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

        impliedVolatility_ = std::sqrt(variance_/expiry_);

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
                    Real sum=0.0;
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

    namespace
    {

        // this class doesn't copy anything so fast to create and use
        // but make sure everything it uses stays in scope... 
        class QuickCap
        {
        public:
            QuickCap(Real Strike,
                const std::vector<Real>& annuities,
                const std::vector<Real>& currentRates,
                const std::vector<Real>& expiries,
                Real price);

            Real operator()(Real volatility) const; // returns difference from input price

            Real vega(Real volatility) const; // returns vol derivative



        private:
            Real strike_;
            const std::vector<Real>& annuities_;
            const std::vector<Real>& currentRates_;
            const std::vector<Real>& expiries_;
            Real price_;

        };

        QuickCap::QuickCap(Real Strike,
            const std::vector<Real>& annuities,
            const std::vector<Real>& currentRates,
            const std::vector<Real>& expiries,
            Real price)
            :
        strike_(Strike),
            annuities_(annuities),
            currentRates_(currentRates),
            expiries_(expiries),
            price_(price)
        {
        }

        Real QuickCap::operator()(Real volatility) const
        {
            Real price =0.0;
            for (Size i=0; i < annuities_.size(); ++i)
            {
                price += blackFormula(Option::Call,strike_,
                    currentRates_[i],
                    volatility*std::sqrt(expiries_[i]),
                    annuities_[i]);


            }
            return price-price_;
        }

        Real QuickCap::vega(Real volatility) const // returns vol derivative
        {
            Real vega =0.0;
            for (Size i=0; i < annuities_.size(); ++i)
            {
         

                vega+= blackFormulaVolDerivative(strike_,currentRates_[i],
                                                 
                                                 volatility*std::sqrt(expiries_[i]),
                                                 expiries_[i],
                                                 annuities_[i],
                                                 0.0);
            }

            return vega;
        }



    }

    CapPseudoDerivative::CapPseudoDerivative(const ext::shared_ptr<MarketModel>& inputModel,
                                             Real strike,
                                             Size startIndex,
                                             Size endIndex,
                                             Real firstDF)
    : firstDF_(firstDF) {
        QL_REQUIRE(startIndex < endIndex, "for a cap pseudo derivative the start of the cap must be before the end");
        QL_REQUIRE( endIndex <= inputModel->numberOfRates(), "for a cap pseudo derivative the end of the cap must before the end of the rates");

        Size numberCaplets = endIndex-startIndex;
        Size numberRates = inputModel->numberOfRates();
        Size factors = inputModel->numberOfFactors();
        LMMCurveState curve(inputModel->evolution().rateTimes());
        curve.setOnForwardRates(inputModel->initialRates());

        const Matrix& totalCovariance(inputModel->totalCovariance(inputModel->numberOfSteps()-1));

        std::vector<Real> displacedImpliedVols(numberCaplets);
        std::vector<Real> annuities(numberCaplets);
        std::vector<Real> initialRates(numberCaplets);
        std::vector<Real> expiries(numberCaplets);

        Real capPrice =0.0;

        Real guess=0.0;
        Real minVol = 1e10;
        Real maxVol =0.0;

        for (Size j = startIndex; j < endIndex; ++j)
        {
            Size capletIndex = j - startIndex;
            Time resetTime = inputModel->evolution().rateTimes()[j];
            expiries[capletIndex] =  resetTime;

            Real sd = std::sqrt(totalCovariance[j][j]);
            displacedImpliedVols[capletIndex] = std::sqrt(totalCovariance[j][j]/resetTime);

            Real forward = inputModel->initialRates()[j];
            initialRates[capletIndex] = forward;

            Real annuity = curve.discountRatio(j+1,0)* inputModel->evolution().rateTaus()[j]*firstDF_;
            annuities[capletIndex] = annuity;

            Real displacement =  inputModel->displacements()[j];

            guess+=  displacedImpliedVols[capletIndex]*(forward+displacement)/forward;
            minVol =std::min(minVol, displacedImpliedVols[capletIndex]);
            maxVol =std::max(maxVol, displacedImpliedVols[capletIndex]*(forward+displacement)/forward);


            Real capletPrice = blackFormula(Option::Call,
                strike,
                forward,
                sd,
                annuity,
                displacement
                );

            capPrice += capletPrice;

        }

        guess/=numberCaplets;


        for (Size step =0; step < inputModel->evolution().numberOfSteps(); ++step)
        {
            Matrix thisDerivative(numberRates,factors,0.0);

            for (Size rate =std::max(inputModel->evolution().firstAliveRate()[step],startIndex); 
                rate < endIndex; ++rate)
            {
                for (Size f=0; f < factors; ++f)
                {
                    Real expiry = inputModel->evolution().rateTimes()[rate];
                    Real volDerivative = inputModel->pseudoRoot(step)[rate][f]
                    /(displacedImpliedVols[rate-startIndex]*expiry);
                    Real capletVega = blackFormulaVolDerivative(strike,inputModel->initialRates()[rate],
                        displacedImpliedVols[rate-startIndex]*std::sqrt(expiry),
                        expiry,
                        annuities[rate-startIndex],
                        inputModel->displacements()[rate]);

                    // note that the cap derivative  is equal to one of the caplet ones so we lose a loop
                    thisDerivative[rate][f] = volDerivative*capletVega;
                }
            }

            priceDerivatives_.push_back(thisDerivative);

        }

        QuickCap capPricer(strike, annuities, initialRates, expiries,capPrice);

        Size maxEvaluations = 1000;
        Real accuracy = 1E-6;

        Brent solver;
        solver.setMaxEvaluations(maxEvaluations);
        impliedVolatility_ =   solver.solve(capPricer,accuracy,guess,minVol*0.99,maxVol*1.01);
      
            

        vega_ = capPricer.vega(impliedVolatility_);



        for (Size step =0; step < inputModel->evolution().numberOfSteps(); ++step)
        {
            Matrix thisDerivative(numberRates,factors,0.0);

            for (Size rate =std::max(inputModel->evolution().firstAliveRate()[step],startIndex); 
                rate < endIndex; ++rate)
            {
                for (Size f=0; f < factors; ++f)
                {
                   
                    thisDerivative[rate][f] = priceDerivatives_[step][rate][f]/vega_;
                }
            }

            volatilityDerivatives_.push_back(thisDerivative);

        }



    }


    const Matrix& CapPseudoDerivative::priceDerivative(Size i) const
    {
        return priceDerivatives_[i];
    }

    const Matrix& CapPseudoDerivative::volatilityDerivative(Size i) const
    { 
        return volatilityDerivatives_[i];
    }



    Real CapPseudoDerivative::impliedVolatility() const
    {
        return impliedVolatility_;
    }




}
