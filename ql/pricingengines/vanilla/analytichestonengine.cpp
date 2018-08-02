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

#include <ql/math/solvers1d/brent.hpp>
#include <ql/math/integrals/simpsonintegral.hpp>
#include <ql/math/integrals/kronrodintegral.hpp>
#include <ql/math/integrals/trapezoidintegral.hpp>
#include <ql/math/integrals/discreteintegrals.hpp>
#include <ql/math/integrals/gausslobattointegral.hpp>

#include <ql/instruments/payoffs.hpp>
#include <ql/pricingengines/blackcalculator.hpp>
#include <ql/pricingengines/vanilla/analytichestonengine.hpp>


#if defined(QL_PATCH_MSVC)
#pragma warning(disable: 4180)
#endif

namespace QuantLib {

    namespace {

        class integrand1 {
          private:
            const Real c_inf_;
            const boost::function<Real(Real)> f_;
          public:
            integrand1(Real c_inf, const boost::function<Real(Real)>& f)
            : c_inf_(c_inf), f_(f) {}
            Real operator()(Real x) const {
                if ((1.0-x)*c_inf_ > QL_EPSILON)
                    return f_(-std::log(0.5-0.5*x)/c_inf_)/((1.0-x)*c_inf_);
                else
                    return 0.0;
            }
        };

        class integrand2 {
          private:
            const Real c_inf_;
            const boost::function<Real(Real)> f_;
          public:
            integrand2(Real c_inf, const boost::function<Real(Real)>& f)
            : c_inf_(c_inf), f_(f) {}
            Real operator()(Real x) const {
                if (x*c_inf_ > QL_EPSILON) {
                    return f_(-std::log(x)/c_inf_)/(x*c_inf_);
                } else {
                    return 0.0;
                }
            }
        };

        class integrand3 {
          private:
            const integrand2 int_;
          public:
            integrand3(Real c_inf, const boost::function<Real(Real)>& f)
            : int_(c_inf, f) {}

            Real operator()(Real x) const { return int_(1.0-x); }
        };

        class u_Max : public std::unary_function<Real, Real> {
          public:
            u_Max(Real c_inf, Real epsilon)
            : c_inf_(c_inf), logEpsilon_(std::log(epsilon)),
              evaluations_(0) {}

            Real operator()(Real u) const {
                ++evaluations_;
                return c_inf_*u + std::log(u) + logEpsilon_;
            }

            Size evaluations() const { return evaluations_; }

          private:
            const Real c_inf_, logEpsilon_;
            mutable Size evaluations_;
        };


        class uHat_Max : public std::unary_function<Real, Real> {
          public:
            uHat_Max(Real v0T2, Real epsilon)
            : v0T2_(v0T2), logEpsilon_(std::log(epsilon)),
              evaluations_(0) {}

            Real operator()(Real u) const {
                ++evaluations_;
                return v0T2_*u*u + std::log(u) + logEpsilon_;
            }

            Size evaluations() const { return evaluations_; }

          private:
            const Real v0T2_, logEpsilon_;
            mutable Size evaluations_;
        };
    }

