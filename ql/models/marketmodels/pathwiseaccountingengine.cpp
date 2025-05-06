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

#include <ql/models/marketmodels/curvestate.hpp>
#include <ql/models/marketmodels/discounter.hpp>
#include <ql/models/marketmodels/evolutiondescription.hpp>
#include <ql/models/marketmodels/evolvers/lognormalfwdrateeuler.hpp>
#include <ql/models/marketmodels/marketmodel.hpp>
#include <ql/models/marketmodels/pathwiseaccountingengine.hpp>
#include <algorithm>
#include <utility>

namespace QuantLib {

    PathwiseAccountingEngine::PathwiseAccountingEngine(
        ext::shared_ptr<LogNormalFwdRateEuler> evolver, // method relies heavily on LMM Euler
        const Clone<MarketModelPathwiseMultiProduct>& product,
        ext::shared_ptr<MarketModel> pseudoRootStructure, // we need pseudo-roots and displacements
        Real initialNumeraireValue)
    : evolver_(std::move(evolver)), product_(product),
      pseudoRootStructure_(std::move(pseudoRootStructure)),
      initialNumeraireValue_(initialNumeraireValue), numberProducts_(product->numberOfProducts()),
      doDeflation_(!product->alreadyDeflated()), numerairesHeld_(product->numberOfProducts()),
      numberCashFlowsThisStep_(product->numberOfProducts()),
      cashFlowsGenerated_(product->numberOfProducts()),
      deflatorAndDerivatives_(pseudoRootStructure_->numberOfRates() + 1) {

        numberRates_ = pseudoRootStructure_->numberOfRates();
        numberSteps_ = pseudoRootStructure_->numberOfSteps();

        Matrix VModel(numberSteps_+1,numberRates_);



        Discounts_ = Matrix(numberSteps_+1,numberRates_+1);

        for (Size i=0; i <= numberSteps_; ++i)
            Discounts_[i][0] = 1.0;


        V_.reserve(numberProducts_);

        Matrix  modelCashFlowIndex(product_->possibleCashFlowTimes().size(), numberRates_+1);


        numberCashFlowsThisIndex_.resize(numberProducts_);

        for (Size i=0; i<numberProducts_; ++i)
        {
            cashFlowsGenerated_[i].resize(
                product_->maxNumberOfCashFlowsPerProductPerStep());

            for (auto& j : cashFlowsGenerated_[i])
                j.amount.resize(numberRates_ + 1);

            numberCashFlowsThisIndex_[i].resize(product_->possibleCashFlowTimes().size());

            V_.push_back(VModel);


            totalCashFlowsThisIndex_.push_back(modelCashFlowIndex);
        }

        LIBORRatios_ = VModel;
        StepsDiscountsSquared_ = VModel;
        LIBORRates_ =VModel;




        const std::vector<Time>& cashFlowTimes =
            product_->possibleCashFlowTimes();
        numberCashFlowTimes_ = cashFlowTimes.size();

        const std::vector<Time>& rateTimes = product_->evolution().rateTimes();
        const std::vector<Time>& evolutionTimes = product_->evolution().evolutionTimes();
        discounters_.reserve(cashFlowTimes.size());

        for (Real cashFlowTime : cashFlowTimes)
            discounters_.emplace_back(cashFlowTime, rateTimes);


        // need to check that we are in money market measure


        // we need to allocate cash-flow times to steps, i.e. what is the last step completed before a flow occurs
        // what we really need is for each step, what cash flow time indices to look at

        cashFlowIndicesThisStep_.resize(numberSteps_);

        for (Size i=0; i < numberCashFlowTimes_; ++i)
        {
            auto it =
                std::upper_bound(evolutionTimes.begin(), evolutionTimes.end(), cashFlowTimes[i]);
            if (it != evolutionTimes.begin())
                --it;
            Size index = it - evolutionTimes.begin();
            cashFlowIndicesThisStep_[index].push_back(i);
        }

        partials_ = Matrix(pseudoRootStructure_->numberOfFactors(),numberRates_);
    }

