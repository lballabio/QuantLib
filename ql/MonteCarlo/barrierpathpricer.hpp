
/*
 Copyright (C) 2003 Neil Firth
 Copyright (C) 2003 Ferdinando Ametrano
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

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

/*! \file barrierpathpricer.hpp
    \brief path pricer for Barrier options
*/

#ifndef quantlib_montecarlo_barrier_path_pricer_h
#define quantlib_montecarlo_barrier_path_pricer_h

#include <ql/Instruments/barrieroption.hpp>
#include <ql/MonteCarlo/pathpricer.hpp>
#include <ql/MonteCarlo/path.hpp>
#include <ql/RandomNumbers/rngtypedefs.hpp>
#include <ql/diffusionprocess.hpp>
#include <ql/Instruments/payoffs.hpp>

namespace QuantLib {

    //! %path pricer for Barrier options
    /*! Uses the Brownian Bridge correction for the barrier found in
        <i>
        Going to Extremes: Correcting Simulation Bias in Exotic
        Option Valuation - D.R. Beaglehole, P.H. Dybvig and G. Zhou
        Financial Analysts Journal; Jan/Feb 1997; 53, 1. pg. 62-68
        </i>
        and
        <i>
        Simulating path-dependent options: A new approach - 
        M. El Babsiri and G. Noel
        Journal of Derivatives; Winter 1998; 6, 2; pg. 65-83
        </i>
    */
    class BarrierPathPricer : public PathPricer<Path> {
      public:
        BarrierPathPricer(Barrier::Type barrierType, 
                          double barrier, 
                          double rebate, 
                          Option::Type type,
                          double underlying,
                          double strike,
                          const RelinkableHandle<TermStructure>& riskFreeTS,
                          const Handle<DiffusionProcess>& diffProcess,
                          UniformRandomSequenceGenerator sequenceGen);
        double operator()(const Path& path) const;
      private:
        double underlying_;
        Barrier::Type barrierType_;
        double barrier_;
        double rebate_;
        Handle<DiffusionProcess> diffProcess_;
        UniformRandomSequenceGenerator sequenceGen_;
        // it would be easy to generalize to more exotic payoffs
        PlainVanillaPayoff payoff_;
    };

}


#endif
