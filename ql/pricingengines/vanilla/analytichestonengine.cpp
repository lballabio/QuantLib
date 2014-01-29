/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2004, 2005, 2008 Klaus Spanderen
 Copyright (C) 2007 StatPro Italia srl

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

/*! \file hestonmodel.hpp
  \brief analytic pricing engine for a heston option
  based on fourier transformation
*/

#include <ql/math/functional.hpp>
#include <ql/math/integrals/simpsonintegral.hpp>
#include <ql/math/integrals/kronrodintegral.hpp>
#include <ql/math/integrals/trapezoidintegral.hpp>
#include <ql/math/integrals/gausslobattointegral.hpp>

#include <ql/instruments/payoffs.hpp>
#include <ql/pricingengines/vanilla/analytichestonengine.hpp>

#if defined(QL_PATCH_MSVC)
#pragma warning(disable: 4180)
#endif

#include <boost/lambda/if.hpp>
#include <boost/lambda/bind.hpp>
#include <boost/lambda/lambda.hpp>

using namespace boost::lambda;

namespace QuantLib {

    // helper class for integration
    class AnalyticHestonEngine::Fj_Helper
        : public std::unary_function<Real, Real>
    {
    public:
        Fj_Helper(const VanillaOption::arguments& arguments,
            const boost::shared_ptr<HestonModel>& model,
            const AnalyticHestonEngine* const engine,
            ComplexLogFormula cpxLog,
            Time term, Real ratio, Size j);

        Fj_Helper(Real kappa, Real theta, Real sigma,
            Real v0, Real s0, Real rho,
            const AnalyticHestonEngine* const engine,
            ComplexLogFormula cpxLog,
            Time term,
            Real strike,
            Real ratio,
            Size j);

         Fj_Helper(Real kappa, Real theta, Real sigma,
            Real v0, Real s0, Real rho,
            ComplexLogFormula cpxLog,
            Time term,
            Real strike,
            Real ratio,
            Size j);

        Real operator()(Real phi)      const;

    private:
        const Size j_;
        //     const VanillaOption::arguments& arg_;
        const Real kappa_, theta_, sigma_, v0_;
        const ComplexLogFormula cpxLog_;

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
        ComplexLogFormula cpxLog,
        Time term, Real ratio, Size j)
        : j_ (j), //arg_(arguments),
        kappa_(model->kappa()), theta_(model->theta()),
        sigma_(model->sigma()), v0_(model->v0()),
        cpxLog_(cpxLog), term_(term),
        x_(std::log(model->process()->s0()->value())),
        sx_(std::log(boost::dynamic_pointer_cast<StrikedTypePayoff>
        (arguments.payoff)->strike())),
        dd_(x_-std::log(ratio)),
        sigma2_(sigma_*sigma_),
        rsigma_(model->rho()*sigma_),
        t0_(kappa_ - ((j_== 1)? model->rho()*sigma_ : 0)),
        b_(0), g_km1_(0),
        engine_(engine)
    {
    }

    AnalyticHestonEngine::Fj_Helper::Fj_Helper(Real kappa, Real theta,
        Real sigma, Real v0, Real s0, Real rho,
        const AnalyticHestonEngine* const engine,
        ComplexLogFormula cpxLog,
        Time term,
        Real strike,
        Real ratio,
        Size j)
        :
        j_(j),
        kappa_(kappa),
        theta_(theta),
        sigma_(sigma),
        v0_(v0),
        cpxLog_(cpxLog),
        term_(term),
        x_(std::log(s0)),
        sx_(std::log(strike)),
        dd_(x_-std::log(ratio)),
        sigma2_(sigma_*sigma_),
        rsigma_(rho*sigma_),
        t0_(kappa - ((j== 1)? rho*sigma : 0)),
        b_(0),
        g_km1_(0),
        engine_(engine)
    {
    }

    AnalyticHestonEngine::Fj_Helper::Fj_Helper(Real kappa, Real theta,
        Real sigma, Real v0, Real s0, Real rho,
        ComplexLogFormula cpxLog,
        Time term,
        Real strike,
        Real ratio,
        Size j)
        :
        j_(j),
        kappa_(kappa),
        theta_(theta),
        sigma_(sigma),
        v0_(v0),
        cpxLog_(cpxLog),
        term_(term),
        x_(std::log(s0)),
        sx_(std::log(strike)),
        dd_(x_-std::log(ratio)),
        sigma2_(sigma_*sigma_),
        rsigma_(rho*sigma_),
        t0_(kappa - ((j== 1)? rho*sigma : 0)),
        b_(0),
        g_km1_(0),
        engine_(0)
    {
    }


