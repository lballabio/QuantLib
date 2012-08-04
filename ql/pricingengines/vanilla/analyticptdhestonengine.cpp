/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2010 Klaus Spanderen

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

/*! \file analyticptdhestonengine.cpp
    \brief analytic piecewise time dependent Heston-model engine
*/

#include <ql/math/functional.hpp>
#include <ql/instruments/payoffs.hpp>
#include <ql/pricingengines/vanilla/analyticptdhestonengine.hpp>


namespace QuantLib {
    // helper class for integration
    class AnalyticPTDHestonEngine::Fj_Helper
        : public std::unary_function<Real, Real> {
            
      public:
        Fj_Helper(
            const Handle<PiecewiseTimeDependentHestonModel>& model,
            Time term, Real strike, Size j);
    
        Real operator()(Real phi) const;
        
      private:
        const Size j_;    
        const Time term_;
        const Real v0_, x_, sx_;
        
        std::vector<Rate> r_, q_;
        const boost::shared_ptr<YieldTermStructure> qTS_;
        const Handle<PiecewiseTimeDependentHestonModel> model_;
        
        const TimeGrid timeGrid_;
    };
        
    AnalyticPTDHestonEngine::Fj_Helper::Fj_Helper(
        const Handle<PiecewiseTimeDependentHestonModel>& model,
        Time term, Real strike, Size j)
    : j_(j),
      term_(term),

      v0_(model->v0()),
      x_ (std::log(model->s0())),
      sx_(std::log(strike)),
      r_(model->timeGrid().size()-1),
      q_(model->timeGrid().size()-1),
      model_(model),
      timeGrid_(model->timeGrid()){
        
        for (Size i=0; i <timeGrid_.size()-1; ++i) {
            const Time begin = std::min(term_, timeGrid_[i]);
            const Time end   = std::min(term_, timeGrid_[i+1]);
            r_[i] = model->riskFreeRate()
                    ->forwardRate(begin, end, Continuous, NoFrequency).rate();
            q_[i] = model->dividendYield()
                    ->forwardRate(begin, end, Continuous, NoFrequency).rate();
        }            
            
        QL_REQUIRE(term_ < model_->timeGrid().back(), "maturity is too large");
    }
        
    Real AnalyticPTDHestonEngine::Fj_Helper::operator()(Real phi) const {

        // avoid numeric overflow for phi->0. 
        // todo: use l'Hospital's rule use to get lim_{phi->0}
        phi = std::max(Real(std::numeric_limits<float>::epsilon()), phi);
        
        std::complex<Real> D = 0.0;
        std::complex<Real> C = 0.0;

        for (Size i=timeGrid_.size()-1; i > 0; --i) {
            const Time begin = timeGrid_[i-1];
            if (begin < term_) {
                const Time end = std::min(term_, timeGrid_[i]);
                const Time tau = end-begin;
                const Time t   = 0.5*(end+begin);
                
                const Real rho = model_->rho(t);
                const Real sigma = model_->sigma(t);
                const Real kappa = model_->kappa(t);
                const Real theta = model_->theta(t);
                
                const Real sigma2 = sigma*sigma;
                const Real t0 = kappa - ((j_== 1)? rho*sigma : 0);
                const Real rpsig = rho*sigma*phi;

                const std::complex<Real> t1 = t0+std::complex<Real>(0, -rpsig);
                const std::complex<Real> d  = std::sqrt(t1*t1 - sigma2*phi
                                 *std::complex<Real>(-phi, (j_== 1)? 1 : -1));
                const std::complex<Real> g = (t1-d)/(t1+d);
                const std::complex<Real> gt 
                                       = (t1-d - D*sigma2)/(t1+d - D*sigma2);
                
                D = (t1+d)/sigma2*(g-gt*std::exp(-d*tau))
                    /(1.0-gt*std::exp(-d*tau));
                
                const std::complex<Real> lng 
                    = std::log((1.0 - gt*std::exp(-d*tau))/(1.0 - gt));
                
                C =(kappa*theta)/sigma2*((t1-d)*tau-2.0*lng)
                    + std::complex<Real>(0.0, phi*(r_[i-1]-q_[i-1])*tau) + C;
            }
        }
        return std::exp(v0_*D+C+std::complex<Real>(0.0, phi*(x_ - sx_))).imag()
                /phi; 
    }

