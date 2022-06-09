/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005, 2007, 2009, 2014 Klaus Spanderen

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

#include <ql/math/distributions/chisquaredistribution.hpp>
#include <ql/math/distributions/normaldistribution.hpp>
#include <ql/math/functional.hpp>
#include <ql/math/integrals/gaussianquadratures.hpp>
#include <ql/math/integrals/gausslobattointegral.hpp>
#include <ql/math/integrals/segmentintegral.hpp>
#include <ql/math/modifiedbessel.hpp>
#include <ql/math/solvers1d/brent.hpp>
#include <ql/processes/eulerdiscretization.hpp>
#include <ql/processes/hestonprocess.hpp>
#include <ql/quotes/simplequote.hpp>
#include <boost/math/distributions/non_central_chi_squared.hpp>
#include <complex>
#include <utility>

namespace QuantLib {

    HestonProcess::HestonProcess(Handle<YieldTermStructure> riskFreeRate,
                                 Handle<YieldTermStructure> dividendYield,
                                 Handle<Quote> s0,
                                 Real v0,
                                 Real kappa,
                                 Real theta,
                                 Real sigma,
                                 Real rho,
                                 Discretization d)
    : StochasticProcess(ext::shared_ptr<discretization>(new EulerDiscretization)),
      riskFreeRate_(std::move(riskFreeRate)), dividendYield_(std::move(dividendYield)),
      s0_(std::move(s0)), v0_(v0), kappa_(kappa), theta_(theta), sigma_(sigma), rho_(rho),
      discretization_(d) {

        registerWith(riskFreeRate_);
        registerWith(dividendYield_);
        registerWith(s0_);
    }

    Size HestonProcess::size() const {
        return 2;
    }

    Size HestonProcess::factors() const {
        return (   discretization_ == BroadieKayaExactSchemeLobatto
                || discretization_ == BroadieKayaExactSchemeTrapezoidal
                || discretization_ == BroadieKayaExactSchemeLaguerre) ? 3 : 2;
    }

    Array HestonProcess::initialValues() const {
        return { s0_->value(), v0_ };
    }

    Array HestonProcess::drift(Time t, const Array& x) const {
        const Real vol = (x[1] > 0.0) ? std::sqrt(x[1])
                         : (discretization_ == Reflection) ? - std::sqrt(-x[1])
                         : 0.0;

        return {
            riskFreeRate_->forwardRate(t, t, Continuous).rate()
               - dividendYield_->forwardRate(t, t, Continuous).rate()
               - 0.5 * vol * vol,
            kappa_* (theta_-((discretization_==PartialTruncation) ? x[1] : vol*vol))
        };
    }

    Matrix HestonProcess::diffusion(Time, const Array& x) const {
        /* the correlation matrix is
           |  1   rho |
           | rho   1  |
           whose square root (which is used here) is
           |  1          0       |
           | rho   sqrt(1-rho^2) |
        */
        Matrix tmp(2,2);
        const Real vol = (x[1] > 0.0) ? std::sqrt(x[1])
                         : (discretization_ == Reflection) ? -std::sqrt(-x[1])
                         : 1e-8; // set vol to (almost) zero but still
                                 // expose some correlation information
        const Real sigma2 = sigma_ * vol;
        const Real sqrhov = std::sqrt(1.0 - rho_*rho_);

        tmp[0][0] = vol;          tmp[0][1] = 0.0;
        tmp[1][0] = rho_*sigma2;  tmp[1][1] = sqrhov*sigma2;
        return tmp;
    }

    Array HestonProcess::apply(const Array& x0,
                               const Array& dx) const {
        return {
            x0[0] * std::exp(dx[0]),
            x0[1] + dx[1]
        };
    }

