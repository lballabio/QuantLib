
/*
 Copyright (C) 2004 Neil Firth 

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

/*! \file mcamericanbasketengine.cpp
    \brief Least-square Monte Carlo engines
*/

#include <functional>
#include <ql/Instruments/basketoption.hpp>
#include <ql/Math/statistics.hpp>
#include <ql/PricingEngines/Basket/mcamericanbasketengine.hpp>
#include <ql/PricingEngines/Vanilla/americanmcengines.hpp>


namespace {

    using QuantLib::Size;

    //! Basis function
    class BasisFunction : std::unary_function<std::vector<double>, double > {
      public:
        virtual ~BasisFunction() {}
        virtual double calculate(const std::vector<double>& x) const = 0;
    };

    class Constant : public BasisFunction {
      private:
        double constant_;
      public:
        Constant(double constant) : constant_(constant) {}
        double calculate(const std::vector<double>& x) const {
            return constant_;
        }
    };

    class Linear : public BasisFunction {
      private:
        Size index_;
        double coeff_;
      public:
        Linear(Size index) : index_(index), coeff_(1.0) {}
        Linear(Size index, double coeff) : index_(index), coeff_(coeff) {} 
        double calculate(const std::vector<double>& x) const {
            return coeff_*x[index_];
        }
    };

    class Square : public BasisFunction {
      private:
        Size index_;
        double coeff_;
      public:
        Square(Size index) : index_(index), coeff_(1.0) {} 
        Square(Size index, double coeff) : index_(index), coeff_(coeff) {} 
        double calculate(const std::vector<double>& x) const {
            return coeff_*x[index_]*x[index_];
        }
    };

    class Cube : public BasisFunction {
      private:
        Size index_;
        double coeff_;
      public:
        Cube(Size index) : index_(index), coeff_(1.0) {}
        Cube(Size index, double coeff) : index_(index), coeff_(coeff) {}
        double calculate(const std::vector<double>& x) const {
            return coeff_*x[index_]*x[index_]*x[index_];
        }
    };

    class LinearCombination : public BasisFunction {
      private:
        Size index1_, index2_;
      public:
        LinearCombination(Size index1, Size index2) 
        : index1_(index1), index2_(index2) {}
        double calculate(const std::vector<double>& x) const {
            return x[index1_]*x[index2_];
        }
    };

    class LinearCombo : public BasisFunction {
      private:
        Handle<BasisFunction> bf1_, bf2_;
      public:
        LinearCombo(const Handle<BasisFunction>& bf1, 
                    const Handle<BasisFunction>& bf2) 
        : bf1_(bf1), bf2_(bf2) {}
        double calculate(const std::vector<double>& x) const {
            return bf1_->calculate(x)*bf2_->calculate(x);
        }
    };

    class Polynomial : public BasisFunction {
      private:
        double factor_;
        Handle<BasisFunction> bf1_, bf2_;
      public:
        Polynomial(double factor, 
                   const Handle<BasisFunction>& bf1, 
                   const Handle<BasisFunction>& bf2) 
        : factor_(factor), bf1_(bf1), bf2_(bf2) {}
        double calculate(const std::vector<double>& x) const {
            return factor_*(bf1_->calculate(x) + bf2_->calculate(x));
        }
    };


    double basketPayoff (QuantLib::BasketOption::BasketType basketType, 
                         const std::vector<double>& assetPrices) {

        double basketPrice = assetPrices[0];
        QuantLib::Size numAssets = assetPrices.size();
        QuantLib::Size j = 0;

        switch (basketType) {
          case QuantLib::BasketOption::Max:
            for (j = 1; j < numAssets; j++) {
                if (assetPrices[j] > basketPrice) {
                    basketPrice = assetPrices[j];
                }
            }
            break;
          case QuantLib::BasketOption::Min:
            for (j = 1; j < numAssets; j++) {
                if (assetPrices[j] < basketPrice) {
                    basketPrice = assetPrices[j];
                }
            }
            break;
        }

        return basketPrice;
    }

}