    // helper class for integration
    class AnalyticHestonEngine::Fj_Helper
        : public std::unary_function<Real, Real>
    {
    public:
        Fj_Helper(const VanillaOption::arguments& arguments,
            const ext::shared_ptr<HestonModel>& model,
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

        Real operator()(Real phi) const;

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
        const ext::shared_ptr<HestonModel>& model,
        const AnalyticHestonEngine* const engine,
        ComplexLogFormula cpxLog,
        Time term, Real ratio, Size j)
        : j_ (j), //arg_(arguments),
        kappa_(model->kappa()), theta_(model->theta()),
        sigma_(model->sigma()), v0_(model->v0()),
        cpxLog_(cpxLog), term_(term),
        x_(std::log(model->process()->s0()->value())),
        sx_(std::log(ext::dynamic_pointer_cast<StrikedTypePayoff>
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
            = engine_ ? engine_->addOnTerm(phi, term_, j_) : Real(0.0);

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
            const std::complex<Real> p = (t1+d)/(t1-d);

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


    class AnalyticHestonEngine::AP_Helper
        : public std::unary_function<Real, Real> {
      public:
        AP_Helper(Time term, Real s0, Real strike, Real ratio,
                  Volatility sigmaBS,
                  const AnalyticHestonEngine* const enginePtr)
        : term_(term),
          sigmaBS_(sigmaBS),
          x_(std::log(s0)),
          sx_(std::log(strike)),
          dd_(x_-std::log(ratio)),
          enginePtr_(enginePtr) {
            QL_REQUIRE(enginePtr != 0, "pricing engine required");
        }

        Real operator()(Real u) const {
            QL_REQUIRE(   enginePtr_->addOnTerm(u, term_, 1)
                            == std::complex<Real>(0.0)
                       && enginePtr_->addOnTerm(u, term_, 2)
                            == std::complex<Real>(0.0),
                       "only Heston model is supported");

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
        const AnalyticHestonEngine* const enginePtr_;
    };

    std::complex<Real> AnalyticHestonEngine::lnChF(
        const std::complex<Real>& z, Time t) const {

        const Real kappa = model_->kappa();
        const Real sigma = model_->sigma();
        const Real theta = model_->theta();
        const Real rho   = model_->rho();
        const Real v0    = model_->v0();

        const Real sigma2 = sigma*sigma;

        const std::complex<Real> g
            = kappa + rho*sigma*std::complex<Real>(z.imag(), -z.real());

        const std::complex<Real> D = std::sqrt(
            g*g + (z*z + std::complex<Real>(-z.imag(), z.real()))*sigma2);

        const std::complex<Real> G = (g-D)/(g+D);

        return v0/sigma2*(1.0-std::exp(-D*t))/(1.0-G*std::exp(-D*t))
                *(g-D) + kappa*theta/sigma2*((g-D)*t
                -2.0*std::log((1.0-G*std::exp(-D*t))/(1.0-G)));
    }

    std::complex<Real> AnalyticHestonEngine::chF(
        const std::complex<Real>& z, Time T) const {
        return std::exp(lnChF(z, T));
    }

    AnalyticHestonEngine::AnalyticHestonEngine(
                              const ext::shared_ptr<HestonModel>& model,
                              Size integrationOrder)
    : GenericModelEngine<HestonModel,
                         VanillaOption::arguments,
                         VanillaOption::results>(model),
      evaluations_(0),
      cpxLog_     (Gatheral),
      integration_(new Integration(
                          Integration::gaussLaguerre(integrationOrder))),
      andersenPiterbargEpsilon_(Null<Real>()) {
    }

    AnalyticHestonEngine::AnalyticHestonEngine(
                              const ext::shared_ptr<HestonModel>& model,
                              Real relTolerance, Size maxEvaluations)
    : GenericModelEngine<HestonModel,
                         VanillaOption::arguments,
                         VanillaOption::results>(model),
      evaluations_(0),
      cpxLog_(Gatheral),
      integration_(new Integration(Integration::gaussLobatto(
                              relTolerance, Null<Real>(), maxEvaluations))),
      andersenPiterbargEpsilon_(Null<Real>()) {
    }

    AnalyticHestonEngine::AnalyticHestonEngine(
                              const ext::shared_ptr<HestonModel>& model,
                              ComplexLogFormula cpxLog,
                              const Integration& integration,
                              const Real andersenPiterbargEpsilon)
    : GenericModelEngine<HestonModel,
                         VanillaOption::arguments,
                         VanillaOption::results>(model),
      evaluations_(0),
      cpxLog_(cpxLog),
      integration_(new Integration(integration)),
      andersenPiterbargEpsilon_(andersenPiterbargEpsilon) {
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
                                             Size& evaluations) {

        const Real ratio = riskFreeDiscount/dividendDiscount;

        evaluations = 0;

        switch(cpxLog) {
          case Gatheral:
          case BranchCorrection: {
            const Real c_inf = std::min(0.2, std::max(0.0001,
                std::sqrt(1.0-rho*rho)/sigma))*(v0 + kappa*theta*term);

            const Real p1 = integration.calculate(c_inf,
                Fj_Helper(kappa, theta, sigma, v0, spotPrice, rho, enginePtr,
                          cpxLog, term, strikePrice, ratio, 1))/M_PI;
            evaluations += integration.numberOfEvaluations();

            const Real p2 = integration.calculate(c_inf,
                Fj_Helper(kappa, theta, sigma, v0, spotPrice, rho, enginePtr,
                          cpxLog, term, strikePrice, ratio, 2))/M_PI;
            evaluations += integration.numberOfEvaluations();

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
          break;
          case AndersenPiterbarg: {
            const Real c_inf =
                std::sqrt(1.0-rho*rho)*(v0 + kappa*theta*term)/sigma;

            const Real fwdPrice = spotPrice / ratio;

            const Real epsilon = enginePtr->andersenPiterbargEpsilon_
                *M_PI/(std::sqrt(strikePrice*fwdPrice)*riskFreeDiscount);

            const Real uM = Integration::andersenPiterbargIntegrationLimit(
                c_inf, epsilon, v0, term);

            const Real vAvg
                = (1-std::exp(-kappa*term))*(v0-theta)/(kappa*term) + theta;

            const Real bsPrice
                = BlackCalculator(Option::Call, strikePrice,
                                  fwdPrice, std::sqrt(vAvg*term),
                                  riskFreeDiscount).value();

            const Real h_cv = integration.calculate(c_inf,
                    AP_Helper(term, spotPrice, strikePrice,
                              ratio, std::sqrt(vAvg), enginePtr), uM)
                * std::sqrt(strikePrice * fwdPrice)*riskFreeDiscount/M_PI;
            evaluations += integration.numberOfEvaluations();

            switch (type.optionType())
            {
              case Option::Call:
                value = bsPrice + h_cv;
                break;
              case Option::Put:
                value = bsPrice + h_cv
                    - riskFreeDiscount*(fwdPrice - strikePrice);
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

    void AnalyticHestonEngine::calculate() const
    {
        // this is a european option pricer
        QL_REQUIRE(arguments_.exercise->type() == Exercise::European,
                   "not an European option");

        // plain vanilla
        ext::shared_ptr<PlainVanillaPayoff> payoff =
            ext::dynamic_pointer_cast<PlainVanillaPayoff>(arguments_.payoff);
        QL_REQUIRE(payoff, "non plain vanilla payoff given");

        const ext::shared_ptr<HestonProcess>& process = model_->process();

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
            const ext::shared_ptr<Integrator>& integrator)
    : intAlgo_(intAlgo),
      integrator_(integrator) { }

    AnalyticHestonEngine::Integration::Integration(
            Algorithm intAlgo,
            const ext::shared_ptr<GaussianQuadrature>& gaussianQuadrature)
    : intAlgo_(intAlgo),
      gaussianQuadrature_(gaussianQuadrature) { }

    AnalyticHestonEngine::Integration
    AnalyticHestonEngine::Integration::gaussLobatto(Real relTolerance,
                                                    Real absTolerance,
                                                    Size maxEvaluations) {
        return Integration(GaussLobatto,
                           ext::shared_ptr<Integrator>(
                               new GaussLobattoIntegral(maxEvaluations,
                                                        absTolerance,
                                                        relTolerance,
                                                        false)));
    }

    AnalyticHestonEngine::Integration
    AnalyticHestonEngine::Integration::gaussKronrod(Real absTolerance,
                                                   Size maxEvaluations) {
        return Integration(GaussKronrod,
                           ext::shared_ptr<Integrator>(
                               new GaussKronrodAdaptive(absTolerance,
                                                        maxEvaluations)));
    }

    AnalyticHestonEngine::Integration
    AnalyticHestonEngine::Integration::simpson(Real absTolerance,
                                               Size maxEvaluations) {
        return Integration(Simpson,
                           ext::shared_ptr<Integrator>(
                               new SimpsonIntegral(absTolerance,
                                                   maxEvaluations)));
    }

    AnalyticHestonEngine::Integration
    AnalyticHestonEngine::Integration::trapezoid(Real absTolerance,
                                        Size maxEvaluations) {
        return Integration(Trapezoid,
                           ext::shared_ptr<Integrator>(
                              new TrapezoidIntegral<Default>(absTolerance,
                                                             maxEvaluations)));
    }

    AnalyticHestonEngine::Integration
    AnalyticHestonEngine::Integration::gaussLaguerre(Size intOrder) {
        QL_REQUIRE(intOrder <= 192, "maximum integraton order (192) exceeded");
        return Integration(GaussLaguerre,
                           ext::shared_ptr<GaussianQuadrature>(
                               new GaussLaguerreIntegration(intOrder)));
    }

    AnalyticHestonEngine::Integration
    AnalyticHestonEngine::Integration::gaussLegendre(Size intOrder) {
        return Integration(GaussLegendre,
                           ext::shared_ptr<GaussianQuadrature>(
                               new GaussLegendreIntegration(intOrder)));
    }

    AnalyticHestonEngine::Integration
    AnalyticHestonEngine::Integration::gaussChebyshev(Size intOrder) {
        return Integration(GaussChebyshev,
                           ext::shared_ptr<GaussianQuadrature>(
                               new GaussChebyshevIntegration(intOrder)));
    }

    AnalyticHestonEngine::Integration
    AnalyticHestonEngine::Integration::gaussChebyshev2nd(Size intOrder) {
        return Integration(GaussChebyshev2nd,
                           ext::shared_ptr<GaussianQuadrature>(
                               new GaussChebyshev2ndIntegration(intOrder)));
    }

    AnalyticHestonEngine::Integration
    AnalyticHestonEngine::Integration::discreteSimpson(Size evaluations) {
        return Integration(
            DiscreteSimpson, ext::shared_ptr<Integrator>(
                new DiscreteSimpsonIntegrator(evaluations)));
    }

    AnalyticHestonEngine::Integration
    AnalyticHestonEngine::Integration::discreteTrapezoid(Size evaluations) {
        return Integration(
            DiscreteTrapezoid, ext::shared_ptr<Integrator>(
                new DiscreteTrapezoidIntegrator(evaluations)));
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
                               const boost::function1<Real, Real>& f,
                               Real maxBound) const {
        Real retVal;

        switch(intAlgo_) {
          case GaussLaguerre:
            retVal = (*gaussianQuadrature_)(f);
            break;
          case GaussLegendre:
          case GaussChebyshev:
          case GaussChebyshev2nd:
            retVal = (*gaussianQuadrature_)(integrand1(c_inf, f));
            break;
          case Simpson:
          case Trapezoid:
          case GaussLobatto:
          case GaussKronrod:
            if (maxBound == Null<Real>())
                retVal = (*integrator_)(integrand2(c_inf, f), 0.0, 1.0);
            else
                retVal = (*integrator_)(f, 0.0, maxBound);
            break;
          case DiscreteTrapezoid:
          case DiscreteSimpson:
            if (maxBound == Null<Real>())
                retVal = (*integrator_)(integrand3(c_inf, f), 0.0, 1.0);
            else
                retVal = (*integrator_)(f, 0.0, maxBound);
            break;
          default:
            QL_FAIL("unknwon integration algorithm");
        }

        return retVal;
     }

    Real AnalyticHestonEngine::Integration::andersenPiterbargIntegrationLimit(
        Real c_inf, Real epsilon, Real v0, Real t) {

        const Real uMaxGuess = -std::log(epsilon)/c_inf;
        const Real uMaxStep = 0.1*uMaxGuess;

        const Real uMax = Brent().solve(u_Max(c_inf, epsilon),
            QL_EPSILON*uMaxGuess, uMaxGuess, uMaxStep);

        const Real uHatMax = Brent().solve(uHat_Max(0.5*v0*t, epsilon),
            QL_EPSILON*std::sqrt(uMaxGuess),
            std::sqrt(uMaxGuess), 0.1*std::sqrt(uMaxGuess));

        return std::max(uMax, uHatMax);
    }
}
