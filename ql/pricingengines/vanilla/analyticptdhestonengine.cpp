/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2010, 2017 Klaus Spanderen

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
#include <ql/pricingengines/blackcalculator.hpp>

namespace QuantLib {

    // helper class for integration
    class AnalyticPTDHestonEngine::Fj_Helper {
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
        const ext::shared_ptr<YieldTermStructure> qTS_;
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

    class AnalyticPTDHestonEngine::AP_Helper {
      public:
        AP_Helper(Time term, Real s0, Real strike, Real ratio,
                  Volatility sigmaBS,
                  const AnalyticPTDHestonEngine* const enginePtr)
        : term_(term),
          sigmaBS_(sigmaBS),
          x_(std::log(s0)),
          sx_(std::log(strike)),
          dd_(x_-std::log(ratio)),
          enginePtr_(enginePtr) {
            QL_REQUIRE(enginePtr != nullptr, "pricing engine required");
        }

        Real operator()(Real u) const {
            const std::complex<Real> z(u, -0.5);

            const std::complex<Real> phiBS
                = std::exp(-0.5*sigmaBS_*sigmaBS_*term_
                           *(z*z + std::complex<Real>(-z.imag(), z.real())));

            return (std::exp(std::complex<Real>(0.0, u*(dd_-sx_)))
                * (phiBS - enginePtr_->chF(z, term_)) / (u*u + 0.25)).real();
        }

      private:
        const Time term_;
        const Volatility sigmaBS_;
        const Real x_, sx_, dd_;
        const AnalyticPTDHestonEngine* const enginePtr_;
    };


    std::complex<Real> AnalyticPTDHestonEngine::lnChF(
        const std::complex<Real>& z, Time T) const {

        const Real v0 = model_->v0();

        std::complex<Real> D = 0.0;
        std::complex<Real> C = 0.0;

        const TimeGrid& timeGrid = model_->timeGrid();
        const Time lastModelTime = timeGrid.back();

        QL_REQUIRE(T <= lastModelTime,
                   "maturity (" << T << ") is too large, "
                   "time grid is bounded by " << lastModelTime);

        const Size lastI = std::distance(timeGrid.begin(),
            std::lower_bound(timeGrid.begin(), timeGrid.end(), T));

        for (Integer i=lastI-1; i >= 0; --i) {
            const Time begin = timeGrid[i];
            const Time end = std::min(T, timeGrid[i+1]);
            const Time tau = end - begin;

            const Time t     = 0.5*(end+begin);
            const Real kappa = model_->kappa(t);
            const Real sigma = model_->sigma(t);
            const Real theta = model_->theta(t);
            const Real rho   = model_->rho(t);

            const Real sigma2 = sigma*sigma;

            const std::complex<Real> k
                = kappa + rho*sigma*std::complex<Real>(z.imag(), -z.real());

            const std::complex<Real> d = std::sqrt(
                k*k + (z*z + std::complex<Real>(-z.imag(), z.real()))*sigma2);

            const std::complex<Real> g = (k-d)/(k+d);

            const std::complex<Real> gt = (k-d-D*sigma2)/(k+d-D*sigma2);

            C += kappa*theta/sigma2*( (k-d)*tau
                   - 2.0*std::log((1.0-gt*std::exp(-d*tau))/(1.0-gt)));

            D = (k+d)/sigma2 * (g - gt*std::exp(-d*tau))
                    /(1.0 - gt*std::exp(-d*tau));
        }

        return D*v0 + C;
    }

    std::complex<Real> AnalyticPTDHestonEngine::chF(
        const std::complex<Real>& z, Time T) const {
        return std::exp(lnChF(z, T));
    }

    AnalyticPTDHestonEngine::AnalyticPTDHestonEngine(
        const ext::shared_ptr<PiecewiseTimeDependentHestonModel>& model,
        Size integrationOrder)
    : GenericModelEngine<PiecewiseTimeDependentHestonModel,
                         VanillaOption::arguments,
                         VanillaOption::results>(model),
      evaluations_(0),
      cpxLog_(Gatheral),
      integration_(new Integration(
          Integration::gaussLaguerre(integrationOrder))),
      andersenPiterbargEpsilon_(Null<Real>()) {
    }
                         
    AnalyticPTDHestonEngine::AnalyticPTDHestonEngine(
        const ext::shared_ptr<PiecewiseTimeDependentHestonModel>& model,
        Real relTolerance, Size maxEvaluations)
    : GenericModelEngine<PiecewiseTimeDependentHestonModel,
                         VanillaOption::arguments,
                         VanillaOption::results>(model),
      evaluations_(0),
      cpxLog_(Gatheral),
      integration_(new Integration(Integration::gaussLobatto(
            relTolerance, Null<Real>(), maxEvaluations))),
      andersenPiterbargEpsilon_(Null<Real>()) {
    }

    AnalyticPTDHestonEngine::AnalyticPTDHestonEngine(
        const ext::shared_ptr<PiecewiseTimeDependentHestonModel>& model,
        ComplexLogFormula cpxLog,
        const Integration& itg,
        Real andersenPiterbargEpsilon)
    : GenericModelEngine<PiecewiseTimeDependentHestonModel,
                         VanillaOption::arguments,
                         VanillaOption::results>(model),
      evaluations_(0),
      cpxLog_(cpxLog),
      integration_(new Integration(itg)),
      andersenPiterbargEpsilon_(andersenPiterbargEpsilon) {
    }