    Real AnalyticHestonEngine::Fj_Helper::operator()(Real phi) const
    {
        const Real rpsig(rsigma_*phi);

        const std::complex<Real> t1 = t0_+std::complex<Real>(0, -rpsig);
        const std::complex<Real> d =
            std::sqrt(t1*t1 - sigma2_*phi
                      *std::complex<Real>(-phi, (j_== 1)? 1 : -1));
        const std::complex<Real> ex = std::exp(-d*term_);
        const std::complex<Real> addOnTerm
            = engine_ > 0 ? engine_->addOnTerm(phi, term_, j_) : 0.0;

        if (cpxLog_ == Gatheral) {
            if (phi != 0.0) {
                if (sigma_ > 1e-5) {
                    const std::complex<Real> p = (t1-d)/(t1+d);
                    const std::complex<Real> g
                                            = std::log((1.0 - p*ex)/(1.0 - p));

                    return
                        std::exp(v0_*(t1-d)*(1.0-ex)/(sigma2_*(1.0-ex*p))
                                 + (kappa_*theta_)/sigma2_*((t1-d)*term_-2.0*g)
                                 + std::complex<Real>(0.0, phi*(dd_-sx_))
                                 + addOnTerm
                                 ).imag()/phi;
                }
                else {
                    const std::complex<Real> td = phi/(2.0*t1)
                                   *std::complex<Real>(-phi, (j_== 1)? 1 : -1);
                    const std::complex<Real> p = td*sigma2_/(t1+d);
                    const std::complex<Real> g = p*(1.0-ex);

                    return
                        std::exp(v0_*td*(1.0-ex)/(1.0-p*ex)
                                 + (kappa_*theta_)*(td*term_-2.0*g/sigma2_)
                                 + std::complex<Real>(0.0, phi*(dd_-sx_))
                                 + addOnTerm
                                 ).imag()/phi;
                }
            }
            else {
                // use l'Hospital's rule to get lim_{phi->0}
                if (j_ == 1) {
                    const Real kmr = rsigma_-kappa_;
                    if (std::fabs(kmr) > 1e-7) {
                        return dd_-sx_
                            + (std::exp(kmr*term_)*kappa_*theta_
                               -kappa_*theta_*(kmr*term_+1.0) ) / (2*kmr*kmr)
                            - v0_*(1.0-std::exp(kmr*term_)) / (2.0*kmr);
                    }
                    else
                        // \kappa = \rho * \sigma
                        return dd_-sx_ + 0.25*kappa_*theta_*term_*term_
                                       + 0.5*v0_*term_;
                }
                else {
                    return dd_-sx_
                        - (std::exp(-kappa_*term_)*kappa_*theta_
                           +kappa_*theta_*(kappa_*term_-1.0))/(2*kappa_*kappa_)
                        - v0_*(1.0-std::exp(-kappa_*term_))/(2*kappa_);
                }
            }
        }
        else if (cpxLog_ == BranchCorrection) {
            const std::complex<Real> p  = (t1+d)/(t1 - d);

            // next term: g = std::log((1.0 - p*std::exp(d*term_))/(1.0 - p))
            std::complex<Real> g;

            // the exp of the following expression is needed.
            const std::complex<Real> e = std::log(p)+d*term_;

            // does it fit to the machine precision?
            if (std::exp(-e.real()) > QL_EPSILON) {
                g = std::log((1.0 - p/ex)/(1.0 - p));
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
                            + addOnTerm
                            ).imag()/phi;
        }
        else {
            QL_FAIL("unknown complex logarithm formula");
        }
    }

    AnalyticHestonEngine::AnalyticHestonEngine(
                              const boost::shared_ptr<HestonModel>& model,
                              Size integrationOrder)
    : GenericModelEngine<HestonModel,
                         VanillaOption::arguments,
                         VanillaOption::results>(model),
      cpxLog_     (Gatheral),
      integration_(new Integration(
                          Integration::gaussLaguerre(integrationOrder))) {
    }

    AnalyticHestonEngine::AnalyticHestonEngine(
                              const boost::shared_ptr<HestonModel>& model,
                              Real relTolerance, Size maxEvaluations)
    : GenericModelEngine<HestonModel,
                         VanillaOption::arguments,
                         VanillaOption::results>(model),
      cpxLog_(Gatheral),
      integration_(new Integration(Integration::gaussLobatto(
                              relTolerance, Null<Real>(), maxEvaluations))) {
    }

