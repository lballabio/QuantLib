
/*
 Copyright (C) 2003 Neil Firth 

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file americanmcengines.cpp
    \brief Least-square Monte Carlo engines
*/

#include <ql/PricingEngines/Vanilla/americanmcengines.hpp>

namespace QuantLib {

    // calculate
    void AmericanMCVanillaEngine::calculate() const {
        // get the parameters
        #if defined(HAVE_BOOST)
        Handle<PlainVanillaPayoff> arg_payoff = 
            boost::dynamic_pointer_cast<PlainVanillaPayoff>(arguments_.payoff);
        QL_REQUIRE(arg_payoff,
                   "AmericanMCVanillaEngine: non-plain payoff given");
        #else
        Handle<PlainVanillaPayoff> arg_payoff = arguments_.payoff;
        #endif

        Option::Type type = arg_payoff->optionType();
        double s0 = arguments_.blackScholesProcess->stateVariable->value();
        double strike  = arg_payoff->strike();

        // double          vol     = arguments_.blackScholesProcess->volTS->blackVol(
        //     arguments_.exercise->lastDate(), s0);
        Rate r = arguments_.blackScholesProcess->riskFreeTS->zeroYield(
            arguments_.exercise->lastDate());
        // Rate q = arguments_.blackScholesProcess->dividendTS->zeroYield(
        //     arguments_.exercise->lastDate());

        // unsigned long   seed    = 1000L;
        PlainVanillaPayoff payoff(type, strike);

        // counter
        Size i;
        Size j;
        // Size k = 0;

        // Number of paths
        Size N = requiredSamples_;

        // Longstaff Schwartz parameters
        /* r = 0.06;
           T = 3;
           s0 = 1.0;
           strike = 1.1;
           type = Option::Type::Put;
           payoff  = PlainPayoff(type, strike);
        */

        Size timeStep;

        // simulate the paths
        Handle<DiffusionProcess> bs(new
            BlackScholesProcess(arguments_.blackScholesProcess->riskFreeTS, 
                                arguments_.blackScholesProcess->dividendTS,
                                arguments_.blackScholesProcess->volTS, 
                                s0));

        Time T = arguments_.blackScholesProcess->riskFreeTS->dayCounter().yearFraction(
            arguments_.blackScholesProcess->riskFreeTS->referenceDate(),
            arguments_.exercise->lastDate());
        // Exercise dates
        TimeGrid grid(T, timeSteps_);

        GaussianRandomSequenceGenerator gen = 
            PseudoRandom::make_sequence_generator(grid.size()-1,seed_);

        // BB here
        Handle<GaussianPathGenerator> pathGenerator(new
            GaussianPathGenerator(bs, grid, gen, true));

        GaussianPathGenerator::sample_type pathHolder = 
            pathGenerator->next();

        Path p = pathHolder.value;
        std::vector<Path> paths (N,p);
        for (i=0; i<N; i++) {
            pathHolder = pathGenerator->next();
            paths[i] = pathHolder.value;
        }

        // get the asset values into an easy container
        std::vector<double> asset = getAssetSequence(s0, paths[0]); 
        // std::vector<std::vector<double> > assetPaths (N, asset);

        AssetGrid assetPaths (N, asset);
        for (i=1; i<N; i++) { 
            assetPaths[i] = getAssetSequence(s0, paths[i]); 
        }

        // change the generated asset prices for the Longstaff-Schwartz
        // example asset prices
        // AssetGrid assetPaths (N, std::vector<double>(timeSteps_));
        // getLSAssetsExample(assetPaths, timeSteps_);


        // Payoff matrix
        // can an existing lattice class be used here?
        // initialise with payoff
        Size payoffGridSize = timeSteps_;
        PayoffGrid payoffMatrix (N, std::vector<double>(payoffGridSize)); 
        for (i=0; i<N; i++) {
            std::vector<double> cashflows = payoffMatrix[i];
            for (j=0; j<cashflows.size()-1; j++) {
                cashflows[j] = 0.0;
            }
            payoffMatrix[i][cashflows.size()-1] = 
                payoff(assetPaths[i][cashflows.size()-1]);
        }

        // LOOP
        int timeLoop;
        for (timeLoop = timeSteps_-2; timeLoop>=0; timeLoop--) {
            timeStep = timeLoop;


            //EuropeanSubPathPricer* pathPricer = 
            //    new EuropeanSubPathPricer(
            //    type, s0, strike, arguments_.riskFreeTS);

            // select in the money paths
            std::vector<int> itmPaths;
            std::vector<double> y(N);
            for (i=0; i<N; i++) {
                double s = assetPaths[i][timeStep];
                double temp  = payoff(s);
                y[i] = temp;
                if (y[i]>0) {
                    itmPaths.push_back(i);
                }
            }

            if (itmPaths.size() > 0) {
                // get the immediate exercise value                
                std::vector<double> y_exercise(itmPaths.size());
                for (i=0; i<itmPaths.size(); i++) {
                    y_exercise[i] = y[itmPaths[i]];
                }

                // discount itm cash flows to current time step
                // for each itm cash flow
                Array y_temp(itmPaths.size());
                for (i=0; i<itmPaths.size(); i++) {

                    // find any payoffs
                    std::vector<double> cashflows = payoffMatrix[itmPaths[i]];
                    double cashflow = 0.0;
                    int cashflowTime = -1;
                    for (j = timeStep; j<timeSteps_; j++) {
                        if (cashflows[j] > 0.0) {
                            cashflow = cashflows[j];
                            cashflowTime = j;
                            break;
                        }
                    }

                    if (cashflowTime >= 0) {
                        // discount 
                        // +1 because the grid includes the start time
                        Time from = grid[timeStep+1];
                        Time to = grid[cashflowTime+1];
                        //Rate forward = arguments_.riskFreeTS_->forward(from, to);

                        // store result in y vector
                        y_temp[i] = cashflow * QL_EXP(-r * (to-from));

                        //Path p = paths_[itmPaths[i]];
                        //double s = p->
                        //y[i] = pathPricer->subPathValue(p, i, N);                    

                    } else {
                        y_temp[i] = 0.0;
                    }
                }

                // calculate the basis functions and 
                // create the design matrix A
                Size numBasisFunctions = 3;
                Matrix A(itmPaths.size(),numBasisFunctions);
                for (i=0; i<itmPaths.size(); i++) {
                    //for (j=0; j<numBasisFunctions; j++) {
                    double assetPrice = 
                        (assetPaths[itmPaths[i]])[timeStep];
                    A[i][0] = 1;
                    A[i][1] = assetPrice;
                    A[i][2] = assetPrice*assetPrice;

                    //}
                }



                    // do least squares regression                
                    SVD svd(A);
                    Matrix U = svd.U();
                    Array s = svd.singularValues();
                    Matrix V = svd.V();
                    // probably faster to do this directly
                    // in MATLAB this is V*(S\(U'*y_temp))
                    Matrix Utrans = transpose(U);
                    Array temp_1 = transpose(U) * y_temp;
                    Array temp_2(V.columns());
                    // Some singular values may be zero so we cannot do
                    // temp_1/=s;
                    for (i=0; i<temp_2.size(); i++) {
                        if (s[i] != 0.0) {
                            temp_1[i] = temp_1[i]/s[i];
                        } else {
                            temp_1[i] = 0.0;
                        }
                    }
                    Array b = V*temp_1;

                    // calculate continuation value
                    Array y_continue = A*b;

                    // modify stopping rule
                    // ensure that only one cashflow on a path is non zero
                    for (i=0; i<itmPaths.size(); i++) {
                        if (y_exercise[i] > y_continue[i]) {
                            for (j=0; j<payoffMatrix[i].size(); j++) {
                                payoffMatrix[itmPaths[i]][j] = 0.0;
                            }
                            payoffMatrix[itmPaths[i]][timeStep] = 
                                y_exercise[i];
                        }
                    }

            }
            // END LOOP
        }

        double total = 0.0;
        // discount paths using stopping rule
        for (timeLoop = timeSteps_-1; timeLoop>=0; timeLoop--) {
            // sum column - may well be more efficient to 
            // discount each path separately
            for (j=0; j<N; j++) {
                total += payoffMatrix[j][timeLoop];
            }

            // discount
            // could / should use rate term stucture
            Time from = grid[timeLoop];
            Time to = grid[timeLoop+1];
            //Rate forward = arguments_.riskFreeTS_->forward(from, to);
            total *= QL_EXP(-r * (to-from));
        }

        results_.value = total/N;
    }