    Real PathwiseAccountingEngine::singlePathValues(std::vector<Real>& values)
    {

        const std::vector<Real> initialForwards_(pseudoRootStructure_->initialRates());
        currentForwards_ = initialForwards_;
        // clear accumulation variables
        for (Size i=0; i < numberProducts_; ++i)
        {
            numerairesHeld_[i]=0.0;

            for (Size j=0; j < numberCashFlowTimes_; ++j)
            {
                numberCashFlowsThisIndex_[i][j] =0;

                for (Size k=0; k <= numberRates_; ++k)
                    totalCashFlowsThisIndex_[i][j][k] =0.0;
            }

            for (Size l=0;  l< numberRates_; ++l)
                for (Size m=0; m <= numberSteps_; ++m)
                    V_[i][m][l] =0.0;

        }



        Real weight = evolver_->startNewPath();
        product_->reset();

        Size thisStep;

        bool done = false;
        do {
            thisStep = evolver_->currentStep();
            Size storeStep = thisStep+1;
            weight *= evolver_->advanceStep();

            done = product_->nextTimeStep(evolver_->currentState(),
                numberCashFlowsThisStep_,
                cashFlowsGenerated_);

            lastForwards_ = currentForwards_;
            currentForwards_ =  evolver_->currentState().forwardRates();

            for (unsigned long i=0; i < numberRates_; ++i)
            {
                Real x=  evolver_->currentState().discountRatio(i+1,i);
                StepsDiscountsSquared_[storeStep][i] = x*x;

                LIBORRatios_[storeStep][i] = currentForwards_[i]/lastForwards_[i];
                LIBORRates_[storeStep][i] = currentForwards_[i];
                Discounts_[storeStep][i+1] = evolver_->currentState().discountRatio(i+1,0);
            }

            // for each product...
            for (Size i=0; i<numberProducts_; ++i)
            {
                // ...and each cash flow...
                for (Size j=0; j<numberCashFlowsThisStep_[i]; ++j)
                {
                    Size k = cashFlowsGenerated_[i][j].timeIndex;
                    ++numberCashFlowsThisIndex_[i][ k];

                    for (Size l=0; l <= numberRates_; ++l)
                        totalCashFlowsThisIndex_[i][k][l] += cashFlowsGenerated_[i][j].amount[l]*weight;

                }
            }


        } while (!done);

        // ok we've gathered cash-flows, still have to backwards computation

        Size factors = pseudoRootStructure_->numberOfFactors();
        const std::vector<Time>& taus= pseudoRootStructure_->evolution(). rateTaus();

        bool flowsFound = false;

        Integer finalStepDone = thisStep;

        for (Integer currentStep =  numberSteps_-1; currentStep >=0 ; --currentStep) // must be a signed type as we go negative
        {
            Integer stepToUse = std::min<Integer>(currentStep, finalStepDone)+1;

            for (Size k=0; k < cashFlowIndicesThisStep_[currentStep].size(); ++k)
            {
                Size cashFlowIndex =cashFlowIndicesThisStep_[currentStep][k];

                // first check to see if anything actually happened before spending time on computing stuff
                bool noFlows = true;
                for (Size l=0; l < numberProducts_ && noFlows; ++l)
                    noFlows = noFlows && (numberCashFlowsThisIndex_[l][cashFlowIndex] ==0);

                flowsFound = flowsFound || !noFlows;

                if (!noFlows)
                {
                    if (doDeflation_)
                        discounters_[cashFlowIndex].getFactors(LIBORRates_, Discounts_,stepToUse, deflatorAndDerivatives_); // get amount to discount cash flow by and amount to multiply its derivatives by

                    for (Size j=0; j < numberProducts_; ++j)
                    {
                        if (numberCashFlowsThisIndex_[j][cashFlowIndex] > 0)
                        {
                            Real deflatedCashFlow = totalCashFlowsThisIndex_[j][cashFlowIndex][0];
                            if (doDeflation_)
                                deflatedCashFlow *= deflatorAndDerivatives_[0];
                            //cashFlowsGenerated_[j][cashFlowIndex].amount[0]*deflatorAndDerivatives_[0];
                            numerairesHeld_[j] += deflatedCashFlow;

                            for (Size i=1; i <= numberRates_; ++i)
                            {
                                Real thisDerivative =  totalCashFlowsThisIndex_[j][cashFlowIndex][i];
                                if (doDeflation_)
                                {
                                    thisDerivative *= deflatorAndDerivatives_[0];
                                    thisDerivative +=  totalCashFlowsThisIndex_[j][cashFlowIndex][0]*deflatorAndDerivatives_[i];
                                }

                                V_[j][stepToUse][i-1] += thisDerivative; // zeroth row of V is t =0 not t_0
                            }
                        }
                    }
                }
            }

            // need to do backwards updating
            if (flowsFound)
            {
                Integer nextStepToUse  = std::min<Integer>(currentStep-1, finalStepDone);
                Integer nextStepIndex = nextStepToUse+1;
                if (nextStepIndex != stepToUse) // then we need to update V
                {

                    const Matrix& thisPseudoRoot_= pseudoRootStructure_->pseudoRoot(currentStep);

                    for (Size i=0; i < numberProducts_; ++i)
                    {
                        // compute partials
                        for (Size f=0; f < factors; ++f)
                        {
                            Real libor = LIBORRates_[stepToUse][numberRates_-1];
                            Real V = V_[i][stepToUse][numberRates_-1];
                            Real pseudo = thisPseudoRoot_[numberRates_-1][f];
                            Real thisPartialTerm = libor*V*pseudo;
                            partials_[f][numberRates_-1] = thisPartialTerm;

                            for (Integer r = numberRates_-2; r >=0 ; --r)
                            {
                                Real thisPartialTermr = LIBORRates_[stepToUse][r]*V_[i][stepToUse][r]*thisPseudoRoot_[r][f];

                                partials_[f][r] = partials_[f][r+1] + thisPartialTermr;

                            }
                        }
                        for (Size j=0; j < numberRates_; ++j)
                        {
                            Real nextV = V_[i][stepToUse][j] * LIBORRatios_[stepToUse][j];
                            V_[i][nextStepIndex][j] = nextV;

                            Real summandTerm = 0.0;
                            for (Size f=0; f < factors; ++f)
                                summandTerm += thisPseudoRoot_[j][f]*partials_[f][j];

                            summandTerm *= taus[j]*StepsDiscountsSquared_[stepToUse][j];

                            V_[i][nextStepIndex][j] += summandTerm;

                        }
                    }

                }
            }




        }

        // write answer into values

        for (Size i=0; i < numberProducts_; ++i)
        {
            values[i] = numerairesHeld_[i]*initialNumeraireValue_;
            for (Size j=0; j < numberRates_; ++j)
                values[(i+1)*numberProducts_+j] = V_[i][0][j]*initialNumeraireValue_;
        }

        return 1.0; // we have put the weight in already, this results in lower variance since weight changes along the path
    }