    AnalyticHestonEngine::AnalyticHestonEngine(
                              const boost::shared_ptr<HestonModel>& model,
                              ComplexLogFormula cpxLog,
                              const Integration& integration)
    : GenericModelEngine<HestonModel,
                         VanillaOption::arguments,
                         VanillaOption::results>(model),
      cpxLog_(cpxLog),
      integration_(new Integration(integration)) {
        QL_REQUIRE(   cpxLog_ != BranchCorrection
                   || !integration.isAdaptiveIntegration(),
                   "Branch correction does not work in conjunction "
                   "with adaptive integration methods");
    }

    Size AnalyticHestonEngine::numberOfEvaluations() const {
        return evaluations_;
    }

    void AnalyticHestonEngine::doCalculation(Real riskFreeDiscount,
                                             Real dividendDiscount,
                                             Real spotPrice,
                                             Real strikePrice,
                                             Real term,
                                             Real kappa, Real theta, Real sigma, Real v0, Real rho,
                                             const TypePayoff& type,
                                             const Integration& integration,
                                             const ComplexLogFormula cpxLog,
                                             const AnalyticHestonEngine* const enginePtr,
                                             Real& value,
                                             Size& evaluations)
    {

        const Real ratio = riskFreeDiscount/dividendDiscount;

        const Real c_inf = std::min(10.0, std::max(0.0001,
                std::sqrt(1.0-square<Real>()(rho))/sigma))
                *(v0 + kappa*theta*term);

        evaluations = 0;
        const Real p1 = integration.calculate(c_inf,
            Fj_Helper(kappa, theta, sigma, v0, spotPrice, rho, enginePtr,
                      cpxLog, term, strikePrice, ratio, 1))/M_PI;
        evaluations+= integration.numberOfEvaluations();

        const Real p2 = integration.calculate(c_inf,
            Fj_Helper(kappa, theta, sigma, v0, spotPrice, rho, enginePtr,
                      cpxLog, term, strikePrice, ratio, 2))/M_PI;
        evaluations+= integration.numberOfEvaluations();

        switch (type.optionType())
        {
          case Option::Call:
            value = spotPrice*dividendDiscount*(p1+0.5)
                           - strikePrice*riskFreeDiscount*(p2+0.5);
            break;
          case Option::Put:
            value = spotPrice*dividendDiscount*(p1-0.5)
                           - strikePrice*riskFreeDiscount*(p2-0.5);
            break;
          default:
            QL_FAIL("unknown option type");
        }

    }

    void AnalyticHestonEngine::calculate() const
    {
        // this is a european option pricer
        QL_REQUIRE(arguments_.exercise->type() == Exercise::European,
                   "not an European option");

        // plain vanilla
        boost::shared_ptr<PlainVanillaPayoff> payoff =
            boost::dynamic_pointer_cast<PlainVanillaPayoff>(arguments_.payoff);
        QL_REQUIRE(payoff, "non plain vanilla payoff given");

        const boost::shared_ptr<HestonProcess>& process = model_->process();

        const Real riskFreeDiscount = process->riskFreeRate()->discount(
                                            arguments_.exercise->lastDate());
        const Real dividendDiscount = process->dividendYield()->discount(
                                            arguments_.exercise->lastDate());

        const Real spotPrice = process->s0()->value();
        QL_REQUIRE(spotPrice > 0.0, "negative or null underlying given");

        const Real strikePrice = payoff->strike();
        const Real term = process->time(arguments_.exercise->lastDate());

        doCalculation(riskFreeDiscount,
                      dividendDiscount,
                      spotPrice,
                      strikePrice,
                      term,
                      model_->kappa(),
                      model_->theta(),
                      model_->sigma(),
                      model_->v0(),
                      model_->rho(),
                      *payoff,
                      *integration_,
                      cpxLog_,
                      this,
                      results_.value,
                      evaluations_);
    }


    AnalyticHestonEngine::Integration::Integration(
            Algorithm intAlgo,
            const boost::shared_ptr<Integrator>& integrator)
    : intAlgo_(intAlgo),
      integrator_(integrator) { }

    AnalyticHestonEngine::Integration::Integration(
            Algorithm intAlgo,
            const boost::shared_ptr<GaussianQuadrature>& gaussianQuadrature)
    : intAlgo_(intAlgo),
      gaussianQuadrature_(gaussianQuadrature) { }

    AnalyticHestonEngine::Integration
    AnalyticHestonEngine::Integration::gaussLobatto(Real relTolerance,
                                                    Real absTolerance,
                                                    Size maxEvaluations) {
        return Integration(GaussLobatto,
                           boost::shared_ptr<Integrator>(
                               new GaussLobattoIntegral(maxEvaluations,
                                                        absTolerance,
                                                        relTolerance,
                                                        false)));
    }