    namespace {
        // This is the continuous version of a characteristic function
        // for the exact sampling of the Heston process, s. page 8, formula 13,
        // M. Broadie, O. Kaya, Exact Simulation of Stochastic Volatility and
        // other Affine Jump Diffusion Processes
        // http://finmath.stanford.edu/seminars/documents/Broadie.pdf
        //
        // This version does not need a branch correction procedure.
        // For details please see:
        // Roger Lord, "Efficient Pricing Algorithms for exotic Derivatives",
        // http://repub.eur.nl/pub/13917/LordR-Thesis.pdf
        std::complex<Real> Phi(const HestonProcess& process,
                               const std::complex<Real>& a,
                               Real nu_0, Real nu_t, Time dt) {
            const Real theta = process.theta();
            const Real kappa = process.kappa();
            const Real sigma = process.sigma();

            const Volatility sigma2 = sigma*sigma;
            const std::complex<Real> ga = std::sqrt(
                    kappa*kappa - 2*sigma2*a*std::complex<Real>(0.0, 1.0));
            const Real d = 4*theta*kappa/sigma2;

            const Real nu = 0.5*d-1;
            const std::complex<Real> z
                = ga*std::exp(-0.5*ga*dt)/(1.0-std::exp(-ga*dt));
            const std::complex<Real> log_z
                = -0.5*ga*dt + std::log(ga/(1.0-std::exp(-ga*dt)));

            const std::complex<Real> alpha
                = 4.0*ga*std::exp(-0.5*ga*dt)/(sigma2*(1.0-std::exp(-ga*dt)));
            const std::complex<Real> beta = 4.0*kappa*std::exp(-0.5*kappa*dt)
                                           /(sigma2*(1.0-std::exp(-kappa*dt)));

            return ga*std::exp(-0.5*(ga-kappa)*dt)*(1-std::exp(-kappa*dt))
                    / (kappa*(1.0-std::exp(-ga*dt)))
                   *std::exp((nu_0+nu_t)/sigma2 * (
                      kappa*(1.0+std::exp(-kappa*dt))/(1.0-std::exp(-kappa*dt))
                        - ga*(1.0+std::exp(-ga*dt))/(1.0-std::exp(-ga*dt))))
                   *std::exp(nu*log_z)/std::pow(z, nu)
                   *((nu_t > 1e-8)
                           ?   modifiedBesselFunction_i(
                                   nu, std::sqrt(nu_0*nu_t)*alpha)
                             / modifiedBesselFunction_i(
                                   nu, std::sqrt(nu_0*nu_t)*beta)
                           : std::pow(alpha/beta, nu)
                     );
        }

        Real ch(const HestonProcess& process,
                Real x, Real u, Real nu_0, Real nu_t, Time dt) {
            return M_2_PI*std::sin(u*x)/u
                    * Phi(process, u, nu_0, nu_t, dt).real();
        }

        Real ph(const HestonProcess& process,
                Real x, Real u, Real nu_0, Real nu_t, Time dt) {
            return M_2_PI*std::cos(u*x)*Phi(process, u, nu_0, nu_t, dt).real();
        }

        Real int_ph(const HestonProcess& process,
                    Real a, Real x, Real y, Real nu_0, Real nu_t, Time t) {
            static const GaussLaguerreIntegration gaussLaguerreIntegration(128);

            const Real rho   = process.rho();
            const Real kappa = process.kappa();
            const Real sigma = process.sigma();
            const Real x0    = std::log(process.s0()->value());

            return gaussLaguerreIntegration(
                [&](Real u){ return ph(process, y, u, nu_0, nu_t, t); })
                / std::sqrt(2*M_PI*(1-rho*rho)*y)
                * std::exp(-0.5*squared(x - x0 - a + y*(0.5-rho*kappa/sigma))
                           /(y*(1-rho*rho)));
        }


        Real pade(Real x, const Real* nominator, const Real* denominator, Size m) {
            Real n=0.0, d=0.0;
            for (Integer i=m-1; i >= 0; --i) {
                n = (n+nominator[i])*x;
                d = (d+denominator[i])*x;
            }
            return (1+n)/(1+d);
        }

