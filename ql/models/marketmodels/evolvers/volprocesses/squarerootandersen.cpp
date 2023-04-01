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


#include <ql/models/marketmodels/evolvers/volprocesses/squarerootandersen.hpp>
#include <ql/errors.hpp>
#include <ql/math/distributions/normaldistribution.hpp>
namespace
QuantLib
{
    SquareRootAndersen::SquareRootAndersen(Real meanLevel,
        Real reversionSpeed,
        Real volVar,
        Real v0,
        const std::vector<Real>& evolutionTimes,
        Size numberSubSteps,
        Real w1,
        Real w2,
        Real cutPoint )
        :
    theta_(meanLevel),
        k_(reversionSpeed),
        epsilon_(volVar),
        v0_(v0),
        numberSubSteps_(numberSubSteps),
        dt_(evolutionTimes.size()*numberSubSteps),
        eMinuskDt_(evolutionTimes.size()*numberSubSteps),
        w1_(w1),
        w2_(w2),
        PsiC_(cutPoint),
        vPath_(evolutionTimes.size()*numberSubSteps+1),
        state_(1)
    {
        Size j=0;
        for (; j < numberSubSteps_; ++j)
            dt_[j] = evolutionTimes[0]/numberSubSteps_;

        for (Size i=1; i < evolutionTimes.size(); ++i)
        {
            Real dt = (evolutionTimes[i] - evolutionTimes[i-1])/numberSubSteps_;

            Real ekdt = std::exp(-k_*dt);
            QL_REQUIRE(dt >0.0, "Steps must be of positive size.");

            for (Size k=0; k < numberSubSteps_; ++k)
            {
                dt_[j] = dt;
                eMinuskDt_[j] = ekdt;

                ++j;
            }
        }
        vPath_[0] = v0_;
    }


    Size SquareRootAndersen::variatesPerStep()
    {
        return numberSubSteps_;
    }

    Size SquareRootAndersen::numberSteps()
    {
        return dt_.size()*numberSubSteps_;
    }

    void SquareRootAndersen::nextPath()
    {
          v_=v0_;
          currentStep_=0;
          subStep_=0;

    }

    void SquareRootAndersen::DoOneSubStep(Real& vt, Real z, Size j)
    {

        Real eminuskT = eMinuskDt_[j];
        Real m = theta_+(vt-theta_)*eminuskT;
        Real s2= vt*epsilon_*epsilon_*eminuskT*(1-eminuskT)/k_
                + theta_*epsilon_*epsilon_*(1- eminuskT)*(1- eminuskT)/(2*k_);
        Real s = std::sqrt(s2);
        Real psi = s*s/(m*m);
        if (psi<= PsiC_)
        {
            Real psiinv = 1.0/psi;
            Real b2 = 2.0*psiinv -1+std::sqrt(2*psiinv*(2*psiinv-1.0));
            Real b = std::sqrt(b2);
            Real a= m/(1+b2);
            vt= a*(b+z)*(b+z);
        }
        else
        {
            Real p = (psi-1.0)/(psi+1.0);
            Real beta = (1.0-p)/m;
            Real u = CumulativeNormalDistribution()(z);

            if (u < p)
            {
                vt=0;
                return;
            }

             vt = std::log((1.0-p)/(1.0-u))/beta;
        }

    }


    Real SquareRootAndersen::nextstep(const std::vector<Real>& variates)
    {
        for (Size j=0; j < numberSubSteps_; ++j)
        {
            DoOneSubStep(v_, variates[j], subStep_);
            ++subStep_;
            vPath_[subStep_] = v_;
        }

        ++currentStep_;

        return 1.0; // no importance sampling here
    }

    Real SquareRootAndersen::stepSd() const
    {
        QL_REQUIRE(currentStep_>0, "nextStep must be called before stepSd");
        Real stepVariance =0.0;
        Size lastStepStart = (currentStep_-1)*numberSubSteps_;
        for (Size k=0; k < numberSubSteps_; ++k)
            stepVariance += w1_*vPath_[k+lastStepStart]+w2_*vPath_[k+lastStepStart+1];

        stepVariance /= numberSubSteps_;

        return std::sqrt(stepVariance);
    }

    const std::vector<Real>& SquareRootAndersen::stateVariables() const
    {
        state_[0] = v_;
        return state_;
    }

    Size SquareRootAndersen::numberStateVariables() const
    {
        return 1;
    }

}