    // put all the asset prices into a vector.
    // s0 is not included in the vector
    std::vector<double> getAssetSequence (double s0, const Path& path) {
        Size n = path.size();
        QL_REQUIRE(n>0,
                   "AmericanMCEngine: the path cannot be empty");

        std::vector<double> asset(n);
        asset[0] = s0;

        double log_drift, log_random;
        log_drift = path.drift()[0];
        log_random = path.diffusion()[0];
        asset[0] = s0*QL_EXP(log_drift + log_random);

        for (Size i = 1; i < n; i++) {
            log_drift = path.drift()[i];
            log_random = path.diffusion()[i];
            asset[i] = asset[i-1]*QL_EXP(log_drift + log_random);
        }

        return asset;
    }

    void getLSAssetsExample(AssetGrid& assetPaths, int timeSteps) {
        Size n = assetPaths.size();
        QL_REQUIRE(n == 8,
                   "AmericanMCEngine: Longstaff Schwartz example "
                   "must have 8 paths");

        QL_REQUIRE(timeSteps == 3,
                   "AmericanMCEngine: Longstaff Schwartz 3 time steps");

        assetPaths[0][0] = 1.09;
        assetPaths[0][1] = 1.08;
        assetPaths[0][2] = 1.34;

        assetPaths[1][0] = 1.16;
        assetPaths[1][1] = 1.26;
        assetPaths[1][2] = 1.54;

        assetPaths[2][0] = 1.22;
        assetPaths[2][1] = 1.07;
        assetPaths[2][2] = 1.03;

        assetPaths[3][0] = 0.93;
        assetPaths[3][1] = 0.97;
        assetPaths[3][2] = 0.92;

        assetPaths[4][0] = 1.11;
        assetPaths[4][1] = 1.56;
        assetPaths[4][2] = 1.52;

        assetPaths[5][0] = 0.76;
        assetPaths[5][1] = 0.77;
        assetPaths[5][2] = 0.90;

        assetPaths[6][0] = 0.92;
        assetPaths[6][1] = 0.84;
        assetPaths[6][2] = 1.01;

        assetPaths[7][0] = 0.88;
        assetPaths[7][1] = 1.22;
        assetPaths[7][2] = 1.34;

    }

}