        // For the definition of the Pade approximation please see e.g.
        // http://wikipedia.org/wiki/Sine_integral#Sine_integral
        Real Si(Real x) {
            if (x <=4.0) {
                const Real n[] =
                    { -4.54393409816329991e-2,1.15457225751016682e-3,
                      -1.41018536821330254e-5,9.43280809438713025e-8,
                      -3.53201978997168357e-10,7.08240282274875911e-13,
                      -6.05338212010422477e-16 };
                const Real d[] =
                    {  1.01162145739225565e-2,4.99175116169755106e-5,
                       1.55654986308745614e-7,3.28067571055789734e-10,
                       4.5049097575386581e-13,3.21107051193712168e-16,
                       0.0 };

                return x*pade(x*x, n, d, sizeof(n)/sizeof(Real));
            }
            else {
                const Real y = 1/(x*x);
                const Real fn[] =
                    { 7.44437068161936700618e2,1.96396372895146869801e5,
                      2.37750310125431834034e7,1.43073403821274636888e9,
                      4.33736238870432522765e10,6.40533830574022022911e11,
                      4.20968180571076940208e12,1.00795182980368574617e13,
                      4.94816688199951963482e12,-4.94701168645415959931e11 };
                const Real fd[] =
                    { 7.46437068161927678031e2,1.97865247031583951450e5,
                      2.41535670165126845144e7,1.47478952192985464958e9,
                      4.58595115847765779830e10,7.08501308149515401563e11,
                      5.06084464593475076774e12,1.43468549171581016479e13,
                      1.11535493509914254097e13, 0.0 };
                const Real f = pade(y, fn, fd, 10)/x;

                const Real gn[] =
                    { 8.1359520115168615e2,2.35239181626478200e5,
                      3.12557570795778731e7,2.06297595146763354e9,
                      6.83052205423625007e10,1.09049528450362786e12,
                      7.57664583257834349e12,1.81004487464664575e13,
                      6.43291613143049485e12,-1.36517137670871689e12 };
                const Real gd[] =
                    { 8.19595201151451564e2,2.40036752835578777e5,
                      3.26026661647090822e7,2.23355543278099360e9,
                      7.87465017341829930e10,1.39866710696414565e12,
                      1.17164723371736605e13,4.01839087307656620e13,
                      3.99653257887490811e13, 0.0};
                const Real g = y*pade(y, gn, gd, 10);

                return M_PI_2 - f*std::cos(x)-g*std::sin(x);
            }
        }

        Real cornishFisherEps(const HestonProcess& process,
                              Real nu_0, Real nu_t, Time dt, Real eps) {
            // use moment generating function to get the
            // first,second, third and fourth moment of the distribution
            const Real d = 1e-2;
            const Real p2 = Phi(process, std::complex<Real>(0, -2*d),
                                                nu_0, nu_t, dt).real();
            const Real p1 = Phi(process, std::complex<Real>(0, -d),
                                                nu_0, nu_t, dt).real();
            const Real p0 = Phi(process, std::complex<Real>(0, 0),
                                                nu_0, nu_t, dt).real();
            const Real pm1= Phi(process, std::complex<Real>(0, d),
                                                 nu_0, nu_t, dt).real();
            const Real pm2= Phi(process, std::complex<Real>(0, 2*d),
                                                 nu_0, nu_t, dt).real();

            const Real avg    = (pm2-8*pm1+8*p1-p2)/(12*d);
            const Real m2     = (-pm2+16*pm1-30*p0+16*p1-p2)/(12*d*d);
            const Real var    = m2 - avg*avg;
            const Real stdDev = std::sqrt(var);

            const Real m3 = (-0.5*pm2 + pm1 - p1 + 0.5*p2)/(d*d*d);
            const Real skew
                = (m3 - 3*var*avg - avg*avg*avg) / (var*stdDev);

            const Real m4 = (pm2 - 4*pm1 + 6*p0 - 4*p1 + p2)/(d*d*d*d);
            const Real kurt
                 =  (m4 - 4*m3*avg + 6*m2*avg*avg - 3*avg*avg*avg*avg)
                   /(var*var);

            // Cornish-Fisher relation to come up with an improved
            // estimate of 1-F(u_\eps) < \eps
            const Real q = InverseCumulativeNormal()(1-eps);
            const Real w =  q + (q*q-1)/6*skew + (q*q*q-3*q)/24*(kurt-3)
                          - (2*q*q*q-5*q)/36*skew*skew;

            return avg + w*stdDev;
        }