    void PathwiseAccountingEngine::multiplePathValues(SequenceStatisticsInc& stats,
        Size numberOfPaths)
    {
        std::vector<Real> values(product_->numberOfProducts()*(numberRates_+1));
        for (Size i=0; i<numberOfPaths; ++i)
        {
            Real weight = singlePathValues(values);
            stats.add(values,weight);
        }
    }

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
 
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    PathwiseVegasAccountingEngine::PathwiseVegasAccountingEngine(
        ext::shared_ptr<LogNormalFwdRateEuler> evolver, // method relies heavily on LMM Euler
        const Clone<MarketModelPathwiseMultiProduct>& product,
        ext::shared_ptr<MarketModel> pseudoRootStructure, // we need pseudo-roots and displacements
        const std::vector<std::vector<Matrix> >& vegaBumps,
        Real initialNumeraireValue)
    : evolver_(std::move(evolver)), product_(product),
      pseudoRootStructure_(std::move(pseudoRootStructure)),
      initialNumeraireValue_(initialNumeraireValue), numberProducts_(product->numberOfProducts()),
      doDeflation_(!product->alreadyDeflated()), numerairesHeld_(product->numberOfProducts()),
      numberCashFlowsThisStep_(product->numberOfProducts()),
      cashFlowsGenerated_(product->numberOfProducts()),
      stepsDiscounts_(pseudoRootStructure_->numberOfRates() + 1),
      vegasThisPath_(product->numberOfProducts(), vegaBumps[0].size()),
      deflatorAndDerivatives_(pseudoRootStructure_->numberOfRates() + 1) {

        stepsDiscounts_[0]=1.0;

        numberRates_ = pseudoRootStructure_->numberOfRates();
        numberSteps_ = pseudoRootStructure_->numberOfSteps();
        fullDerivatives_.resize(numberRates_);


        const EvolutionDescription& evolution = pseudoRootStructure_->evolution();
        numeraires_ =  moneyMarketMeasure(evolution);


        QL_REQUIRE(vegaBumps.size() == numberSteps_, "we need one vector of vega bumps for each step.");

        numberBumps_ = vegaBumps[0].size();

        for (Size i =0; i < numberSteps_; ++i)
        {
            Size thisSize = vegaBumps[i].size();
            QL_REQUIRE(thisSize == numberBumps_,"We must have precisely the same number of bumps for each step.");
            jacobianComputers_.emplace_back(
                pseudoRootStructure_->pseudoRoot(i), evolution.firstAliveRate()[i], numeraires_[i],
                evolution.rateTaus(), vegaBumps[i], pseudoRootStructure_->displacements());

            jacobiansThisPaths_.emplace_back(numberBumps_, pseudoRootStructure_->numberOfRates());
        }



        Matrix VModel(numberSteps_+1,numberRates_);



        Discounts_ = Matrix(numberSteps_+1,numberRates_+1);

        for (Size i=0; i <= numberSteps_; ++i)
            Discounts_[i][0] = 1.0;


        V_.reserve(numberProducts_);

        Matrix  modelCashFlowIndex(product_->possibleCashFlowTimes().size(), numberRates_+1);


        numberCashFlowsThisIndex_.resize(numberProducts_);

        for (Size i=0; i<numberProducts_; ++i)
        {
            cashFlowsGenerated_[i].resize(
                product_->maxNumberOfCashFlowsPerProductPerStep());

            for (auto& j : cashFlowsGenerated_[i])
                j.amount.resize(numberRates_ + 1);

            numberCashFlowsThisIndex_[i].resize(product_->possibleCashFlowTimes().size());

            V_.push_back(VModel);


            totalCashFlowsThisIndex_.push_back(modelCashFlowIndex);
        }

        LIBORRatios_ = VModel;
        StepsDiscountsSquared_ = VModel;
        LIBORRates_ =VModel;




        const std::vector<Time>& cashFlowTimes =
            product_->possibleCashFlowTimes();
        numberCashFlowTimes_ = cashFlowTimes.size();

        const std::vector<Time>& rateTimes = product_->evolution().rateTimes();
        const std::vector<Time>& evolutionTimes = product_->evolution().evolutionTimes();
        discounters_.reserve(cashFlowTimes.size());

        for (Real cashFlowTime : cashFlowTimes)
            discounters_.emplace_back(cashFlowTime, rateTimes);


        // need to check that we are in money market measure


        // we need to allocate cash-flow times to steps, i.e. what is the last step completed before a flow occurs
        // what we really need is for each step, what cash flow time indices to look at

        cashFlowIndicesThisStep_.resize(numberSteps_);

        for (Size i=0; i < numberCashFlowTimes_; ++i)
        {
            auto it =
                std::upper_bound(evolutionTimes.begin(), evolutionTimes.end(), cashFlowTimes[i]);
            if (it != evolutionTimes.begin())
                --it;
            Size index = it - evolutionTimes.begin();
            cashFlowIndicesThisStep_[index].push_back(i);
        }

        partials_ = Matrix(pseudoRootStructure_->numberOfFactors(),numberRates_);
    }

