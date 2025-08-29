/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Mark Joshi

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#ifndef quantlib_square_root_andersen_hpp
#define quantlib_square_root_andersen_hpp

#include <ql/models/marketmodels/evolvers/marketmodelvolprocess.hpp>

namespace QuantLib 
{

  
    /*!
   Displaced diffusion LMM with uncorrelated vol process. Called "Shifted BGM" with Heston vol by Brace in "Engineering BGM."
   Vol process is an external input.
    
    */
    class SquareRootAndersen  : public MarketModelVolProcess
    {
      public:
          SquareRootAndersen(Real meanLevel,
                             Real reversionSpeed,
                             Real volVar,
                             Real v0,
                             const std::vector<Real>& evolutionTimes,
                             Size numberSubSteps_,
                             Real w1,
                             Real w2,
                             Real cutPoint = 1.5);

          Size variatesPerStep() override;
          Size numberSteps() override;

          void nextPath() override;
          Real nextstep(const std::vector<Real>& variates) override;
          Real stepSd() const override;

          const std::vector<Real>& stateVariables() const override;
          Size numberStateVariables() const override;

        private:

          void DoOneSubStep(Real& v, Real variate, Size j);

          Real theta_; // mean level

          Real k_; // reversion speed
          Real epsilon_; // volvar

          Real v0_; // initial value of instantaneous variance
          Size numberSubSteps_; // sub steps per evolution time 


          std::vector<Real> dt_; // time step lengths
          std::vector<Real> eMinuskDt_; // exp( - k * dt) 

          Real w1_; // weights to use for computing variance across step
          Real w2_; 
          Real PsiC_; // cut-off between two types of evolution

          //! evolving values
          Real v_;
          Size currentStep_;
          Size subStep_;
          std::vector<Real> vPath_;

          mutable std::vector<Real> state_;
 
    };

}

#endif
