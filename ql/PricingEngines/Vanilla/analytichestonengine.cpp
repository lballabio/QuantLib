/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2004, 2005 Klaus Spanderen

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file hestonmodel.hpp
  \brief analytic pricing engine for a heston option
  based on fourier transformation
*/

#include <ql/PricingEngines/Vanilla/analytichestonengine.hpp>
#include <ql/Instruments/payoffs.hpp>

#if !defined(QL_PATCH_MSVC6) && !defined(QL_PATCH_BORLAND)

namespace QuantLib {

    // helper class for integration
    class AnalyticHestonEngine::Fj_Helper {
      public:
        Fj_Helper(const VanillaOption::arguments& arguments,
                  const boost::shared_ptr<HestonModel>& model,
                  const AnalyticHestonEngine* const engine,
                  Time term, Real ratio, Size j);

        Real operator()(Real phi)      const;

      private:
        const Size j_;
        const VanillaOption::arguments& arg_;
        const Real kappa_, theta_, sigma_, v0_;

        // helper variables
        const Time term_;
        const Real x_, sx_, dd_;
        const Real sigma2_, rsigma_;
        const Real t0_;

        // log branch counter
        mutable int  b_;     // log branch counter
        mutable Real g_km1_; // imag part of last log value

        const AnalyticHestonEngine* const engine_;
    };


    AnalyticHestonEngine::Fj_Helper::Fj_Helper(
        const VanillaOption::arguments& arguments,
        const boost::shared_ptr<HestonModel>& model,
        const AnalyticHestonEngine* const engine,
        Time term, Real ratio, Size j)
    : j_ (j), arg_(arguments),
      kappa_(model->kappa()), theta_(model->theta()),
      sigma_(model->sigma()), v0_(model->v0()),
      term_(term),
      x_(std::log(boost::dynamic_pointer_cast<HestonProcess>
                  (arg_.stochasticProcess)->s0())),
      sx_(std::log(boost::dynamic_pointer_cast<StrikedTypePayoff>
                   (arg_.payoff)->strike())),
      dd_(x_-std::log(ratio)),
      sigma2_(sigma_*sigma_),
      rsigma_(model->rho()*sigma_),
      t0_(kappa_ - ((j_== 1)? model->rho()*sigma_ : 0)),
      b_(0), g_km1_(0),
      engine_(engine) {
    }

    Real AnalyticHestonEngine::Fj_Helper::operator()(Real phi) const {
        const Real rpsig(rsigma_*phi);

        const std::complex<Real> t1 = t0_+std::complex<Real>(0, -rpsig);
        const std::complex<Real> d =
            std::sqrt(t1*t1 - sigma2_*phi
                      *std::complex<Real>(-phi, (j_== 1)? 1 : -1));
        const std::complex<Real> p  = (t1+d)/(t1 - d);
        const std::complex<Real> ex = std::exp(-d*term_);

        // next term: g = std::log((1.0 - p*std::exp(d*term_))/(1.0 - p))
        std::complex<Real> g;

        // the exp of the following expression is needed.
        const std::complex<Real> e = std::log(p)+d*term_;

        // does it fit to the machine precision?
        if (std::exp(-e.real()) > QL_EPSILON) {
            g = std::log((1.0 - p*std::exp(d*term_))/(1.0 - p));
        } else {
            // use a "big phi" approximation
            g = d*term_ + std::log(p/(p - 1.0));

            if (g.imag() > M_PI || g.imag() <= -M_PI) {
                // get back to principal branch of the complex logarithm
                Real im = std::fmod(g.imag(), 2*M_PI);
                if (im > M_PI)
                    im -= 2*M_PI;
                else if (im <= -M_PI)
                    im += 2*M_PI;

                g = std::complex<Real>(g.real(), im);
            }
        }

        // be careful here as we have to use a log branch correction
        // to deal with the discontinuities of the complex logarithm.
        // the principal branch is not always the correct one.
        // (s. A. Sepp, chapter 4)
        // remark: there is still the change that we miss a branch
        // if the order of the integration is not high enough.
        const Real tmp = g.imag() - g_km1_;
        if (tmp <= -M_PI)
            ++b_;
        else if (tmp > M_PI)
            --b_;

        g_km1_ = g.imag();
        g += std::complex<Real>(0, 2*b_*M_PI);

        return std::exp(v0_*(t1+d)*(ex-1.0)/(sigma2_*(ex-p))
                        + (kappa_*theta_)/sigma2_*((t1+d)*term_-2.0*g)
                        + std::complex<Real>(0,phi*(dd_-sx_))
                        + engine_->jumpDiffusionTerm(phi, term_, j_)
                        ).imag()/phi;
    }



    AnalyticHestonEngine::AnalyticHestonEngine(
        const boost::shared_ptr<HestonModel> & model,
        Size integrationOrder)
    : GenericModelEngine<HestonModel,
                         VanillaOption::arguments,
                         VanillaOption::results>(model),
      gaussLaguerre(integrationOrder) {}


    std::complex<Real>
    AnalyticHestonEngine::jumpDiffusionTerm(Real phi, Time t, Size j) const {
        return std::complex<Real>(0,0);
    }

    void AnalyticHestonEngine::calculate() const {
        // this is a european option pricer
        QL_REQUIRE(arguments_.exercise->type() == Exercise::European,
                   "not an European option");

        // plain vanilla
        boost::shared_ptr<StrikedTypePayoff> payoff =
            boost::dynamic_pointer_cast<StrikedTypePayoff>(arguments_.payoff);
        QL_REQUIRE(payoff, "non-striked payoff given");

        // Heston process
        boost::shared_ptr<HestonProcess> process =
            boost::dynamic_pointer_cast<HestonProcess>(
                                                arguments_.stochasticProcess);

        const Rate riskFreeDiscount = process->riskFreeRate()->discount(
                                            arguments_.exercise->lastDate());
        const Rate dividendDiscount = process->dividendYield()->discount(
                                            arguments_.exercise->lastDate());
        const Real ratio = riskFreeDiscount/dividendDiscount;

        const Real spotPrice = process->s0();
        const Real strikePrice = payoff->strike();
        const Real term = process->time(arguments_.exercise->lastDate());

        const Real p1 = gaussLaguerre(
            Fj_Helper(arguments_, model_, this, term, ratio, 1))/M_PI;
        const Real p2 = gaussLaguerre(
            Fj_Helper(arguments_, model_, this, term, ratio, 2))/M_PI;

        switch (payoff->optionType()) {
          case Option::Call:
            results_.value = spotPrice*dividendDiscount*(p1+0.5)
                           - strikePrice*riskFreeDiscount*(p2+0.5);
            break;
          case Option::Put:
            results_.value = spotPrice*dividendDiscount*(p1-0.5)
                           - strikePrice*riskFreeDiscount*(p2-0.5);
            break;
          default:
            QL_FAIL("unknown option type");
        }
    }
}

#endif