    Real PathwiseVegasAccountingEngine::singlePathValues(std::vector<Real>& values)
    {

        const std::vector<Real>& initialForwards_(pseudoRootStructure_->initialRates());
        currentForwards_ = initialForwards_;
        // clear accumulation variables
        for (Size i=0; i < numberProducts_; ++i)
        {
            numerairesHeld_[i]=0.0;

            for (Size j=0; j < numberCashFlowTimes_; ++j)
            {
                numberCashFlowsThisIndex_[i][j] =0;

                for (Size k=0; k <= numberRates_; ++k)
                    totalCashFlowsThisIndex_[i][j][k] =0.0;
            }

            for (Size l=0;  l< numberRates_; ++l)
                for (Size m=0; m <= numberSteps_; ++m)
                    V_[i][m][l] =0.0;

            for (Size p=0; p < numberBumps_; ++p)
                vegasThisPath_[i][p] =0.0;

        }



        Real weight = evolver_->startNewPath();
        product_->reset();

        Size thisStep;

        bool done = false;
        do {
            thisStep = evolver_->currentStep();
            Size storeStep = thisStep+1;
            weight *= evolver_->advanceStep();

            done = product_->nextTimeStep(evolver_->currentState(),
                numberCashFlowsThisStep_,
                cashFlowsGenerated_);

            lastForwards_ = currentForwards_;
            currentForwards_ =  evolver_->currentState().forwardRates();

            for (unsigned long i=0; i < numberRates_; ++i)
            {
                Real x=  evolver_->currentState().discountRatio(i+1,i);
                stepsDiscounts_[i+1] = x;
                StepsDiscountsSquared_[storeStep][i] = x*x;

                LIBORRatios_[storeStep][i] = currentForwards_[i]/lastForwards_[i];
                LIBORRates_[storeStep][i] = currentForwards_[i];
                Discounts_[storeStep][i+1] = evolver_->currentState().discountRatio(i+1,0);
            }

            jacobianComputers_[thisStep].getBumps(lastForwards_,
                                         stepsDiscounts_,
                                         currentForwards_,
                                         evolver_->browniansThisStep(),
                                         jacobiansThisPaths_[thisStep]);



            // for each product...
            for (Size i=0; i<numberProducts_; ++i)
            {
                // ...and each cash flow...
                for (Size j=0; j<numberCashFlowsThisStep_[i]; ++j)
                {
                    Size k = cashFlowsGenerated_[i][j].timeIndex;
                    ++numberCashFlowsThisIndex_[i][ k];

                    for (Size l=0; l <= numberRates_; ++l)
                        totalCashFlowsThisIndex_[i][k][l] += cashFlowsGenerated_[i][j].amount[l]*weight;

                }
            }


        } while (!done);

        // ok we've gathered cash-flows, still have to backwards computation

        Size factors = pseudoRootStructure_->numberOfFactors();
        const std::vector<Time>& taus= pseudoRootStructure_->evolution(). rateTaus();

        bool flowsFound = false;

        Integer finalStepDone = thisStep;

        for (Integer currentStep =  numberSteps_-1; currentStep >=0 ; --currentStep) // must be a signed type as we go negative
        {
            Integer stepToUse = std::min<Integer>(currentStep, finalStepDone)+1;

            for (Size k=0; k < cashFlowIndicesThisStep_[currentStep].size(); ++k)
            {
                Size cashFlowIndex =cashFlowIndicesThisStep_[currentStep][k];

                // first check to see if anything actually happened before spending time on computing stuff
                bool noFlows = true;
                for (Size l=0; l < numberProducts_ && noFlows; ++l)
                    noFlows = noFlows && (numberCashFlowsThisIndex_[l][cashFlowIndex] ==0);

                flowsFound = flowsFound || !noFlows;

                if (!noFlows)
                {
                    if (doDeflation_)
                        discounters_[cashFlowIndex].getFactors(LIBORRates_, Discounts_,stepToUse, deflatorAndDerivatives_); // get amount to discount cash flow by and amount to multiply its derivatives by

                    for (Size j=0; j < numberProducts_; ++j)
                    {
                        if (numberCashFlowsThisIndex_[j][cashFlowIndex] > 0)
                        {
                            Real deflatedCashFlow = totalCashFlowsThisIndex_[j][cashFlowIndex][0];
                            if (doDeflation_)
                                deflatedCashFlow *= deflatorAndDerivatives_[0];
                            //cashFlowsGenerated_[j][cashFlowIndex].amount[0]*deflatorAndDerivatives_[0];
                            numerairesHeld_[j] += deflatedCashFlow;

                            for (Size i=1; i <= numberRates_; ++i)
                            {
                                Real thisDerivative =  totalCashFlowsThisIndex_[j][cashFlowIndex][i];
                                if (doDeflation_)
                                {
                                    thisDerivative *= deflatorAndDerivatives_[0];
                                    thisDerivative +=  totalCashFlowsThisIndex_[j][cashFlowIndex][0]*deflatorAndDerivatives_[i];
                                    fullDerivatives_[i-1] = thisDerivative;
                                }
                                else
                                    fullDerivatives_[i-1] = thisDerivative;

                                V_[j][stepToUse][i-1] += thisDerivative; // zeroth row of V is t =0 not t_0
                            }

                            // ok we've got the derivatives and stored them, now add them to vegas
                            // this corresponds to the \frac{\partial F_n}[\partial theta} term
                            // we add the indirect terms later

                            for (Size k=0; k < numberBumps_; ++k)
                                for (Size i=0; i < numberRates_; ++i)
                                {
                                    vegasThisPath_[j][k] +=  fullDerivatives_[i]*jacobiansThisPaths_[stepToUse-1][k][i];
                                }


                        } // end of (numberCashFlowsThisIndex_[j][cashFlowIndex] > 0)
                    } // end of (Size j=0; j < numberProducts_; ++j)
                } // end of  if (!noFlows)
            }

            // need to do backwards updating
            if (flowsFound)
            {
                Integer nextStepToUse  = std::min<Integer>(currentStep-1, finalStepDone);
                Integer nextStepIndex = nextStepToUse+1;
                if (nextStepIndex != stepToUse) // then we need to update V
                {

                    const Matrix& thisPseudoRoot_= pseudoRootStructure_->pseudoRoot(currentStep);

                    for (Size i=0; i < numberProducts_; ++i)
                    {
                        // compute partials
                        for (Size f=0; f < factors; ++f)
                        {
                            Real libor = LIBORRates_[stepToUse][numberRates_-1];
                            Real V = V_[i][stepToUse][numberRates_-1];
                            Real pseudo = thisPseudoRoot_[numberRates_-1][f];
                            Real thisPartialTerm = libor*V*pseudo;
                            partials_[f][numberRates_-1] = thisPartialTerm;

                            for (Integer r = numberRates_-2; r >=0 ; --r)
                            {
                                Real thisPartialTermr = LIBORRates_[stepToUse][r]*V_[i][stepToUse][r]*thisPseudoRoot_[r][f];

                                partials_[f][r] = partials_[f][r+1] + thisPartialTermr;

                            }
                        } // end of (Size f=0; f < factors; ++f)

                        for (Size j=0; j < numberRates_; ++j)
                        {
                            Real nextV = V_[i][stepToUse][j] * LIBORRatios_[stepToUse][j];
                            V_[i][nextStepIndex][j] = nextV;

                            Real summandTerm = 0.0;
                            for (Size f=0; f < factors; ++f)
                                summandTerm += thisPseudoRoot_[j][f]*partials_[f][j];

                            summandTerm *= taus[j]*StepsDiscountsSquared_[stepToUse][j];

                            V_[i][nextStepIndex][j] += summandTerm;

                        } //end of  for (Size j=0; j < numberRates_; ++j)

                    // we've done the Vs now the vegas

                        if (nextStepIndex >0)

                            for (Size l=0; l < numberBumps_; ++l)
                                for (Size j=0; j < numberRates_; ++j)
                                    vegasThisPath_[i][l] +=  V_[i][nextStepIndex][j] * jacobiansThisPaths_[nextStepIndex-1][l][j];


                    } // end of (Size i=0; i < numberProducts_; ++i)



                } //  end of   if (nextStepIndex != stepToUse)
            } // end of  if (flowsFound)

        } // end of  for (Integer currentStep =  numberSteps_-1; currentStep >=0 ; --currentStep)

        // write answer into values

        Size entriesPerProduct = 1+numberRates_+numberBumps_;

        for (Size i=0; i < numberProducts_; ++i)
        {
            values[i*entriesPerProduct] = numerairesHeld_[i]*initialNumeraireValue_;
            for (Size j=0; j < numberRates_; ++j)
                values[i*entriesPerProduct+1+j] = V_[i][0][j]*initialNumeraireValue_;
            for (Size k=0; k < numberBumps_; ++k)
                values[i*entriesPerProduct + numberRates_ +k +1 ] = vegasThisPath_[i][k]*initialNumeraireValue_;
        }

        return 1.0; // we have put the weight in already, this results in lower variance since weight changes along the path
    }