    AnalyticHestonEngine::Integration
    AnalyticHestonEngine::Integration::gaussKronrod(Real absTolerance,
                                                   Size maxEvaluations) {
        return Integration(GaussKronrod,
                           boost::shared_ptr<Integrator>(
                               new GaussKronrodAdaptive(absTolerance,
                                                        maxEvaluations)));
    }

    AnalyticHestonEngine::Integration
    AnalyticHestonEngine::Integration::simpson(Real absTolerance,
                                               Size maxEvaluations) {
        return Integration(Simpson,
                           boost::shared_ptr<Integrator>(
                               new SimpsonIntegral(absTolerance,
                                                   maxEvaluations)));
    }

    AnalyticHestonEngine::Integration
    AnalyticHestonEngine::Integration::trapezoid(Real absTolerance,
                                        Size maxEvaluations) {
        return Integration(Trapezoid,
                           boost::shared_ptr<Integrator>(
                              new TrapezoidIntegral<Default>(absTolerance,
                                                             maxEvaluations)));
    }

    AnalyticHestonEngine::Integration
    AnalyticHestonEngine::Integration::gaussLaguerre(Size intOrder) {
        QL_REQUIRE(intOrder <= 192, "maximum integraton order (192) exceeded");
        return Integration(GaussLaguerre,
                           boost::shared_ptr<GaussianQuadrature>(
                               new GaussLaguerreIntegration(intOrder)));
    }

    AnalyticHestonEngine::Integration
    AnalyticHestonEngine::Integration::gaussLegendre(Size intOrder) {
        return Integration(GaussLegendre,
                           boost::shared_ptr<GaussianQuadrature>(
                               new GaussLegendreIntegration(intOrder)));
    }

    AnalyticHestonEngine::Integration
    AnalyticHestonEngine::Integration::gaussChebyshev(Size intOrder) {
        return Integration(GaussChebyshev,
                           boost::shared_ptr<GaussianQuadrature>(
                               new GaussChebyshevIntegration(intOrder)));
    }

    AnalyticHestonEngine::Integration
    AnalyticHestonEngine::Integration::gaussChebyshev2nd(Size intOrder) {
        return Integration(GaussChebyshev2nd,
                           boost::shared_ptr<GaussianQuadrature>(
                               new GaussChebyshev2ndIntegration(intOrder)));
    }

    Size AnalyticHestonEngine::Integration::numberOfEvaluations() const {
        if (integrator_) {
            return integrator_->numberOfEvaluations();
        }
        else if (gaussianQuadrature_) {
            return gaussianQuadrature_->order();
        }
        else {
            QL_FAIL("neither Integrator nor GaussianQuadrature given");
        }
    }

    bool AnalyticHestonEngine::Integration::isAdaptiveIntegration() const {
        return intAlgo_ == GaussLobatto
            || intAlgo_ == GaussKronrod
            || intAlgo_ == Simpson
            || intAlgo_ == Trapezoid;
    }

    Real AnalyticHestonEngine::Integration::calculate(
                               Real c_inf,
                               const boost::function1<Real, Real>& f) const {
        Real retVal;

        switch(intAlgo_) {
          case GaussLaguerre:
            retVal = gaussianQuadrature_->operator()(f);
            break;
          case GaussLegendre:
          case GaussChebyshev:
          case GaussChebyshev2nd:
            retVal = gaussianQuadrature_->operator()(
                boost::function1<Real, Real>(
                    if_then_else_return ( (boost::lambda::_1+1.0)*c_inf 
                                          > QL_EPSILON,
                        boost::lambda::bind(f, -boost::lambda::bind(
                            std::ptr_fun<Real,Real>(std::log),
                            0.5*boost::lambda::_1+0.5 )/c_inf )
                                          /((boost::lambda::_1+1.0)*c_inf),
                        boost::lambda::bind(constant<Real, Real>(0.0), 
                                            boost::lambda::_1))));
            break;
          case Simpson:
          case Trapezoid:
          case GaussLobatto:
          case GaussKronrod:
            retVal = integrator_->operator()(
                boost::function1<Real, Real>(
                    if_then_else_return ( boost::lambda::_1*c_inf > QL_EPSILON,
                        boost::lambda::bind(f,-boost::lambda::bind(
                            std::ptr_fun<Real,Real>(std::log), 
                            boost::lambda::_1)/c_inf) /(boost::lambda::_1*c_inf),
                        boost::lambda::bind(constant<Real, Real>(0.0), 
                                            boost::lambda::_1))),
                0.0, 1.0);
            break;
          default:
              QL_FAIL("unknwon integration algorithm");
        }

        return retVal;
     }
}
