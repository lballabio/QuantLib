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

#ifndef quantlib_market_model_vol_process_hpp
#define quantlib_market_model_vol_process_hpp
#include <ql/types.hpp>
#include <vector>

namespace QuantLib 
{

  
    /*!
   Displaced diffusion LMM with uncorrelated vol process. Called "Shifted BGM" with Heston vol by Brace in "Engineering BGM."
   Vol process is an external input.
    
    */
    class MarketModelVolProcess 
    {
      public:
          MarketModelVolProcess() = default;
          virtual ~MarketModelVolProcess() = default;

          virtual Size variatesPerStep()=0;
          virtual Size numberSteps()=0;

          virtual void nextPath() =0;
          virtual Real nextstep(const std::vector<Real>& variates)=0;
          virtual Real stepSd() const =0;

          virtual const std::vector<Real>& stateVariables() const=0;
          virtual Size numberStateVariables() const=0;
     
      private:
 
    };

}

#endif


#ifndef id_3f5ea6cce458812eac4ced8817c07ce1
#define id_3f5ea6cce458812eac4ced8817c07ce1
inline bool test_3f5ea6cce458812eac4ced8817c07ce1(const int* i) {
    return i != nullptr;
}
#endif
