
/*
 Copyright (C) 2003 Neil Firth

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email ferdinando@ametrano.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file mcengine.hpp
    \brief base class for Monte Carlo engines
*/

#ifndef quantlib_american_montecarlo_engine_h
#define quantlib_american_montecarlo_engine_h

#include <ql/dataformatters.hpp>
#include <ql/grid.hpp>
#include <ql/payoff.hpp>
#include <ql/Math/matrix.hpp>
#include <ql/Math/svd.hpp>
#include <ql/MonteCarlo/montecarlomodel.hpp>
#include <ql/MonteCarlo/mctypedefs.hpp>
#include <ql/MonteCarlo/europeanpathpricer.hpp>
#include <ql/RandomNumbers/rngtypedefs.hpp>
#include <ql/PricingEngines/vanillaengines.hpp>
//#include <ql/Volatilities/blackconstantvol.hpp>

//using MonteCarlo::GaussianPathGenerator;
using std::vector;
using QuantLib::Math::Matrix;
using QuantLib::Math::SVD;

namespace QuantLib {

    namespace PricingEngines {

        // asset grid
        typedef vector< vector<double> > AssetGrid;

        // payoff grid
        typedef vector< vector<double> > PayoffGrid;        

        class AmericanMCVanillaEngine : public VanillaEngine {
            public:
                //AmericanMCVanillaEngine() {}
                AmericanMCVanillaEngine(Size requiredSamples = Null<int>(), 
                                        long seed = 0);
                void calculate() const;  
            private:
                Size requiredSamples_; 
                //maxSamples_;
                //double requiredTolerance_;
                long seed_;
        };

        // constructor
        inline
        AmericanMCVanillaEngine::AmericanMCVanillaEngine (Size requiredSamples,
                                                          long seed) :
            requiredSamples_(requiredSamples),
            seed_(seed) {}

        // put all the asset prices into a vector.
        // s0 is not included in the vector
        vector<double> getAssetSequence (const double s0,
                                         const MonteCarlo::Path& path);
        
        // get the Longstaff-Schwartz exampe asset prices
        // s0 is not included in the vector
        void getLSAssetsExample (AssetGrid& assetPaths,
                                 const int timeSteps);
    }
}


#endif
