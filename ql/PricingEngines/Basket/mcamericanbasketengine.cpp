
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


namespace QuantLib {

    namespace {

        //! Basis function
        class BasisFunction : std::unary_function<std::vector<double>,double> {
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

        class BasisPower : public BasisFunction {
          private:
            Size index_;
            double power_;
            double coeff_;
          public:
            BasisPower(Size index, double power) : 
                        index_(index), power_(power), coeff_(1.0) {}
            BasisPower(Size index, double power, double coeff) : 
                        index_(index), power_(power), coeff_(coeff) {}
            double calculate(const std::vector<double>& x) const {
                return coeff_*QL_POW(x[index_], power_);
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

        class MyPolynomial : public BasisFunction {
          private:
            double factor_;
            Handle<std::vector<Handle<BasisFunction> > >
                basisFunctions_;
          public:
            MyPolynomial(double factor, 
                         const Handle<std::vector<Handle<BasisFunction> > >& 
                                        basisFunctions)
            : factor_(factor), basisFunctions_(basisFunctions) {}
            double calculate(const std::vector<double>& x) const;
        };

        double MyPolynomial::calculate(const std::vector<double>& x) const {
            double result = 0.0;
            for (Size j = 0; j<basisFunctions_->size(); j++) {
                result =+ (basisFunctions_->at(j))->calculate(x);
            }
            return factor_*result;
        }

        double basketPayoff(BasketOption::BasketType basketType, 
                            const std::vector<double>& assetPrices) {

            double basketPrice = assetPrices[0];
            Size numAssets = assetPrices.size();
            Size j;

            switch (basketType) {
              case BasketOption::Max:
                for (j = 1; j < numAssets; j++) {
                    if (assetPrices[j] > basketPrice) {
                        basketPrice = assetPrices[j];
                    }
                }
                break;

              case BasketOption::Min:
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

        // set up the basis functions        
        std::vector<Handle<BasisFunction> > basisFunctions;

        bool monomial = true;
        bool legendre = false;
        bool laguerre = false;

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
                    (new BasisPower(0, 4)));

                basisFunctions.push_back(Handle<BasisFunction> 
                    (new BasisPower(0, 5)));
 */
            } else if (legendre) {
                // legendre polynomials
                Handle<BasisFunction> legendre_0(new Constant(1));
                Handle<BasisFunction> legendre_1(new Linear(0));
                Handle<BasisFunction> legendre_2(new Polynomial 
                    (0.5, Handle<BasisFunction> (new Constant(-1)),
                            Handle<BasisFunction> (new Square(0, 3))));
                Handle<BasisFunction> legendre_3(new Polynomial 
                    (0.5, Handle<BasisFunction> (new Linear(0, -3)), 
                            Handle<BasisFunction> (new Cube(0, 5))));
                
                Handle<std::vector<Handle<BasisFunction> > > basis4(new
                    std::vector<Handle<BasisFunction> > (3) );
                basis4->at(0) = Handle<BasisFunction> (new Constant(3));
                basis4->at(1) = Handle<BasisFunction> (new Square(0, -30));
                basis4->at(2) = Handle<BasisFunction> (new BasisPower(0, 4, 35));
                Handle<BasisFunction> legendre_4(
                        new MyPolynomial(0.125, basis4));

                Handle<std::vector<Handle<BasisFunction> > > basis5(new
                    std::vector<Handle<BasisFunction> > (3) );                
                basis5->at(0) = Handle<BasisFunction> (new Linear(0,15));
                basis5->at(1) = Handle<BasisFunction> (new Cube(0, -70));
                basis5->at(2) = Handle<BasisFunction> (new BasisPower(0, 5, 63));
                Handle<BasisFunction> legendre_5(
                        new MyPolynomial(0.125, basis5));

                basisFunctions.push_back(legendre_0); 
                basisFunctions.push_back(legendre_1);
                basisFunctions.push_back(legendre_2);
                basisFunctions.push_back(legendre_3);
                basisFunctions.push_back(legendre_4);
                basisFunctions.push_back(legendre_5);

            } else if (laguerre) {
                // laguerre polynomials
                Handle<BasisFunction> laguerre_0(new Constant(1));
                Handle<BasisFunction> laguerre_1(new Polynomial 
                    (1, Handle<BasisFunction> (new Constant(1)), 
                        Handle<BasisFunction> (new Linear(0, -1))));

                Handle<std::vector<Handle<BasisFunction> > > basis2(new
                    std::vector<Handle<BasisFunction> > (3) );
                basis2->at(0) = Handle<BasisFunction> (new Constant(2));
                basis2->at(1) = Handle<BasisFunction> (new Linear(0, -4));
                basis2->at(2) = Handle<BasisFunction> (new Square(0, 1));
                Handle<BasisFunction> laguerre_2(
                    new MyPolynomial(0.5, basis2));

                Handle<std::vector<Handle<BasisFunction> > > basis3(new
                    std::vector<Handle<BasisFunction> > (4) );
                basis3->at(0) = Handle<BasisFunction> (new Constant(6));
                basis3->at(1) = Handle<BasisFunction> (new Linear(0, -18));
                basis3->at(2) = Handle<BasisFunction> (new Square(0, 9));
                basis3->at(3) = Handle<BasisFunction> (new Cube(0, -1));
                Handle<BasisFunction> laguerre_3(
                    new MyPolynomial(1.0/6.0, basis3));

                basisFunctions.push_back(laguerre_0); 
                basisFunctions.push_back(laguerre_1); 
                basisFunctions.push_back(laguerre_2); 
                basisFunctions.push_back(laguerre_3); 
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

        // create a Gaussian Random Sequence Generator
        GaussianRandomSequenceGenerator gen = 
            PseudoRandom::make_sequence_generator(
                                numAssets*(grid.size()-1),seed_);

        GaussianLowDiscrepancySequenceGenerator quasiGen = 
            LowDiscrepancy::make_sequence_generator(
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

        Handle<MultiPathGenerator<GaussianLowDiscrepancySequenceGenerator> >
            quasiMultipathGenerator(
            new MultiPathGenerator<GaussianLowDiscrepancySequenceGenerator> (
                                            diffusionProcs, 
                                            arguments_.correlation, grid, 
                                            quasiGen, brownianBridge));

        MultiPathGenerator<GaussianLowDiscrepancySequenceGenerator>
            ::sample_type quasiMultipathHolder = multipathGenerator->next(); 

        MultiPathGenerator<GaussianRandomSequenceGenerator>::sample_type 
            multipathHolder = multipathGenerator->next();

        bool isQuasi = false;
        if (isQuasi) {
            multipathHolder = quasiMultipathHolder;
        }

        // generate the paths
        MultiPath multipath = multipathHolder.value;
        std::vector<MultiPath> multipaths (N, multipath);
        for (i=0; i<N/2; i++) {
            multipathHolder = multipathGenerator->next();
            multipaths[i] = multipathHolder.value;
        }

        // get the asset values into an easy container
        std::vector<double> assetPath = getAssetSequence(
                        initialPrices[0], (multipaths[0])[0]); 
        // int assetPathLength = assetPath.size();
        std::vector< std::vector<double> >  temp_asset(numAssets, assetPath);

        std::vector<std::vector<std::vector<double> > > 
                multiAssetPaths(N, temp_asset);
        for (i=0; i<N/2; i++) {
            multipath = multipaths[i];
            for (j = 0; j < numAssets; j++) {
                multiAssetPaths[i][j] = 
                    getAssetSequence(initialPrices[j], multipath[j]);
                multiAssetPaths[N/2 + i][j] = 
                    getAntiAssetSequence(initialPrices[j], multipath[j]);
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
                        normalizedContinuationValue[i]*QL_EXP(-r * (to-from));
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

                // uncomment if you want to cut and paste
                // basis function coeffs and singular values
                // to Excel
                //std::cout << "\nb=" << b << "s=" << s ;

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

        Statistics stats = GeneralStatistics();
        for (i=0; i<N; i++) {
            stats.add(normalizedContinuationValue[i]*strike);
        }

        /*
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
        */

        results_.value  = stats.mean();
        results_.errorEstimate = stats.errorEstimate();
    }

    // put all the antithetic asset prices into a vector.
    // s0 is not included in the vector
    std::vector<double> getAntiAssetSequence (double s0, const Path& path) {
        Size n = path.size();
        QL_REQUIRE(n>0,
                   "AmericanMCEngine: the path cannot be empty");

        std::vector<double> asset(n);
        asset[0] = s0;

        double log_drift, log_random;
        log_drift = path.drift()[0];
        log_random = path.diffusion()[0];
        asset[0] = s0*QL_EXP(log_drift - log_random);

        for (Size i = 1; i < n; i++) {
            log_drift = path.drift()[i];
            log_random = path.diffusion()[i];
            asset[i] = asset[i-1]*QL_EXP(log_drift - log_random);
        }

        return asset;
    }

}