    void PathwiseVegasAccountingEngine::multiplePathValues(std::vector<Real>& means, std::vector<Real>& errors,
        Size numberOfPaths)
    {
        std::vector<Real> values(product_->numberOfProducts()*(1+numberRates_+numberBumps_));
        means.resize(values.size());
        errors.resize(values.size());
        std::vector<Real> sums(values.size(),0.0);
        std::vector<Real> sumsqs(values.size(),0.0);



        for (Size i=0; i<numberOfPaths; ++i)
        {
            /* Real weight = */ singlePathValues(values);
            // stats.add(values,weight);
            for (Size j=0; j < values.size(); ++j)
            {
                sums[j] += values[j];
                sumsqs[j] += values[j]*values[j];

            }
        }

        for (Size j=0; j < values.size(); ++j)
            {
                means[j] = sums[j]/numberOfPaths;
                Real meanSq = sumsqs[j]/numberOfPaths;
                Real variance = meanSq - means[j]*means[j];
                errors[j] = std::sqrt(variance/numberOfPaths);

            }
    }

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    PathwiseVegasOuterAccountingEngine::PathwiseVegasOuterAccountingEngine(
        ext::shared_ptr<LogNormalFwdRateEuler> evolver, // method relies heavily on LMM Euler
        const Clone<MarketModelPathwiseMultiProduct>& product,
        ext::shared_ptr<MarketModel> pseudoRootStructure, // we need pseudo-roots and displacements
        const std::vector<std::vector<Matrix> >& vegaBumps,
        Real initialNumeraireValue)
    : evolver_(std::move(evolver)), product_(product),
      pseudoRootStructure_(std::move(pseudoRootStructure)), vegaBumps_(vegaBumps),
      initialNumeraireValue_(initialNumeraireValue), numberProducts_(product->numberOfProducts()),
      doDeflation_(!product->alreadyDeflated()), numerairesHeld_(product->numberOfProducts()),
      numberCashFlowsThisStep_(product->numberOfProducts()),
      cashFlowsGenerated_(product->numberOfProducts()),
      stepsDiscounts_(pseudoRootStructure_->numberOfRates() + 1),
      elementary_vegas_ThisPath_(product->numberOfProducts()),
      deflatorAndDerivatives_(pseudoRootStructure_->numberOfRates() + 1) {

        stepsDiscounts_[0]=1.0;

        numberRates_ = pseudoRootStructure_->numberOfRates();
        numberSteps_ = pseudoRootStructure_->numberOfSteps();
        factors_ = pseudoRootStructure_->numberOfFactors();
        fullDerivatives_.resize(numberRates_);


        const EvolutionDescription& evolution = pseudoRootStructure_->evolution();
        numeraires_ =  moneyMarketMeasure(evolution);


        QL_REQUIRE(vegaBumps.size() == numberSteps_, "we need precisely one vector of vega bumps for each step.");

        numberBumps_ = vegaBumps[0].size();

       std::vector<Matrix> jacobiansThisPathsModel;
       jacobiansThisPathsModel.reserve(numberRates_);
       for (Size i =0; i < numberRates_; ++i)
           jacobiansThisPathsModel.emplace_back(numberRates_, factors_);


        for (Size i =0; i < numberSteps_; ++i)
        {
            jacobianComputers_.emplace_back(
                pseudoRootStructure_->pseudoRoot(i), evolution.firstAliveRate()[i], numeraires_[i],
                evolution.rateTaus(), pseudoRootStructure_->displacements());

            // vector of vector of matrices to store jacobians of rates with respect to pseudo-root
            // elements
            jacobiansThisPaths_.push_back(jacobiansThisPathsModel);
        }



        Matrix VModel(numberSteps_+1,numberRates_);



        Discounts_ = Matrix(numberSteps_+1,numberRates_+1);

        for (Size i=0; i <= numberSteps_; ++i)
            Discounts_[i][0] = 1.0;


        V_.reserve(numberProducts_);

        Matrix  modelCashFlowIndex(product_->possibleCashFlowTimes().size(), numberRates_+1);


        numberCashFlowsThisIndex_.resize(numberProducts_);

        for (Size i=0; i<numberProducts_; ++i)
        {
            cashFlowsGenerated_[i].resize(
                product_->maxNumberOfCashFlowsPerProductPerStep());

            for (auto& j : cashFlowsGenerated_[i])
                j.amount.resize(numberRates_ + 1);

            numberCashFlowsThisIndex_[i].resize(product_->possibleCashFlowTimes().size());

            V_.push_back(VModel);


            totalCashFlowsThisIndex_.push_back(modelCashFlowIndex);
        }

        LIBORRatios_ = VModel;
        StepsDiscountsSquared_ = VModel;
        LIBORRates_ =VModel;




        const std::vector<Time>& cashFlowTimes =
            product_->possibleCashFlowTimes();
        numberCashFlowTimes_ = cashFlowTimes.size();

        const std::vector<Time>& rateTimes = product_->evolution().rateTimes();
        const std::vector<Time>& evolutionTimes = product_->evolution().evolutionTimes();
        discounters_.reserve(cashFlowTimes.size());

        for (Real cashFlowTime : cashFlowTimes)
            discounters_.emplace_back(cashFlowTime, rateTimes);


        // need to check that we are in money market measure


        // we need to allocate cash-flow times to steps, i.e. what is the last step completed before a flow occurs
        // what we really need is for each step, what cash flow time indices to look at

        cashFlowIndicesThisStep_.resize(numberSteps_);

        for (Size i=0; i < numberCashFlowTimes_; ++i)
        {
            auto it =
                std::upper_bound(evolutionTimes.begin(), evolutionTimes.end(), cashFlowTimes[i]);
            if (it != evolutionTimes.begin())
                --it;
            Size index = it - evolutionTimes.begin();
            cashFlowIndicesThisStep_[index].push_back(i);
        }

        partials_ = Matrix(pseudoRootStructure_->numberOfFactors(),numberRates_);

//      set up this container object        
//        std::vector<std::vector<std::vector<Matrix> >  > elementary_vegas_ThisPath_;  // dimensions are product, step, rate, rate and factor

        { // force destruction of modelVegaMatrix as soon as no longer needed 
            Matrix modelVegaMatrix(numberRates_, factors_,0.0);

            for (Size i=0; i < numberProducts_; ++i)
            {  
                elementary_vegas_ThisPath_[i].resize(numberSteps_);
                for (Size j=0; j < numberSteps_; ++j)
                {
                   
                        elementary_vegas_ThisPath_[i][j]= modelVegaMatrix;
                }
            }
        } // modelVegaMatrix destroyed here 

        numberElementaryVegas_ = numberSteps_*numberRates_*factors_;
/*
        gaussians_.resize(numberSteps_);
        distinguishedFactor_=0;
        distinguishedRate_=0;
        distinguishedStep_=0;
*/
    }