    AnalyticPTDHestonEngine::AnalyticPTDHestonEngine(
        const boost::shared_ptr<PiecewiseTimeDependentHestonModel>& model,
        Size integrationOrder)
    : GenericModelEngine<PiecewiseTimeDependentHestonModel,
                         VanillaOption::arguments,
                         VanillaOption::results>(model),
      integration_(new AnalyticHestonEngine::Integration(
        AnalyticHestonEngine::Integration::gaussLaguerre(integrationOrder))) {
    }
                         
    AnalyticPTDHestonEngine::AnalyticPTDHestonEngine(
        const boost::shared_ptr<PiecewiseTimeDependentHestonModel>& model,
        Real relTolerance, Size maxEvaluations)
    : GenericModelEngine<PiecewiseTimeDependentHestonModel,
                         VanillaOption::arguments,
                         VanillaOption::results>(model),
      integration_(new AnalyticHestonEngine::Integration(
        AnalyticHestonEngine::Integration::gaussLobatto(
                               relTolerance, Null<Real>(), maxEvaluations))) {
    }

    void AnalyticPTDHestonEngine::calculate() const {
        // this is an european option pricer
        QL_REQUIRE(arguments_.exercise->type() == Exercise::European,
                "not an European option");

        // plain vanilla
        boost::shared_ptr<PlainVanillaPayoff> payoff =
            boost::dynamic_pointer_cast<PlainVanillaPayoff>(arguments_.payoff);
        QL_REQUIRE(payoff, "non-striked payoff given");
        
        const Real v0 = model_->v0();
        const Real spotPrice = model_->s0();
        QL_REQUIRE(spotPrice > 0.0, "negative or null underlying given");
        
        const Real strike = payoff->strike();
        const Real term 
            = model_->riskFreeRate()->dayCounter().yearFraction(
                                     model_->riskFreeRate()->referenceDate(), 
                                     arguments_.exercise->lastDate());
        const Real riskFreeDiscount = model_->riskFreeRate()->discount(
                                            arguments_.exercise->lastDate());
        const Real dividendDiscount = model_->dividendYield()->discount(
                                            arguments_.exercise->lastDate());

        //average values
        const TimeGrid& timeGrid = model_->timeGrid();
        const Size n = timeGrid.size()-1;
        Real kappaAvg = 0.0, thetaAvg = 0.0,  sigmaAvg=0.0, rhoAvg = 0.0;

        for (Size i=1; i <= n; ++i) {
            const Time t = 0.5*(timeGrid[i-1] + timeGrid[i]);
            kappaAvg += model_->kappa(t);
            thetaAvg += model_->theta(t);
            sigmaAvg += model_->sigma(t);
            rhoAvg   += model_->rho(t);
        }
        kappaAvg/=n; thetaAvg/=n; sigmaAvg/=n; rhoAvg/=n;
        
        const Real c_inf = std::min(10.0, std::max(0.0001,
                std::sqrt(1.0-square<Real>()(rhoAvg))/sigmaAvg))
                *(v0 + kappaAvg*thetaAvg*term);

        const Real p1 = integration_->calculate(c_inf,
                                Fj_Helper(model_, term, strike, 1))/M_PI;

        const Real p2 = integration_->calculate(c_inf,
                                Fj_Helper(model_, term, strike, 2))/M_PI;

        switch (payoff->optionType())
        {
          case Option::Call:
            results_.value = spotPrice*dividendDiscount*(p1+0.5)
                            - strike*riskFreeDiscount*(p2+0.5);
            break;
          case Option::Put:
            results_.value = spotPrice*dividendDiscount*(p1-0.5)
                            - strike*riskFreeDiscount*(p2-0.5);
            break;
          default:
            QL_FAIL("unknown option type");
        }
    }
}
