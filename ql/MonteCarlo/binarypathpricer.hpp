
/*
 Copyright (C) 2003 Neil Firth
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

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

/*! \file binarypathpricer.hpp
    \brief path pricer for Binary options
*/

#ifndef quantlib_montecarlo_binary_path_pricer_h
#define quantlib_montecarlo_binary_path_pricer_h

#include <ql/Instruments/binaryoption.hpp>
#include <ql/MonteCarlo/pathpricer.hpp>
#include <ql/MonteCarlo/path.hpp>
#include <ql/RandomNumbers/rngtypedefs.hpp>
#include <ql/diffusionprocess.hpp>
#include <ql/payoff.hpp>

namespace QuantLib {

    namespace MonteCarlo {
        
        //! %path pricer for Binary options
        /* %Path pricer for Binary options
            
            Uses the Brownian Bridge correction for the barrier found in

            Going to Extremes: Correcting Simulation Bias in Exotic
              Option Valuation - D.R. Beaglehole, P.H. Dybvig and G. Zhou
                Financial Analysts Journal; Jan/Feb 1997; 53, 1. pg. 62-68

            and

            Simulating path-dependent options: A new approach - 
              M. El Babsiri and G. Noel
                Journal of Derivatives; Winter 1998; 6, 2; pg. 65-83

        */
        class BinaryPathPricer : public PathPricer<Path> {
          public:
            BinaryPathPricer(
                Binary::Type binaryType, 
                double barrier,                 
                double cashPayoff,
                Option::Type type,
                double underlying,                
                const RelinkableHandle<TermStructure>& riskFreeTS,
                const Handle<DiffusionProcess>& diffProcess,
                RandomNumbers::UniformRandomSequenceGenerator sequenceGen);
            double operator()(const Path& path) const;
          private:
            
            Binary::Type binaryType_;
            double barrier_;               
            double cashPayoff_;
            Option::Type type_;
            double underlying_;
            Handle<DiffusionProcess> diffProcess_;
            RandomNumbers::UniformRandomSequenceGenerator sequenceGen_;

            
        };
    
    }

}

#endif
