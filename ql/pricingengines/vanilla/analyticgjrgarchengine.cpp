/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Yee Man Chan

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

/*! \file gjrgarchmodel.hpp
  \brief analytical approximation pricing engine for a GJR-GARCH option
  based on Edgeworth expansion
*/

#include <ql/pricingengines/vanilla/analyticgjrgarchengine.hpp>
#include <ql/math/distributions/normaldistribution.hpp>
#include <ql/instruments/payoffs.hpp>
#include <cmath>

using std::exp;
using std::pow;

namespace QuantLib {


    AnalyticGJRGARCHEngine::AnalyticGJRGARCHEngine(
                              const ext::shared_ptr<GJRGARCHModel>& model)
    : GenericModelEngine<GJRGARCHModel,
                         VanillaOption::arguments,
                         VanillaOption::results>(model) {init_ = false;}

    void AnalyticGJRGARCHEngine::calculate() const {
        // this is a european option pricer
        QL_REQUIRE(arguments_.exercise->type() == Exercise::European,
                   "not an European option");

        // plain vanilla
        ext::shared_ptr<StrikedTypePayoff> payoff =
            ext::dynamic_pointer_cast<StrikedTypePayoff>(arguments_.payoff);
        QL_REQUIRE(payoff, "non-striked payoff given");

        const ext::shared_ptr<GJRGARCHProcess>& process = model_->process();

        const Rate riskFreeDiscount = process->riskFreeRate()->discount(
                                            arguments_.exercise->lastDate());
        const Rate dividendDiscount = process->dividendYield()->discount(
                                            arguments_.exercise->lastDate());
        const Real spotPrice = process->s0()->value();
        QL_REQUIRE(spotPrice > 0.0, "negative or null underlying given");
        const Real strikePrice = payoff->strike();
        const Real term = process->time(arguments_.exercise->lastDate());
        Size T = Size(std::lround(process->daysPerYear()*term));
        Real r = -std::log(riskFreeDiscount/dividendDiscount)/(process->daysPerYear()*term);
        Real h1 = process->v0();
        Real b0 = process->omega();
        Real b2 = process->alpha();
        Real b1 = process->beta();
        Real b3 = process->gamma();
        Real la = process->lambda();
        Real N = CumulativeNormalDistribution()(la);
        Real n = std::exp(-la*la/2)/(M_SQRTPI*M_SQRT2);
        const Real s = spotPrice;
        const Real x = strikePrice;
        Real m1, m2, m3, v1, v2, v3, z1, z2, x1;
        Real ex, ex2, ex3, ex4;
        Real sEh = 0.0, sEh2 = 0.0, sEhh = 0.0, sEh1_2eh = 0.0;
        Real sEhhh = 0.0, sEh2h = 0.0, sEhh2 = 0.0, sEh3 = 0.0;
        Real sEh1_2eh2 = 0.0, sEh3_2eh = 0.0, sEh1_2ehh = 0.0, sEhh1_2eh = 0.0;
        Real sEhe2h = 0.0, sEh1_2eh1_2eh = 0.0;
        Real sEh3_2e3h = 0.0;
        Real SD1, SD2, SD3;
        Real ST1, ST2, ST3, ST4;
        Real SQ2, SQ4, SQ5;
        Size i, j, k;
        Real stdev, sigma, k3, k4;
        Real d, del, d_, C, A3, A4, Capp;
        bool constants_match = false;
        
        if (!init_ || b1 != b1_ || b2 != b2_ || b3 != b3_ || la != la_) {
            // compute the useful coefficients
            m1 = b1 + (b2+b3*N)*(1+la*la) + b3*la*n; // ok
            m2 = b1*b1 + b2*b2*(pow(la,4)+6*la*la+3)
                + (b3*b3+2*b2*b3)*( pow(la,4)*N
                                   +pow(la,3)*n+6*la*la*N+5*la*n+3*N)
                + 2*b1*b2*(1+la*la) + 2*b3*b1*(la*la*N+la*n+N); // ok
            m3 = pow(b1,3)
                + (3*b3*b3*b1+6*b1*b2*b3)*(pow(la,3)*n+5*la*n+3*N
                                           +pow(la,4)*N+6*la*la*N)
                + pow(b2,3)*(15+pow(la,6)+15*pow(la,4)+45*la*la)
                + (pow(b3,3)+3*b2*b2*b3+3*b3*b3*b2)
                *(pow(la,5)*n+14*pow(la,3)*n+33*la*n+15*N
                  +15*pow(la,4)*N+45*la*la*N+pow(la,6)*N)
                + 3*b1*b1*b2*(1+la*la) + 3*b1*b1*b3*(la*n+N+la*la*N)
                + 3*b1*b2*b2*(3+pow(la,4)+6*la*la); // ok
            v1 = -2*b2*la - 2*b3*(n+la*N); // ok
            v2 = -4*b2*b2*(3*la+pow(la,3))
                - (4*b3*b3+8*b2*b3)*(la*la*n+2*n+pow(la,3)*N+3*la*N)
                - 4*b1*b2*la - 4*b3*b1*(n+la*N); // ok
            v3 = -12*b3*b1*(b3+2*b2)*(la*la*n+2*n+pow(la,3)*N+3*la*N)
                - 6*pow(b2,3)*la*(15+pow(la,4)+10*la*la)
                - 6*b3*(b3*b3+3*b2*b2+3*b3*b2)
                *(9*la*la*n+8*n+15*la*N+pow(la,4)*n+pow(la,5)*N
                  +10*pow(la,3)*N)
                - 6*b1*b1*b2*la - 6*b3*b1*b1*(n+la*N)
                - 12*b2*b2*b1*(3*la+std::pow(la,3)); // ok
            z1 = b1 + b2*(3+la*la) + b3*(la*n+3*N+la*la*N); // ok
            z2 = b1*b1 + b2*b2*(15+pow(la,4)+18*la*la)
                + (b3*b3+2*b2*b3)*(pow(la,3)*n+17*la*n+15*N
                                   +pow(la,4)*N+18*la*la*N)
                + 2*b1*b2*(3+la*la) + 2*b3*b1*(la*n+3*N+la*la*N); // ok
            x1 = -6*b2*la - 2*b3*(4*n+3*la*N); // ok
            b1_ = b1; b2_ = b2; b3_ = b3; la_ = la;
            m1_ = m1; m2_ = m2; m3_ = m3; 
            v1_ = v1; v2_ = v2; v3_ = v3; z1_ = z1; z2_ = z2; x1_ = x1;
        } else {
            // these assignments are never used ?
            // b1 = b1_; b2 = b2_; b3 = b3_; la = la_;
            // m1 = m1_; m2 = m2_; m3 = m3_;
            // v1 = v1_; v2 = v2_; v3 = v3_; z1 = z1_; z2 = z2_; x1 = x1_;
            constants_match = true;
        }
        
        // compute the first four moments
        if (!init_ || !constants_match || b0 != b0_ || h1 != h1_ || T != T_) {
            // these assignments are never used ?
            //b1 = b1_; b2 = b2_; b3 = b3_; la = la_;
            m1 = m1_; m2 = m2_; m3 = m3_; 
            v1 = v1_; v2 = v2_; /*v3 = v3_;*/ z1 = z1_; /*z2 = z2_;*/ x1 = x1_;

            std::unique_ptr<Real[]> m1ai(new Real[T]);
            std::unique_ptr<Real[]> m2ai(new Real[T]);
            std::unique_ptr<Real[]> m3ai(new Real[T]);
            m1ai[0] = m2ai[0] = m3ai[0] = 1.0;
            for (i=1; i < T; ++i) {
                m1ai[i] = m1ai[i-1]*m1;
                m2ai[i] = m2ai[i-1]*m2;
                m3ai[i] = m3ai[i-1]*m3;
            }

            for (i = 0; i < T; ++i) {
                Real m1i = m1ai[i];
                Real m2i = m2ai[i];
                Real m3i = m3ai[i];

                Real m1im2i = m1i-m2i, m1im3i = m1i-m3i, m2im3i = m2i-m3i;
                Real Eh = b0*(1-m1i)/(1-m1) + m1i*h1; // ko
                Real Eh2 = b0*b0*((1+m1)*(1-m2i)/(1-m2)
                                  - 2*m1*m1im2i/(m1-m2))/(1-m1)
                    + 2*b0*m1*m1im2i*h1/(m1-m2)
                    + m2i*h1*h1; // ko
                Real Eh3 = pow(b0,3)*(
                    (1-m3i)/(1-m3)
                    + 3*m2*((1-m3i)/(1-m3)-m2im3i/(m2-m3))/(1-m2) 
                    + 3*m1*((1-m3i)/(1-m3)-m1im3i/(m1-m3))/(1-m1) 
                    + 6*m1*m2*(
                               ((1-m3i)/(1-m3)-m2im3i/(m2-m3))/(1-m2)
                               + (m2im3i/(m2-m3)-m1im3i/(m1-m3))/(m1-m2)
                               )/(1-m1))
                    + 3*b0*b0*m1*h1*(m1im3i/(m1-m3)
                                +2*m2*(m1im3i/(m1-m3)-m2im3i/(m2-m3))/(m1-m2))
                    + 3*b0*m2*h1*h1*m2im3i/(m2-m3) 
                    + m3i*h1*h1*h1; // ko
                Real Eh3_2 = .375*std::pow(Eh,-0.5)*Eh2+.625*std::pow(Eh,1.5);
                Real Eh5_2 = 1.875*std::pow(Eh,0.5)*Eh2-.875*std::pow(Eh,2.5);
                sEh += Eh;
                sEh2 += Eh2;
                sEh3 += Eh3;
                for (j = 0; j < T-i-1; ++j) {
                    Real Ehh = b0*Eh*(1-m1ai[j+1])/(1-m1)+ Eh2*m1ai[j+1]; // ko
                    Real Ehh2 = b0*b0*Eh*((1+m1)*(1-m2ai[j+1])/(1-m2) 
                                  - 2*m1*(m1ai[j+1]
                                          -m2ai[j+1])/(m1-m2))/(1-m1)
                        + 2*b0*m1*Eh2*(m1ai[j+1]-m2ai[j+1])/(m1-m2)
                        + m2ai[j+1]*Eh3; // ko
                    Real Eh2h = b0*Eh2*(1-m1ai[j+1])/(1-m1) 
                        + m1ai[j+1]*Eh3; // ok
                    Real Eh1_2eh = v1*m1ai[j]*Eh3_2; // ko
                    Real Eh1_2eh2 = 2*b0*v1*(m1ai[j+1]
                                             -m2ai[j+1])*Eh3_2/(m1-m2) 
                        + v2*m2ai[j]*Eh5_2; // ko
                    Real Ehij = b0*(1-m1ai[i+j+1])/(1-m1) 
                        + m1ai[i+j+1]*h1; // ko
                    Real Ehh3_2 = 0.375*Ehh2/std::sqrt(Ehij) 
                        + 0.75*std::sqrt(Ehij)*Ehh 
                        - 0.125*std::pow(Ehij,1.5)*Eh; // ko
                    Real Eh3_2eh = v1*m1ai[j]*Eh5_2; // ko
                    Real Eh3_2e3h = x1*m1ai[j]*Eh5_2; // ok
                    Real Eh1_2eh3_2 = 0.375*Eh1_2eh2/std::sqrt(Ehij) 
                        + 0.75*std::sqrt(Ehij)*Eh1_2eh; // ko
                    sEhh += Ehh;
                    sEh1_2eh += Eh1_2eh;
                    sEhh2 += Ehh2; 
                    sEh2h += Eh2h;
                    sEh1_2eh2 += Eh1_2eh2;
                    sEh3_2eh += Eh3_2eh;
                    sEhe2h += b0*Eh*(1-m1ai[j+1])/(1-m1) 
                        + z1*m1ai[j]*Eh2; // ko
                    sEh3_2e3h += Eh3_2e3h; // ok
                    for (k = 0; k < T-i-j-2; ++k) {
                        Real Ehhh = b0*Ehh*(1-m1ai[k+1])/(1-m1) 
                            + m1ai[k+1]*Ehh2; //ko
                        Real Eh1_2ehh = b0*Eh1_2eh*(1-m1ai[k+1])/(1-m1) 
                            + m1ai[k+1]*Eh1_2eh2; // ko
                        sEhhh += Ehhh;
                        sEh1_2ehh += Eh1_2ehh;
                        sEhh1_2eh += v1*m1ai[k]*Ehh3_2; // ko
                        sEh1_2eh1_2eh += v1*m1ai[k]*Eh1_2eh3_2; // ko
                    }
                }
            }
            
            ex = T*r - 0.5*sEh; 
            SD1 = 2*sEhh + sEh2;
            SD2 = sEh;
            SD3 = sEh1_2eh;
            ex2 = T*T*r*r - T*r*sEh + 0.25*SD1 + SD2 - SD3;
            ST1 = 6*sEhhh + (3*sEhh2 + (3*sEh2h + sEh3));
            ST2 = 3*sEh1_2eh;
            ST3 = 2*sEhh1_2eh + (2*sEh1_2ehh + (2*sEh3_2eh + sEh1_2eh2));
            ST4 = sEhe2h + (sEhh + (sEh2 + 2*sEh1_2eh1_2eh)); 
            ex3 = pow(T*r,3) - 1.5*T*T*r*r*sEh 
                + 3*T*r*(SD1/4+SD2-SD3) + (ST2-ST1/8+3*ST3/4-3*ST4/2);
            SQ2 = 6*sEhe2h + (12*sEh1_2eh1_2eh + 3*sEh2);     
            SQ4 = 2*sEhhh + 2*sEhh2;
            SQ5 = 3*sEhh1_2eh + 3*sEh1_2ehh + 3*sEh3_2eh 
                + 3*sEh1_2eh2 + sEh3_2e3h;
            ex4 = pow(T*r,4) - 2*pow(T*r,3)*sEh 
                + 6*T*T*r*r*(SD1/4+SD2-SD3) + T*r*(4*ST2-ST1/2+3*ST3-6*ST4) 
                + (SQ2+3*SQ4/2-2*SQ5);
            
            // compute variance, skewness, kurtosis
            sigma = ex2 - ex*ex;
            // 3rd central moment mu3
            k3 = ex3 - 3*sigma*ex - ex*ex*ex;
            // 4th central moment mu4
            k4 = ex4 + 6*ex*ex*ex2 - 3*ex*ex*ex*ex - 4*ex*ex3;
            k3 /= std::pow(sigma,1.5); // 3rd standardized moment, ie skewness 
            k4 /= pow(sigma,2); // 4th standardized moment, ie kurtosis
            ex_ = ex; sigma_ = sigma; 
            k3_ = k3; k4_ = k4; r_ = r; T_ = T; b0_ = b0; h1_ = h1;
        } else {
            ex = ex_; sigma = sigma_; 
            k3 = k3_; k4 = k4_; r = r_; T = T_; /*b0 = b0_; h1 = h1_;*/ // never used ?
        }
        
        // compute call option price
        stdev = std::sqrt(sigma);
        del = (ex - r*T + sigma/2)/stdev;
        d = (std::log(s/x) + (r*T+sigma/2))/stdev;
        d_ = d+del;
        C = s*std::exp(del*stdev)*CumulativeNormalDistribution()(d_) 
            - x*std::exp(-r*T)*CumulativeNormalDistribution()(d_-stdev);
        A3 = s*std::exp(del*stdev)*stdev*((2*stdev-d_)
                   *std::exp(-d_*d_/2)/std::sqrt(2*M_PI)
                   +sigma*CumulativeNormalDistribution()(d_))/6;
        A4 = s*std::exp(del*stdev)*stdev*(
            (d_*d_-1-3*stdev*(d_-stdev))*exp(-d_*d_/2)/std::sqrt(2*M_PI)
            -sigma*stdev*CumulativeNormalDistribution()(d_))/24;
        Capp = C + k3*A3 + (k4-3)*A4;
        init_ = true;

        switch (payoff->optionType()) {
          case Option::Call:
            results_.value = Capp;
            break;
          case Option::Put:
            results_.value = Capp+strikePrice*riskFreeDiscount/dividendDiscount
                -spotPrice;
            break;
          default:
            QL_FAIL("unknown option type");
        }
    }   
}