        Real cdf_nu_ds(const HestonProcess& process,
                       Real x, Real nu_0, Real nu_t, Time dt,
                       HestonProcess::Discretization discretization) {
            const Real eps = 1e-4;
            const Real u_eps = std::min(100.0,
                std::max(0.1, cornishFisherEps(process, nu_0, nu_t, dt, eps)));

            switch (discretization) {
              case HestonProcess::BroadieKayaExactSchemeLaguerre:
              {
                  static const GaussLaguerreIntegration
                      gaussLaguerreIntegration(128);

                // get the upper bound for the integration
                Real upper = u_eps/2.0;
                while (std::abs(Phi(process,upper,nu_0,nu_t,dt)/upper)
                        > eps) upper*=2.0;

                return (x < upper)
                    ? std::max(0.0, std::min(1.0,
                        gaussLaguerreIntegration(
                            [&](Real u){ return ch(process, x, u, nu_0, nu_t, dt); })))
                    : 1.0;
              }
              case HestonProcess::BroadieKayaExactSchemeLobatto:
              {
                // get the upper bound for the integration
                Real upper = u_eps/2.0;
                while (std::abs(Phi(process, upper,nu_0,nu_t,dt)/upper)
                        >  eps) upper*=2.0;

                return (x < upper)
                    ? std::max(0.0, std::min(1.0,
                        GaussLobattoIntegral(Null<Size>(), eps)(
                            [&](Real xi){ return ch(process, x, xi, nu_0, nu_t, dt); },
                            QL_EPSILON, upper)))
                    : 1.0;
              }
              case HestonProcess::BroadieKayaExactSchemeTrapezoidal:
              {
                const Real h = 0.05;

                Real si = Si(0.5*h*x);
                Real s = M_2_PI*si;
                std::complex<Real> f;
                Size j = 0;
                do {
                    ++j;
                    const Real u = h*j;
                    const Real si_n = Si(x*(u+0.5*h));

                    f = Phi(process, u, nu_0, nu_t, dt);
                    s+= M_2_PI*f.real()*(si_n-si);
                    si = si_n;
                }
                while (M_2_PI*std::abs(f)/j > eps);

                return s;
              }
              default:
                QL_FAIL("unknown integration method");
            }
        }
    }

    Real cdf_nu_ds_minus_x(const HestonProcess &process, Real x, Real nu_0,
                           Real nu_t, Time dt,
                           HestonProcess::Discretization discretization,
                           Real x0) {
        return cdf_nu_ds(process, x, nu_0, nu_t, dt, discretization) - x0;
    }

    Real HestonProcess::pdf(Real x, Real v, Time t, Real eps) const {
         const Real k = sigma_*sigma_*(1-std::exp(-kappa_*t))/(4*kappa_);
         const Real a = std::log(  dividendYield_->discount(t)
                                   / riskFreeRate_->discount(t))
                      + rho_/sigma_*(v - v0_ - kappa_*theta_*t);

         const Real x0 = std::log(s0()->value());
         Real upper = std::max(0.1, -(x-x0-a)/(0.5-rho_*kappa_/sigma_)), f=0, df=1;

         while (df > 0.0 || f > 0.1*eps) {
             const Real f1 = x-x0-a+upper*(0.5-rho_*kappa_/sigma_);
             const Real f2 = -0.5*f1*f1/(upper*(1-rho_*rho_));

             df = 1/std::sqrt(2*M_PI*(1-rho_*rho_))
                 * ( -0.5/(upper*std::sqrt(upper))*std::exp(f2)
                    + 1/std::sqrt(upper)*std::exp(f2)*(-0.5/(1-rho_*rho_))
                           *(-1/(upper*upper)*f1*f1
                             + 2/upper*f1*(0.5-rho_*kappa_/sigma_)));

             f = std::exp(f2)/ std::sqrt(2*M_PI*(1-rho_*rho_)*upper);
             upper*=1.5;
         }

         upper = 2.0*cornishFisherEps(*this, v0_, v, t,1e-3);

         return SegmentIntegral(100)(
               [&](Real xi){ return int_ph(*this, a, x, xi, v0_, v, t); },
               QL_EPSILON, upper)
               * boost::math::pdf(
                     boost::math::non_central_chi_squared_distribution<Real>(
                         4*theta_*kappa_/(sigma_*sigma_),
                         4*kappa_*std::exp(-kappa_*t)
                         /((sigma_*sigma_)*(1-std::exp(-kappa_*t)))*v0_),
                     v/k) / k;
     }