    void AnalyticPTDHestonEngine::calculate() const {
        // this is an european option pricer
        QL_REQUIRE(arguments_.exercise->type() == Exercise::European,
                "not an European option");

        // plain vanilla
        ext::shared_ptr<PlainVanillaPayoff> payoff =
            ext::dynamic_pointer_cast<PlainVanillaPayoff>(arguments_.payoff);
        QL_REQUIRE(payoff, "non-striked payoff given");
        
        const Real v0 = model_->v0();
        const Real spotPrice = model_->s0();
        QL_REQUIRE(spotPrice > 0.0, "negative or null underlying given");
        
        const Real strike = payoff->strike();
        const Real term 
            = model_->riskFreeRate()->dayCounter().yearFraction(
                                     model_->riskFreeRate()->referenceDate(), 
                                     arguments_.exercise->lastDate());

        QL_REQUIRE(term < model_->timeGrid().back() ||
                       close_enough(term, model_->timeGrid().back()),
                   "maturity (" << term << ") is too large, time grid is bounded by "
                                << model_->timeGrid().back());

        const Real riskFreeDiscount = model_->riskFreeRate()->discount(
                                            arguments_.exercise->lastDate());
        const Real dividendDiscount = model_->dividendYield()->discount(
                                            arguments_.exercise->lastDate());

        //average values
        const TimeGrid& timeGrid = model_->timeGrid();
        QL_REQUIRE(timeGrid.size() > 1, "at least two model points needed");

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

        evaluations_ = 0;

        switch(cpxLog_) {
          case Gatheral: {
            const Real c_inf = std::min(0.2, std::max(0.0001,
                std::sqrt(1.0-squared(rhoAvg))/sigmaAvg))
                *(v0 + kappaAvg*thetaAvg*term);

            const Real p1 = integration_->calculate(c_inf,
                                    Fj_Helper(model_, term, strike, 1))/M_PI;
            evaluations_ += integration_->numberOfEvaluations();

            const Real p2 = integration_->calculate(c_inf,
                                    Fj_Helper(model_, term, strike, 2))/M_PI;
            evaluations_ += integration_->numberOfEvaluations();

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
          break;
          case AndersenPiterbarg: {
              QL_REQUIRE(term <= timeGrid.back(),
                         "maturity (" << term << ") is too large, "
                         "time grid is bounded by " << timeGrid.back());

              const Time t05 = 0.5*timeGrid.at(1);

              const std::complex<Real> D_u_inf =
                  -std::complex<Real>(
                      std::sqrt(1-squared(model_->rho(t05))),
                      model_->rho(t05)) / model_->sigma(t05);

              const Size lastI = std::distance(timeGrid.begin(),
                  std::lower_bound(timeGrid.begin(), timeGrid.end(), term));

              std::complex<Real> C_u_inf(0.0, 0.0);
              for (Size i=0; i < lastI; ++i) {
                  const Time begin = timeGrid[i];
                  const Time end   = std::min(term, timeGrid[i+1]);
                  const Time tau   = end - begin;
                  const Time t     = 0.5*(end+begin);

                  const Real kappa = model_->kappa(t);
                  const Real theta = model_->theta(t);
                  const Real sigma = model_->sigma(t);
                  const Real rho = model_->rho(t);

                  C_u_inf += -kappa*theta*tau / sigma
                      *std::complex<Real>(std::sqrt(1-rho*rho), rho);
              }

              const Real ratio = riskFreeDiscount/dividendDiscount;

              const Real fwdPrice = spotPrice / ratio;

              const Real epsilon = andersenPiterbargEpsilon_
                  *M_PI/(std::sqrt(strike*fwdPrice)*riskFreeDiscount);

              const Real c_inf = -(C_u_inf + D_u_inf*v0).real();

              const ext::function<Real()> uM = [=](){
                  return Integration::andersenPiterbargIntegrationLimit(c_inf, epsilon, v0, term);
              };

              const Real vAvg
                  = (1-std::exp(-kappaAvg*term))*(v0-thetaAvg)
                    /(kappaAvg*term) + thetaAvg;

              const Real bsPrice
                  = BlackCalculator(Option::Call, strike,
                                    fwdPrice, std::sqrt(vAvg*term),
                                    riskFreeDiscount).value();

              const Real h_cv = integration_->calculate(c_inf,
                      AP_Helper(term, spotPrice, strike,
                                ratio, std::sqrt(vAvg), this),uM)
                  * std::sqrt(strike * fwdPrice)*riskFreeDiscount/M_PI;
              evaluations_ += integration_->numberOfEvaluations();

              switch (payoff->optionType())
              {
                case Option::Call:
                  results_.value = bsPrice + h_cv;
                  break;
                case Option::Put:
                  results_.value = bsPrice + h_cv
                      - riskFreeDiscount*(fwdPrice - strike);
                  break;
                default:
                  QL_FAIL("unknown option type");
              }
            }
            break;

            default:
              QL_FAIL("unknown complex log formula");
          }
    }
 
    Size AnalyticPTDHestonEngine::numberOfEvaluations() const {
        return evaluations_;
    }
}