namespace QuantLib {

    // calculate
    void MCAmericanBasketEngine::calculate() const {

        #if defined(HAVE_BOOST)
        Handle<PlainVanillaPayoff> payoff_handle =
            boost::dynamic_pointer_cast<PlainVanillaPayoff>(arguments_.payoff);
        QL_REQUIRE(payoff_handle,
                   "MCAmericanBasketEngine: non-plain payoff given");
        #else
        Handle<PlainVanillaPayoff> payoff_handle = arguments_.payoff;
        #endif

        double strike = payoff_handle->strike();
        PlainVanillaPayoff payoff(payoff_handle->optionType(), strike);

        Size numAssets = arguments_.blackScholesProcesses.size();

        bool brownianBridge = false;

        Rate r = arguments_.blackScholesProcesses[0]->riskFreeTS->zeroYield(
            arguments_.exercise->lastDate());

        // counters
        Size i; // for paths
        Size j; // for assets
        Size k; // for basis functions
        Size timeStep; // for time loops

        // Number of paths
        Size N = requiredSamples_;

        // normalize the prices?
        // bool normalize = true;

        // set up the basis functions        
        std::vector<Handle<BasisFunction> > basisFunctions;

        bool monomial = false;
        if (numAssets == 1) {
            if (monomial) {
                // monomials
                basisFunctions.push_back(Handle<BasisFunction> 
                    (new Constant(1)));

                basisFunctions.push_back(Handle<BasisFunction> 
                    (new Linear(0)));

                basisFunctions.push_back(Handle<BasisFunction> 
                    (new Square(0)));
                /*
                basisFunctions.push_back(Handle<BasisFunction> 
                    (new Cube(0)));

                basisFunctions.push_back(Handle<BasisFunction> 
                    (new LinearCombo
                        (Handle<BasisFunction>(new Square(0)), 
                         Handle<BasisFunction>(new Square(0)))));

                basisFunctions.push_back(Handle<BasisFunction> 
                    (new LinearCombo
                        (Handle<BasisFunction>(new Square(0)), 
                         Handle<BasisFunction>(new Cube(0)))));
                */
            } else {
                // legendre polynomials
                basisFunctions.push_back(Handle<BasisFunction> 
                    (new Constant(1)));

                basisFunctions.push_back(Handle<BasisFunction> 
                    (new Linear(0)));

                basisFunctions.push_back(Handle<BasisFunction> 
                    (new Polynomial(0.5, 
                                    Handle<BasisFunction>(new Constant(-1)), 
                                    Handle<BasisFunction>(new Square(0, 3)))));

                /*
                basisFunctions.push_back(Handle<BasisFunction> 
                    (new Polynomial(0.5, 
                                    Handle<BasisFunction>(new Linear(0, -3)), 
                                    Handle<BasisFunction>(new Cube(0, 5)))));
                */
            }

        } else if (numAssets == 3) {

            basisFunctions.push_back(Handle<BasisFunction> 
                (new Constant(1)));

            basisFunctions.push_back(Handle<BasisFunction> 
                (new Linear(1)));
            basisFunctions.push_back(Handle<BasisFunction> 
                (new Linear(2)));

            basisFunctions.push_back(Handle<BasisFunction> 
                (new Square(1)));
            basisFunctions.push_back(Handle<BasisFunction> 
                (new Square(2)));

            basisFunctions.push_back(Handle<BasisFunction> 
                (new Cube(1)));
            basisFunctions.push_back(Handle<BasisFunction> 
                (new Cube(2)));

            basisFunctions.push_back(Handle<BasisFunction> 
                (new LinearCombo(Handle<BasisFunction>(new Square(1)), 
                                 Handle<BasisFunction>(new Square(1)))));
            basisFunctions.push_back(Handle<BasisFunction> 
                (new LinearCombo(Handle<BasisFunction>(new Square(2)), 
                                 Handle<BasisFunction>(new Square(2)))));

            basisFunctions.push_back(Handle<BasisFunction> 
                (new LinearCombo(Handle<BasisFunction>(new Square(1)), 
                                 Handle<BasisFunction>(new Cube(1)))));
            basisFunctions.push_back(Handle<BasisFunction> 
                (new LinearCombo(Handle<BasisFunction>(new Square(2)), 
                                 Handle<BasisFunction>(new Cube(2)))));

            basisFunctions.push_back(Handle<BasisFunction> 
                (new LinearCombination(1,2)));

            basisFunctions.push_back(Handle<BasisFunction> 
                (new LinearCombo(Handle<BasisFunction>(new Linear(1)), 
                                 Handle<BasisFunction>(new Square(2)))));
            basisFunctions.push_back(Handle<BasisFunction> 
                (new LinearCombo(Handle<BasisFunction>(new Linear(2)), 
                                 Handle<BasisFunction>(new Square(1)))));

            basisFunctions.push_back(Handle<BasisFunction> 
                (new LinearCombo(Handle<BasisFunction>(new Square(1)), 
                                 Handle<BasisFunction>(new Square(2)))));

            basisFunctions.push_back(Handle<BasisFunction> 
                (new LinearCombo(Handle<BasisFunction>(new Linear(1)), 
                                 Handle<BasisFunction>(new Cube(2)))));
            basisFunctions.push_back(Handle<BasisFunction> 
                (new LinearCombo(Handle<BasisFunction>(new Linear(2)), 
                                 Handle<BasisFunction>(new Cube(1)))));

            basisFunctions.push_back(Handle<BasisFunction> 
                (new LinearCombo(Handle<BasisFunction>(new Square(1)), 
                                 Handle<BasisFunction>(new Cube(2)))));
            basisFunctions.push_back(Handle<BasisFunction> 
                (new LinearCombo(Handle<BasisFunction>(new Square(2)), 
                                 Handle<BasisFunction>(new Cube(1)))));

            basisFunctions.push_back(Handle<BasisFunction> 
                (new LinearCombo(Handle<BasisFunction>(new Linear(1)), 
                                 Handle<BasisFunction>
                                 (new LinearCombo(Handle<BasisFunction>(
                                                            new Cube(2)),
                                                  Handle<BasisFunction>(
                                                            new Cube(2)))))));
            basisFunctions.push_back(Handle<BasisFunction> 
                (new LinearCombo(Handle<BasisFunction>(new Linear(2)), 
                                 Handle<BasisFunction>
                                 (new LinearCombo(Handle<BasisFunction>(
                                                            new Cube(2)),
                                                  Handle<BasisFunction>(
                                                            new Cube(2)))))));
        }

        Size numBasisFunctions = basisFunctions.size();

        // create the time grid
        Time T = arguments_.blackScholesProcesses[0]->riskFreeTS->
                    dayCounter().yearFraction( 
                        arguments_.blackScholesProcesses[0]->
                            riskFreeTS->referenceDate(),
                        arguments_.exercise->lastDate());
        TimeGrid grid(T, timeSteps_);
        // int gridLength = grid.size();

        // create a Gaussian Random Sequence Generator
        GaussianRandomSequenceGenerator gen = 
            PseudoRandom::make_sequence_generator(
                                numAssets*(grid.size()-1),seed_);

        // set up the diffuction processes
        std::vector<double> initialPrices (numAssets);
        std::vector<Handle<DiffusionProcess> > diffusionProcs;
        for (j = 0; j < numAssets; j++) { 
            initialPrices[j] = arguments_.blackScholesProcesses[j]
                ->stateVariable->value();
            Handle<DiffusionProcess> bs(new
              BlackScholesProcess(
                arguments_.blackScholesProcesses[j]->riskFreeTS,
                arguments_.blackScholesProcesses[j]->dividendTS,
                arguments_.blackScholesProcesses[j]->volTS,
                initialPrices[j]));
            diffusionProcs.push_back(bs);
        }

        // create the MultiPathGenerator
        Handle<MultiPathGenerator<GaussianRandomSequenceGenerator> > 
            multipathGenerator(
                new MultiPathGenerator<GaussianRandomSequenceGenerator> (
                                            diffusionProcs, 
                                            arguments_.correlation, grid, 
                                            gen, brownianBridge));

        MultiPathGenerator<GaussianRandomSequenceGenerator>::sample_type 
            multipathHolder = multipathGenerator->next();

        // generate the paths
        MultiPath multipath = multipathHolder.value;
        std::vector<MultiPath> multipaths (N, multipath);
        for (i=0; i<N; i++) {
            multipathHolder = multipathGenerator->next();
            multipaths[i] = multipathHolder.value;
        }

        // get the asset values into an easy container
        std::vector<double> assetPath = getAssetSequence(
                        initialPrices[0], (multipaths[0])[0]); 
        // int assetPathLength = assetPath.size();
        std::vector< std::vector<double> >  temp_asset(numAssets, assetPath);
        std::vector<std::vector<std::vector<double> > > multiAssetPaths(N, temp_asset);
        for (i=0; i<N; i++) {
            multipath = multipaths[i];
            for (j = 0; j < numAssets; j++) {
                multiAssetPaths[i][j] = getAssetSequence(initialPrices[j], multipath[j]);
            }
        }

        // initialise rollback vector with payoff        
        std::vector<double> normalizedContinuationValue(N);
        for (i=0; i<N; i++) {
            std::vector<double> finalPrices(numAssets);
            for (j=0; j<numAssets; j++) {
                finalPrices[j] = multiAssetPaths[i][j][timeSteps_-1];
            }
            normalizedContinuationValue[i] = payoff(basketPayoff(
                        arguments_.basketType, finalPrices))/strike; 
        }

        Array temp_coeffs(numBasisFunctions,1.0);
        std::vector<Array> basisCoeffs(timeSteps_-1, temp_coeffs);

        std::vector<double> assetPrices(numAssets);
        std::vector<double> normalizedAssetPrices(numAssets);

        // LOOP   
        int timeLoop;
        for (timeLoop = timeSteps_-2; timeLoop>=0; timeLoop--) {
            timeStep = timeLoop;

            // rollback all paths
            // use rollback vector
            for (i=0; i<N; i++) {
                if (normalizedContinuationValue[i] > 0.0) {
                    // discount 
                    // +1 because the grid includes the start time
                    Time from = grid[timeStep+1];
                    Time to = grid[timeStep+2];

                    normalizedContinuationValue[i] = 
                        normalizedContinuationValue[i]* QL_EXP(-r * (to-from));
                }
            }

            // select in the money paths            
            std::vector<int> itmPaths;
            std::vector<double> y(N);
            for (i=0; i<N; i++) {
                for (j=0; j<numAssets; j++) {
                    assetPrices[j] = multiAssetPaths[i][j][timeStep];
                }
                y[i] = payoff(basketPayoff(arguments_.basketType, 
                                           assetPrices));
                if (y[i] > 0) {
                    itmPaths.push_back(i);
                }
            }

            // loop over all itm paths
            Size num_itmPaths = itmPaths.size();
            if (num_itmPaths > 0) {

                // for itm paths   
                std::vector<double> y_exercise(num_itmPaths);
                Array y_temp(num_itmPaths);
                for (i=0; i<num_itmPaths; i++) {
                    // get the immediate exercise value                
                    // normalized..                    
                    y_exercise[i] = y[itmPaths[i]] / strike;

                    // get discounted continuation value                    
                    y_temp[i] = normalizedContinuationValue[itmPaths[i]]; 
                }

                // calculate the basis functions and 
                // create the design matrix A                                
                Matrix A(num_itmPaths,numBasisFunctions);
                for (i=0; i<num_itmPaths; i++) {

                    for (j=0; j<numAssets; j++) {
                        normalizedAssetPrices[j] = 
                            multiAssetPaths[itmPaths[i]][j][timeStep]/strike;
                    }
                    // sort - ascending order
                    std::sort(normalizedAssetPrices.begin(), 
                                normalizedAssetPrices.end());

                    for (k=0; k<numBasisFunctions; k++) {
                        A[i][k] = 
                          basisFunctions[k]->calculate(normalizedAssetPrices);
                    }
                }

                // do least squares regression                
                SVD svd(A);
                Matrix U(A.rows(), A.rows());
                Array s(A.columns());
                Matrix V(A.columns(), A.columns());
                svd.getU(U);
                svd.getV(V);
                svd.getSingularValues(s);
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

                // store the basisCoeffs for later
                basisCoeffs[timeStep] = b;

                // calculate continuation value
                Array y_continue = A*b;

                // modify stopping rule                
                for (i=0; i<num_itmPaths; i++) {
                    if (y_exercise[i] > y_continue[i]) {
                        normalizedContinuationValue[itmPaths[i]] = 
                                y_exercise[i];
                    }
                }

            }
            // END LOOP
        }

        std::cout << std::endl;
        std::cout << " ==== Basis Coeffs ===  " << std::endl;
        for (timeStep = 0; timeStep < basisCoeffs.size(); timeStep++) {
            Array b = basisCoeffs[timeStep]; 
            std::cout << " [";
            for (k = 0; k < numBasisFunctions; k++) {
                std::cout << " " << b[k];
            }
            std::cout << " ]" << std::endl;
        }

        // evaluate the option using the continuation rule and 
        // basis function definitions
        // could use a new set of paths here   
        GeneralStatistics stats;
        double value = 0.0;
        double exerciseValue = 0.0;
        double continueValue = 0.0;
        for (i = 0; i < N; i++) {
            bool exercised = false;

            for (timeStep = 0; timeStep < timeSteps_-1; timeStep++) {
                //std::cout << " ==== TIMESTEP ===== " << timeStep << std::endl;

                for (j = 0; j < numAssets; j++) {
                    assetPrices[j] = multiAssetPaths[i][j][timeStep];
                }

                exerciseValue = payoff(basketPayoff(
                        arguments_.basketType, assetPrices));

                if (exerciseValue > 0.0) {
                    Array b = basisCoeffs[timeStep];
                    //std::cout << " [ ";
                    //for (Size kk = 0; kk < numBasisFunctions; kk++) {
                    //    double bcoeff = b[kk];
                    //    std::cout << bcoeff << " ";
                    //}
                    //std::cout << "] " << std::endl;

                    for (j = 0; j < numAssets; j++) {
                        normalizedAssetPrices[j] = assetPrices[j] / strike;
                    }

                    // sort - ascending order
                    std::sort(normalizedAssetPrices.begin(), 
                                normalizedAssetPrices.end());

                    continueValue = 0.0;
                    for (k = 0; k < numBasisFunctions; k++) {
                        continueValue =+ b[k]*(basisFunctions[k]->
                                calculate(normalizedAssetPrices));
                    }
                    continueValue = continueValue * strike;

                    if (exerciseValue >= continueValue) {
                        // discount              
                        // time grid, so +1
                        Time to = grid[timeStep+1];
                        value = exerciseValue * arguments_.blackScholesProcesses[0]
                                    ->riskFreeTS->discount(to);
                        stats.add(value);
                        exercised = true;
                        break;
                    }
                }

            }
            if (!exercised) {
                for (j = 0; j < numAssets; j++) {
                    assetPrices[j] = multiAssetPaths[i][j][timeSteps_-1];
                }
                exerciseValue = payoff(basketPayoff(arguments_.basketType, assetPrices)) *
                        arguments_.blackScholesProcesses[0]->riskFreeTS->discount(
                            arguments_.exercise->lastDate());
                stats.add(exerciseValue);
            }
        }

        results_.value  = stats.mean();
        results_.errorEstimate = stats.errorEstimate();
    }

    // put all the asset prices into a vector.
    // s0 is not included in the vector
    /*std::vector<double> getAssetSequence (double s0, const Path& path) {
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
*/
}