    Array HestonProcess::evolve(Time t0, const Array& x0,
                                Time dt, const Array& dw) const {
        Array retVal(2);
        Real vol, vol2, mu, nu, dy;

        const Real sdt = std::sqrt(dt);
        const Real sqrhov = std::sqrt(1.0 - rho_*rho_);

        switch (discretization_) {
          // For the definition of PartialTruncation, FullTruncation
          // and Reflection  see Lord, R., R. Koekkoek and D. van Dijk (2006),
          // "A Comparison of biased simulation schemes for
          //  stochastic volatility models",
          // Working Paper, Tinbergen Institute
          case PartialTruncation:
            vol = (x0[1] > 0.0) ? std::sqrt(x0[1]) : 0.0;
            vol2 = sigma_ * vol;
            mu =    riskFreeRate_->forwardRate(t0, t0+dt, Continuous)
                  - dividendYield_->forwardRate(t0, t0+dt, Continuous)
                    - 0.5 * vol * vol;
            nu = kappa_*(theta_ - x0[1]);

            retVal[0] = x0[0] * std::exp(mu*dt+vol*dw[0]*sdt);
            retVal[1] = x0[1] + nu*dt + vol2*sdt*(rho_*dw[0] + sqrhov*dw[1]);
            break;
          case FullTruncation:
            vol = (x0[1] > 0.0) ? std::sqrt(x0[1]) : 0.0;
            vol2 = sigma_ * vol;
            mu =    riskFreeRate_->forwardRate(t0, t0+dt, Continuous)
                  - dividendYield_->forwardRate(t0, t0+dt, Continuous)
                    - 0.5 * vol * vol;
            nu = kappa_*(theta_ - vol*vol);

            retVal[0] = x0[0] * std::exp(mu*dt+vol*dw[0]*sdt);
            retVal[1] = x0[1] + nu*dt + vol2*sdt*(rho_*dw[0] + sqrhov*dw[1]);
            break;
          case Reflection:
            vol = std::sqrt(std::fabs(x0[1]));
            vol2 = sigma_ * vol;
            mu =    riskFreeRate_->forwardRate(t0, t0+dt, Continuous)
                  - dividendYield_->forwardRate(t0, t0+dt, Continuous)
                    - 0.5 * vol*vol;
            nu = kappa_*(theta_ - vol*vol);

            retVal[0] = x0[0]*std::exp(mu*dt+vol*dw[0]*sdt);
            retVal[1] = vol*vol
                        +nu*dt + vol2*sdt*(rho_*dw[0] + sqrhov*dw[1]);
            break;
          case NonCentralChiSquareVariance:
            // use Alan Lewis trick to decorrelate the equity and the variance
            // process by using y(t)=x(t)-\frac{rho}{sigma}\nu(t)
            // and Ito's Lemma. Then use exact sampling for the variance
            // process. For further details please read the Wilmott thread
            // "QuantLib code is very high quality"
            vol = (x0[1] > 0.0) ? std::sqrt(x0[1]) : 0.0;
            mu =   riskFreeRate_->forwardRate(t0, t0+dt, Continuous)
                 - dividendYield_->forwardRate(t0, t0+dt, Continuous)
                   - 0.5 * vol*vol;

            retVal[1] = varianceDistribution(x0[1], dw[1], dt);
            dy = (mu - rho_/sigma_*kappa_
                          *(theta_-vol*vol)) * dt + vol*sqrhov*dw[0]*sdt;

            retVal[0] = x0[0]*std::exp(dy + rho_/sigma_*(retVal[1]-x0[1]));
            break;
          case QuadraticExponential:
          case QuadraticExponentialMartingale:
          {
            // for details of the quadratic exponential discretization scheme
            // see Leif Andersen,
            // Efficient Simulation of the Heston Stochastic Volatility Model
            const Real ex = std::exp(-kappa_*dt);

            const Real m  =  theta_+(x0[1]-theta_)*ex;
            const Real s2 =  x0[1]*sigma_*sigma_*ex/kappa_*(1-ex)
                           + theta_*sigma_*sigma_/(2*kappa_)*(1-ex)*(1-ex);
            const Real psi = s2/(m*m);

            const Real g1 =  0.5;
            const Real g2 =  0.5;
                  Real k0 = -rho_*kappa_*theta_*dt/sigma_;
            const Real k1 =  g1*dt*(kappa_*rho_/sigma_-0.5)-rho_/sigma_;
            const Real k2 =  g2*dt*(kappa_*rho_/sigma_-0.5)+rho_/sigma_;
            const Real k3 =  g1*dt*(1-rho_*rho_);
            const Real k4 =  g2*dt*(1-rho_*rho_);
            const Real A  =  k2+0.5*k4;

            if (psi < 1.5) {
                const Real b2 = 2/psi-1+std::sqrt(2/psi*(2/psi-1));
                const Real b  = std::sqrt(b2);
                const Real a  = m/(1+b2);

                if (discretization_ == QuadraticExponentialMartingale) {
                    // martingale correction
                    QL_REQUIRE(A < 1/(2*a), "illegal value");
                    k0 = -A*b2*a/(1-2*A*a)+0.5*std::log(1-2*A*a)
                         -(k1+0.5*k3)*x0[1];
                }
                retVal[1] = a*(b+dw[1])*(b+dw[1]);
            }
            else {
                const Real p = (psi-1)/(psi+1);
                const Real beta = (1-p)/m;

                const Real u = CumulativeNormalDistribution()(dw[1]);

                if (discretization_ == QuadraticExponentialMartingale) {
                    // martingale correction
                    QL_REQUIRE(A < beta, "illegal value");
                    k0 = -std::log(p+beta*(1-p)/(beta-A))-(k1+0.5*k3)*x0[1];
                }
                retVal[1] = ((u <= p) ? 0.0 : std::log((1-p)/(1-u))/beta);
            }

            mu =   riskFreeRate_->forwardRate(t0, t0+dt, Continuous)
                 - dividendYield_->forwardRate(t0, t0+dt, Continuous);

            retVal[0] = x0[0]*std::exp(mu*dt + k0 + k1*x0[1] + k2*retVal[1]
                                       +std::sqrt(k3*x0[1]+k4*retVal[1])*dw[0]);
          }
          break;
          case BroadieKayaExactSchemeLobatto:
          case BroadieKayaExactSchemeLaguerre:
          case BroadieKayaExactSchemeTrapezoidal:
          {
            const Real nu_0 = x0[1];
            const Real nu_t = varianceDistribution(nu_0, dw[1], dt);

            const Real x = std::min(1.0-QL_EPSILON,
                std::max(0.0, CumulativeNormalDistribution()(dw[2])));

            const Real vds = Brent().solve(
                [&](Real xi){ return cdf_nu_ds_minus_x(*this, xi, nu_0, nu_t, dt, discretization_, x); },
                1e-5, theta_*dt, 0.1*theta_*dt);

            const Real vdw
                = (nu_t - nu_0 - kappa_*theta_*dt + kappa_*vds)/sigma_;

            mu = ( riskFreeRate_->forwardRate(t0, t0+dt, Continuous)
                  -dividendYield_->forwardRate(t0, t0+dt, Continuous))*dt
                - 0.5*vds + rho_*vdw;

            const Volatility sig = std::sqrt((1-rho_*rho_)*vds);
            const Real s = x0[0]*std::exp(mu + sig*dw[0]);

            retVal[0] = s;
            retVal[1] = nu_t;
          }
          break;
          default:
            QL_FAIL("unknown discretization schema");
        }

        return retVal;
    }

    const Handle<Quote>& HestonProcess::s0() const {
        return s0_;
    }

    const Handle<YieldTermStructure>& HestonProcess::dividendYield() const {
        return dividendYield_;
    }

    const Handle<YieldTermStructure>& HestonProcess::riskFreeRate() const {
        return riskFreeRate_;
    }

    Time HestonProcess::time(const Date& d) const {
        return riskFreeRate_->dayCounter().yearFraction(
                                           riskFreeRate_->referenceDate(), d);
    }

    Real HestonProcess::varianceDistribution(Real v, Real dw, Time dt) const {
        const Real df  = 4*theta_*kappa_/(sigma_*sigma_);
        const Real ncp = 4*kappa_*std::exp(-kappa_*dt)
            /(sigma_*sigma_*(1-std::exp(-kappa_*dt)))*v;

        const Real p = std::min(1.0-QL_EPSILON,
            std::max(0.0, CumulativeNormalDistribution()(dw)));

        return sigma_*sigma_*(1-std::exp(-kappa_*dt))/(4*kappa_)
            *InverseNonCentralCumulativeChiSquareDistribution(df, ncp, 100)(p);
    }
}