    Real PathwiseVegasOuterAccountingEngine::singlePathValues(std::vector<Real>& values)
    {

        const std::vector<Real>& initialForwards_(pseudoRootStructure_->initialRates());
        currentForwards_ = initialForwards_;
        // clear accumulation variables
        for (Size i=0; i < numberProducts_; ++i)
        {
            numerairesHeld_[i]=0.0;

            for (Size j=0; j < numberCashFlowTimes_; ++j)
            {
                numberCashFlowsThisIndex_[i][j] =0;

                for (Size k=0; k <= numberRates_; ++k)
                    totalCashFlowsThisIndex_[i][j][k] =0.0;
            }

            for (Size l=0;  l< numberRates_; ++l)
                for (Size m=0; m <= numberSteps_; ++m)
                    V_[i][m][l] =0.0;

        }



        Real weight = evolver_->startNewPath();
        product_->reset();

        Size thisStep;

        bool done = false;
        do 
        {
            thisStep = evolver_->currentStep();
            Size storeStep = thisStep+1;
            weight *= evolver_->advanceStep();

            done = product_->nextTimeStep(evolver_->currentState(),
                numberCashFlowsThisStep_,
                cashFlowsGenerated_);

            lastForwards_ = currentForwards_;
            currentForwards_ =  evolver_->currentState().forwardRates();

            for (unsigned long i=0; i < numberRates_; ++i)
            {
                Real x=  evolver_->currentState().discountRatio(i+1,i);
                stepsDiscounts_[i+1] = x;
                StepsDiscountsSquared_[storeStep][i] = x*x;

                LIBORRatios_[storeStep][i] = currentForwards_[i]/lastForwards_[i];
                LIBORRates_[storeStep][i] = currentForwards_[i];
                Discounts_[storeStep][i+1] = evolver_->currentState().discountRatio(i+1,0);
            }

            jacobianComputers_[thisStep].getBumps(lastForwards_,
                                         stepsDiscounts_,
                                         currentForwards_,
                                         evolver_->browniansThisStep(),
                                         jacobiansThisPaths_[thisStep]);

//            gaussians_[thisStep] = evolver_->browniansThisStep();



            // for each product...
            for (Size i=0; i<numberProducts_; ++i)
            {
                // ...and each cash flow...
                for (Size j=0; j<numberCashFlowsThisStep_[i]; ++j)
                {
                    Size k = cashFlowsGenerated_[i][j].timeIndex;
                    ++numberCashFlowsThisIndex_[i][ k];

                    for (Size l=0; l <= numberRates_; ++l)
                        totalCashFlowsThisIndex_[i][k][l] += cashFlowsGenerated_[i][j].amount[l]*weight;

                }
            }


        } while (!done);

        // ok we've gathered cash-flows, still have to backwards computation

        Size factors = pseudoRootStructure_->numberOfFactors();
        const std::vector<Time>& taus= pseudoRootStructure_->evolution(). rateTaus();

        bool flowsFound = false;

        Integer finalStepDone = thisStep;

        for (Integer currentStep =  numberSteps_-1; currentStep >=0 ; --currentStep) // must be a signed type as we go negative
        {
            Integer stepToUse = std::min<Integer>(currentStep, finalStepDone)+1;

            for (Size k=0; k < cashFlowIndicesThisStep_[currentStep].size(); ++k)
            {
                Size cashFlowIndex =cashFlowIndicesThisStep_[currentStep][k];

                // first check to see if anything actually happened before spending time on computing stuff
                bool noFlows = true;
                for (Size l=0; l < numberProducts_ && noFlows; ++l)
                    noFlows = noFlows && (numberCashFlowsThisIndex_[l][cashFlowIndex] ==0);

                flowsFound = flowsFound || !noFlows;

                if (!noFlows)
                {
                    if (doDeflation_)
                        discounters_[cashFlowIndex].getFactors(LIBORRates_, Discounts_,stepToUse, deflatorAndDerivatives_); // get amount to discount cash flow by and amount to multiply its derivatives by

                    for (Size j=0; j < numberProducts_; ++j)
                    {
                        if (numberCashFlowsThisIndex_[j][cashFlowIndex] > 0)
                        {
                            Real deflatedCashFlow = totalCashFlowsThisIndex_[j][cashFlowIndex][0];
                            if (doDeflation_)
                                deflatedCashFlow *= deflatorAndDerivatives_[0];
                            //cashFlowsGenerated_[j][cashFlowIndex].amount[0]*deflatorAndDerivatives_[0];
                            numerairesHeld_[j] += deflatedCashFlow;

                            for (Size i=1; i <= numberRates_; ++i)
                            {
                                Real thisDerivative =  totalCashFlowsThisIndex_[j][cashFlowIndex][i];
                                if (doDeflation_)
                                {
                                    thisDerivative *= deflatorAndDerivatives_[0];
                                    thisDerivative +=  totalCashFlowsThisIndex_[j][cashFlowIndex][0]*deflatorAndDerivatives_[i];
                                    fullDerivatives_[i-1] = thisDerivative;
                                }
                                else
                                    fullDerivatives_[i-1] = thisDerivative;

                                V_[j][stepToUse][i-1] += thisDerivative; // zeroth row of V is t =0 not t_0
                            } // end of  for (Size i=1; i <= numberRates_; ++i)
                        } // end of (numberCashFlowsThisIndex_[j][cashFlowIndex] > 0)
                    } // end of (Size j=0; j < numberProducts_; ++j)
                } // end of  if (!noFlows)
            }

            // need to do backwards updating
            if (flowsFound)
            {
                Integer nextStepToUse  = std::min<Integer>(currentStep-1, finalStepDone);
                Integer nextStepIndex = nextStepToUse+1;
                if (nextStepIndex != stepToUse) // then we need to update V
                {

                    const Matrix& thisPseudoRoot_= pseudoRootStructure_->pseudoRoot(currentStep);

                    for (Size i=0; i < numberProducts_; ++i)
                    {
                        // compute partials
                        for (Size f=0; f < factors; ++f)
                        {
                            Real libor = LIBORRates_[stepToUse][numberRates_-1];
                            Real V = V_[i][stepToUse][numberRates_-1];
                            Real pseudo = thisPseudoRoot_[numberRates_-1][f];
                            Real thisPartialTerm = libor*V*pseudo;
                            partials_[f][numberRates_-1] = thisPartialTerm;

                            for (Integer r = numberRates_-2; r >=0 ; --r)
                            {
                                Real thisPartialTermr = LIBORRates_[stepToUse][r]*V_[i][stepToUse][r]*thisPseudoRoot_[r][f];

                                partials_[f][r] = partials_[f][r+1] + thisPartialTermr;

                            }
                        } // end of (Size f=0; f < factors; ++f)

                        for (Size j=0; j < numberRates_; ++j)
                        {
                            Real nextV = V_[i][stepToUse][j] * LIBORRatios_[stepToUse][j];
                            V_[i][nextStepIndex][j] = nextV;

                            Real summandTerm = 0.0;
                            for (Size f=0; f < factors; ++f)
                                summandTerm += thisPseudoRoot_[j][f]*partials_[f][j];

                            summandTerm *= taus[j]*StepsDiscountsSquared_[stepToUse][j];

                            V_[i][nextStepIndex][j] += summandTerm;

                        } //end of  for (Size j=0; j < numberRates_; ++j)

                    } // end of (Size i=0; i < numberProducts_; ++i)

                } //  end of   if (nextStepIndex != stepToUse)

            } // end of  if (flowsFound)

        } // end of  for (Integer currentStep =  numberSteps_-1; currentStep >=0 ; --currentStep)


        // all V matrices computed we now compute the elementary vegas for this path 

        for (Size i=0; i < numberProducts_; ++i)
        {
                for (Size j=0; j < numberSteps_; ++j)
                {
                    Size nextIndex = j+1;

                    // we know V, we need to pair against the senstivity of the rate to the elementary vega
                    // note the simplification here arising from the fact that the elementary vega affects the evolution on precisely one step

                    for (Size k=0; k < numberRates_; ++k)
                        for (Size f=0; f < factors_; ++f)
                        {
                                Real sensitivity =0.0;

                                for (Size r=0; r < numberRates_; ++r)
                                {
                                        sensitivity += V_[i][nextIndex][r]*jacobiansThisPaths_[j][r][k][f];

                                  }
/*
                                  if (j ==distinguishedStep_ && k ==distinguishedRate_ &&f== distinguishedFactor_)
                                      std::cout << sensitivity << "," <<  jacobiansThisPaths_[j][j][k][f] << "," << gaussians_[j][f] << "," << V_[i][nextIndex][j] << "," << LIBORRates_[nextIndex][j] << "\n";
  */                


                                elementary_vegas_ThisPath_[i][j][k][f] = sensitivity;
                        }
                }
        }


        // write answer into values

        Size entriesPerProduct = 1+numberRates_+numberElementaryVegas_;

        for (Size i=0; i < numberProducts_; ++i)
        {
            values[i*entriesPerProduct] = numerairesHeld_[i]*initialNumeraireValue_;
            for (Size j=0; j < numberRates_; ++j)
                values[i*entriesPerProduct+1+j] = V_[i][0][j]*initialNumeraireValue_;

            for (Size k=0; k < numberSteps_; ++k)
                for (Size l=0; l < numberRates_; ++l)
                    for (Size m=0; m < factors_; ++m)
                        values[i*entriesPerProduct + numberRates_ +1 + m+ l*factors_ + k*numberRates_*factors_] = elementary_vegas_ThisPath_[i][k][l][m]*initialNumeraireValue_;

        }

        return 1.0; // we have put the weight in already, this results in lower variance since weight changes along the path
    
}

    void PathwiseVegasOuterAccountingEngine::multiplePathValuesElementary(std::vector<Real>& means, std::vector<Real>& errors,
        Size numberOfPaths)
    {
        Size numberOfElementaryVegas = numberRates_*numberSteps_*factors_;

        std::vector<Real> values(product_->numberOfProducts()*(1+numberRates_+numberOfElementaryVegas));
        means.resize(values.size());
        errors.resize(values.size());
        std::vector<Real> sums(values.size(),0.0);
        std::vector<Real> sumsqs(values.size(),0.0);



        for (Size i=0; i<numberOfPaths; ++i)
        {
          singlePathValues(values);
          
          for (Size j=0; j < values.size(); ++j)
            {
                sums[j] += values[j];
                sumsqs[j] += values[j]*values[j];

            }
        }

        for (Size j=0; j < values.size(); ++j)
            {
                means[j] = sums[j]/numberOfPaths;
                Real meanSq = sumsqs[j]/numberOfPaths;
                Real variance = meanSq - means[j]*means[j];
                errors[j] = std::sqrt(variance/numberOfPaths);

            }
    }

        void PathwiseVegasOuterAccountingEngine::multiplePathValues(std::vector<Real>& means, std::vector<Real>& errors,Size numberOfPaths)
        {
            std::vector<Real> allMeans;
            std::vector<Real> allErrors;

            multiplePathValuesElementary(allMeans,allErrors,numberOfPaths);

            Size outDataPerProduct = 1+numberRates_+numberBumps_;
            Size inDataPerProduct = 1+numberRates_+numberElementaryVegas_;

            means.resize((1+numberRates_+numberBumps_)*numberProducts_);
            errors.resize((1+numberRates_+numberBumps_)*numberProducts_); // post linear combinations, errors are not meaningful so don't attempt to compute s.e.s for vegas

            for (Size p=0; p < numberProducts_; ++p)
            {
                for (Size i=0; i < 1 + numberRates_; ++i)
                {
                      means[i+p*outDataPerProduct] = allMeans[i+p*inDataPerProduct];
                      errors[i+p*outDataPerProduct] = allErrors[i+p*inDataPerProduct];
                }

               for (Size bump=0; bump<numberBumps_; ++bump)
                {
                    Real thisVega=0.0;


                    for (Size t=0; t < numberSteps_; ++t)
                        for (Size r=0; r < numberRates_; ++r)
                            for (Size f=0; f < factors_; ++f)
                                thisVega+= vegaBumps_[t][bump][r][f]*allMeans[p*inDataPerProduct+1+numberRates_+t*numberRates_*factors_+r*factors_+f];


                    means[p*outDataPerProduct+1+numberRates_+bump] = thisVega;
               }
                    
            }

        } // end of method

} // end of namespace









