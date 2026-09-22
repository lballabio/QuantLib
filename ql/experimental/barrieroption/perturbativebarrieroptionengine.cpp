/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Lorella Fatone
 Copyright (C) 2008 Maria Cristina Recchioni
 Copyright (C) 2008 Francesco Zirilli
 Copyright (C) 2008 StatPro Italia srl

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

#include <ql/errors.hpp>
#include <ql/exercise.hpp>
#include <ql/experimental/barrieroption/perturbativebarrieroptionengine.hpp>
#include <ql/types.hpp>
#include <functional>
#include <algorithm>
#include <cmath>
#include <utility>

using namespace std;

namespace {

    inline QuantLib::Real SIGN(const QuantLib::Real& a, const QuantLib::Real& b) 
    {
        if (b > 0.0) 
            return std::fabs(a);
        else
            return -std::fabs(a);
    }

}

namespace QuantLib {

    namespace {

    constexpr double PI = 3.14159265358979324;
    constexpr double halfPi = PI / 2.0;
    constexpr double twoPi = 2.0 * PI;
    constexpr double sqrtTwo = 1.4142135623730950488;
    constexpr double sqrtPi = 1.7724538509055160273;
    constexpr double sqrtTwoPi = sqrtTwo * sqrtPi;

    // TODO: review these local distribution helpers against the existing
    // QuantLib implementations before consolidating them.
    Real ND2(Real a, Real b, Real rho);
    // standard normal cumulative distribution function
    Real PHID(Real Z);

    // Functions used to compute the first order approximation
    Real ff(Real p,Real tt,Real a, Real b, Real gm);
    Real v(Real p, Real tt,Real a,Real b,Real gm);
    Real llold(Real p,Real tt, Real a, Real b,
                 Real c, Real gm);

    // Functions used to compute the second order approximation
    Real derivn3(Real limit[4], Real sigmarho[4], Real sigma[4], int idx);
    Real ddvv(Real s, Real p, Real tt, Real a,
                Real b, Real gm);
    Real ddff(Real s, Real p,Real tt,Real a,Real b,Real gm);
    Real dll(Real s,Real p,Real tt,Real a,Real b,
               Real c,Real gm);
    Real ddll(Real s,Real p,Real tt, Real ax, Real bx,
                Real c, Real gm);
    Real dvv(Real s,Real p,Real tt,Real a,Real b,Real gm);
    Real dff(Real s, Real p,Real tt,Real a,Real b,Real gm);
    Real tvtl(int jj, const Real limit[4], const Real sigmarho[4], Real epsi);

    template <class Integr, class IntegralAlpha, class IntegralVariance,
              class Alpha, class SigmaQ>
    Real BarrierUPD(Real kprice,
                    Real stock,
                    Real hbarr,
                    Real taumin,
                    Real taumax,
                    int iord,
                    int igm,
                    Size npoint,
                    Size npoint2,
                    const Integr& integr,
                    const IntegralAlpha& integalpha,
                    const IntegralVariance& integs,
                    const Alpha& alpha,
                    const SigmaQ& sigmaq) {
        Real v0=0.0, v1=0.0, v1p=0.0, v2p=0.0, v2pp=0.0, gm=0.0;
        Size i=0,j=0;
        Real tmp=0.0, e1=0.0, e2=0.0, e3=0.0, e4=0.0;
        Real xstar=0.0, s0=0.0;
        Real sigmat=0.0, disc=0.0, d1=0.0,d2=0.0,d3=0.0,d4=0.0;
        Real et=0.0,tt=0.0, dt=0.0,p=0.0;
        Real dsqpi;
        Real caux=0.0,ccaux=0.0;
        Real auxnew=0.0;
        Real x=0.0,b=0.0,c=0.0;

        if(igm==0) {
            gm=0.0;
        } else if(igm==1) {
            gm=integalpha(taumin,taumax)/(0.5*integs(taumin,taumax));
        } else {
            gm=0.0;
        }

        /*
          !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
          !!                                 !!
          !! xstar=min(0,log(kprice/hbarr))  !!
          !!                                 !!
          !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        */

        xstar=log(kprice/hbarr);

        if(xstar>0.0) xstar=0.0;
        sigmat=integs(taumin,taumax);
        disc=-integr(taumin,taumax);
        const Real oneMinusGamma = 1.0-gm;
        const Real onePlusGamma = 1.0+gm;
        const Real barrierFactor = hbarr*exp(0.5*onePlusGamma*xstar)
                     - kprice*exp(-0.5*oneMinusGamma*xstar);

        /*
          !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
          !!    Change of variable            !!
          !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        */
        s0=stock/hbarr;

        /*
          !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
          !!                                       !!
          !! Compute the zero-th order term P_0    !!
          !!                                       !!
          !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        */
        x=log(s0);
        const Real invSqrtSigmat = 1.0 / std::sqrt(sigmat);
        const Real halfSigmat = 0.5 * sigmat;
        const Real minusGammaNumerator = xstar + oneMinusGamma * halfSigmat;
        const Real plusGammaNumerator = xstar - onePlusGamma * halfSigmat;
        const Real scaledLogS0 = x * invSqrtSigmat;
        const Real scaledMinusGammaNumerator = minusGammaNumerator * invSqrtSigmat;
        const Real scaledPlusGammaNumerator = plusGammaNumerator * invSqrtSigmat;
        d1=scaledMinusGammaNumerator-scaledLogS0;
        d2=scaledMinusGammaNumerator+scaledLogS0;
        d3=scaledPlusGammaNumerator-scaledLogS0;
        d4=scaledPlusGammaNumerator+scaledLogS0;

        e1=PHID(d1);
        e2=PHID(d2);
        e3=PHID(d3);
        e4=PHID(d4);

        v0=kprice*e1-kprice*exp(oneMinusGamma*x)*e2;
        v0=v0+exp(gm*halfSigmat)*(-hbarr*s0*e3+hbarr*exp(-gm*x)*e4);
        v0=v0*exp(disc);

        if(iord==0) return v0;

        /*
          !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
          !!                                           !!
          !! Compute the first order term  P_1         !!
          !!                                           !!
          !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        */

        dt=(taumax-taumin)/double(npoint);

        tt=0.5*integs(taumin,taumax);

        et=exp(0.5*(1.0-gm)*x);

        dsqpi=sqrtPi;

        v1=0.0;
        for( i=1;i<=npoint;i++) {
            v1p=0.0;
            tmp=taumin+dt*double(2*i-1)*0.5;
            p=0.5*integs(tmp,taumax);
            const Real expGammaP = exp(gm*p);
            /*
              !!
              !! Function E(p,tt,a,b,gm)
              !!
            */
            ccaux=v(p,tt,x,xstar,gm)+v(p,tt,x,-xstar,gm)-v(p,tt,-x,xstar,gm)-v(p,tt,-x,-xstar,gm);
            auxnew=ccaux*(-kprice*exp(-xstar*0.5*(1.0-gm))+hbarr*exp(xstar*0.5*(1.0+gm)));
            v1p=v1p+auxnew;
            /*
              !!
              !! Function L(p,tt,a,b,c,gm)
              !!
            */
            b=gm-1.0;
            c=-xstar;
            ccaux=llold(p,tt,x,b,c,gm)-llold(p,tt,-x,b,c,gm);
            auxnew=kprice*(1.0-gm)*ccaux;
            v1p=v1p+auxnew;

            b=-(gm+1.0);
            c=xstar;
            ccaux=llold(p,tt,x,b,c,gm)-llold(p,tt,-x,b,c,gm);
            auxnew=-expGammaP*hbarr*ccaux;
            v1p=v1p+auxnew;

            b=(gm+1.0);
            c=-xstar;
            ccaux=llold(p,tt,x,b,c,gm)-llold(p,tt,-x,b,c,gm);
            auxnew=expGammaP*hbarr*gm*ccaux;
            v1p=v1p+auxnew;
            /*
              !!
              !! Function F(p,tt,a,b,c,gm)
              !!
            */
            b=gm-1.0;
            auxnew=-kprice*(1.0-gm)*(ff(p,tt,x,b,gm)-ff(p,tt,-x,b,gm));
            v1p=v1p+auxnew;

            b=gm+1.0;
            auxnew=-expGammaP*gm*hbarr*(ff(p,tt,x,b,gm)-ff(p,tt,-x,b,gm));
            v1p=v1p+auxnew;

            v1=v1+(alpha(tmp)-gm*0.5*sigmaq(tmp))*v1p;
        }

        v1=exp(disc)*et*v1*dt/(dsqpi*2.0);

        if(iord==1) return v0+v1;

        /*
          !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
          !!                                          !!
          !! Compute the second order term P_2        !!
          !!                                          !!
          !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        */

        Real v2,dtp, tmp1,s;
        v2=0.0;

        for(i=1;i<=npoint;i++) {
            v2p=0.0;
            tmp=taumin+dt*(double)(2*i-1)*0.5;
            p=0.5*integs(tmp,taumax);

            dtp=(taumax-tmp)/(double)(npoint2);
            const Real outerPerturbation = alpha(tmp)-gm*0.5*sigmaq(tmp);

            for(j=1;j<=npoint2; j++) {
                tmp1=tmp+dtp*(double)(2*j-1)*0.50;
                s=0.50*integs(tmp1,taumax);
                const Real expGammaS = exp(gm*s);
                const Real innerPerturbation = alpha(tmp1)-gm*0.5*sigmaq(tmp1);

                caux=dll(s,p,tt,-x,-1.0+gm,-xstar,gm)-dll(s,p,tt,x,-1.0+gm,-xstar,gm);
                v2pp=caux*kprice*(1.0-gm);

                caux=dll(s,p,tt,-x,-1.0-gm,xstar,gm)-dll(s,p,tt,x,-1.0-gm,xstar,gm);
                v2pp=v2pp-expGammaS*hbarr*caux;

                caux=dll(s,p,tt,-x,1.0+gm,-xstar,gm)-dll(s,p,tt,x,1.0+gm,-xstar,gm);
                v2pp=v2pp+expGammaS*gm*hbarr*caux;

                caux=+dvv(s,p,tt,-x,xstar,gm)-dvv(s,p,tt,x,xstar,gm);
                caux=caux+(dvv(s,p,tt,-x,-xstar,gm)-dvv(s,p,tt,x,-xstar,gm));
                v2pp=v2pp+barrierFactor*caux;

                caux=dff(s,p,tt,-x,-1.0+gm,gm)-dff(s,p,tt,x,-1.0+gm,gm);
                v2pp=v2pp-(1.0-gm)*kprice*caux;

                caux=dff(s,p,tt,-x,1.0+gm,gm)-dff(s,p,tt,x,1.0+gm,gm);
                v2pp=v2pp-expGammaS*gm*hbarr*caux;

                v2pp=v2pp*0.5*(1.0-gm);

                caux=-ddll(s,p,tt,-x,-1.0+gm,-xstar,gm)+ddll(s,p,tt,x,-1.0+gm,-xstar,gm);
                v2pp=v2pp+caux*kprice*(1.0-gm);

                caux=-ddll(s,p,tt,-x,-1.0-gm,xstar,gm)+ddll(s,p,tt,x,-1.0-gm,xstar,gm);
                v2pp=v2pp-expGammaS*hbarr*caux;

                caux=-ddll(s,p,tt,-x,1.0+gm,-xstar,gm)+ddll(s,p,tt,x,1.0+gm,-xstar,gm);
                v2pp=v2pp+expGammaS*gm*hbarr*caux;

                caux=-ddvv(s,p,tt,-x,xstar,gm)+ddvv(s,p,tt,x,xstar,gm);
                caux=caux+(-dvv(s,p,tt,-x,-xstar,gm)+dvv(s,p,tt,x,-xstar,gm));

                v2pp=v2pp+barrierFactor*caux;

                caux=-ddff(s,p,tt,-x,-1+gm,gm)+ddff(s,p,tt,x,-1+gm,gm);
                v2pp=v2pp-(1.0-gm)*kprice*caux;

                caux=-ddff(s,p,tt,-x,1.0+gm,gm)+ddff(s,p,tt,x,1.0+gm,gm);
                v2pp=v2pp-expGammaS*gm*hbarr*caux;

                v2p=v2p+innerPerturbation*v2pp;
            }

            v2=v2+v2p*outerPerturbation*dtp;
        }

        v2=exp(disc)*et*v2*dt;

        return v0+v1+v2;
    }


    Real PHID(Real Z){
        /*
         *     Normal distribution probabilities accurate to 1D-15.
         *     Z = number of standard deviations from the mean.
         *
         *     The software that computes the normal distribution
         *     probabilities has been developed by M.C. Recchioni
         *     based upon algorithm 5666 (Programmer Alan Miller)
         *     for the error function, taken from:
         *     Hart, J.F. et al, 'Computer Approximations', Wiley, 1968
         *
         */
        Real P0, P1, P2, P3, P4, P5, P6;
        Real Q0, Q1, Q2, Q3, Q4, Q5, Q6, Q7;
        Real P, EXPNTL, CUTOFF, ZABS;

        P0 = 220.2068679123761;
        P1 = 221.2135961699311;
        P2 = 112.0792914978709;
        P3 = 33.91286607838300;
        P4 = 6.373962203531650;
        P5 = 0.7003830644436881;
        P6 = 0.03526249659989109;

        Q0 = 440.4137358247522;
        Q1 = 793.8265125199484;
        Q2 = 637.3336333788311;
        Q3 = 296.5642487796737;
        Q4 = 86.78073220294608;
        Q5 = 16.064177579206950;
        Q6 = 1.7556671631826420;
        Q7 = 0.088388347648318440;
        CUTOFF = 7.071067811865475;

        ZABS = fabs(Z);
        /*
          |Z| > 37
        */
        if (ZABS > 37)
            P = 0;
        else
        {
            /*
              |Z| <= 37
            */
            EXPNTL =exp(-ZABS*ZABS/2);
            /*
              |Z| < CUTOFF = 10/SQRT(2)
            */
            if ( ZABS < CUTOFF )
                P = EXPNTL*((((((P6*ZABS + P5)*ZABS + P4)*ZABS + P3)*ZABS+ P2)*ZABS + P1)*ZABS + P0)/(((((((Q7*ZABS + Q6)*ZABS + Q5)*ZABS + Q4)*ZABS + Q3)*ZABS + Q2)*ZABS + Q1)*ZABS + Q0);
            /*
              |Z| >= CUTOFF.
            */
            else
                P = EXPNTL/(ZABS + 1/(ZABS + 2/(ZABS + 3/(ZABS + 4/(ZABS + 0.65)))))/sqrtTwoPi;

        }
        if ( Z > 0 ) P = 1 - P;

        return(P);
    }


    /*
      !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
      !!                                                              !!
      !! Functions needed to compute the  first order term  P_1       !!
      !!                                                              !!
      !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    */

    /*
      !!
      !! Function F(p,tt,a,b,gm)
      !!
    */
    Real ff(Real p,Real tt,Real a, Real b, Real gm) {
        Real phid;
        Real aa, caux;

        aa=-(b*p-b*tt+a)/std::sqrt(2.0*(tt-p));

        caux=2.0*sqrtPi*PHID(aa);
        aa=b*b-(1.0-gm)*(1.0-gm);
        aa=aa/4.0;
        phid=exp(-0.5*a*b)*exp(aa*(tt-p))*caux;

        return phid;
    }

    /*
      !!
      !! Function  E(p,tt,a,b,gm)
      !!
    */
    Real v(Real p, Real tt,Real a,Real b,Real gm)
    {
        Real result;
        Real aa,caux;

        aa=-(p*(a-b)+b*tt)/std::sqrt(2.0*p*tt*(tt-p));
        caux=PHID(aa);

        aa=exp((a-b)*(a-b)/(4.0*tt))*exp((1.0-gm)*(1.0-gm)*tt/4.0)*std::sqrt(tt);
        result=caux/aa;

        return(result);
    }

    /*
      !!
      !! Fuction L(p,tt,a,b,c,gm)
      !!
    */
    Real llold(Real p,Real tt, Real a, Real b,Real c, Real gm){
        Real bvnd;
        Real xx,yy,rho,caux;
        Real aa;

        xx=(-a+b*(tt-p))/std::sqrt(2.0*(tt-p));
        yy=(-a+b*tt+c)/std::sqrt(2.0*tt);
        rho=std::sqrt((tt-p)/tt);
        aa=b*b-(1.0-gm)*(1.0-gm);
        aa=aa/4.0;
        caux=ND2(-xx,-yy,rho);

        bvnd=2.0*sqrtPi*exp(-a*b*0.5)*exp(aa*(tt-p))*caux;
        return(bvnd);
    }

    /*
      !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
      !!                                                              !!
      !! Functions needed to compute the  second order term  P_2      !!
      !!                                                              !!
      !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    */

    /*
      !!
      !! Function  D_E(s,p,tt,a,b,gm)
      !!
    */
    Real dvv(Real s,Real p,Real tt,Real a,Real b,Real gm)
    {
        Real result;
        Real aa,caux,caux1,caux2;
        Real xx,yy,rho;

        aa=(a*p+b*(tt-p))/std::sqrt(2.0*p*tt*(tt-p));
        caux=PHID(aa);

        aa=exp((a-b)*(a-b)/(4.0*tt))*exp((1.0-gm)*(1.0-gm)*tt/4.0)*std::sqrt(tt);
        caux=-caux/aa;

        xx=(a*p+b*(tt-p))/std::sqrt(2.0*tt*p*(tt-p));
        yy=(a*s+b*(tt-s))/std::sqrt(2.0*tt*s*(tt-s));
        rho=std::sqrt((s*(tt-p))/(p*(tt-s)));
        caux1=ND2(-xx,-yy,rho);
        caux1=caux1/aa;


        aa=exp((a+b)*(a+b)/(4.0*tt))*exp((1.0-gm)*(1.0-gm)*tt/4.0)*std::sqrt(tt);

        xx=(a*p-b*(tt-p))/std::sqrt(2.0*tt*p*(tt-p));
        yy=(a*s-b*(tt-s))/std::sqrt(2.0*tt*s*(tt-s));
        rho=std::sqrt((s*(tt-p))/(p*(tt-s)));
        caux2=ND2(-xx,-yy,rho);
        caux2=caux2/aa;
        result=(caux+caux1+caux2)/(2.0*sqrtPi);
        return(result);
    }

    /*
      !!
      !! Function D_F(s,p,tt,a,b,gm)
      !!
    */
    Real dff(Real s, Real p,Real tt,Real a,Real b,Real gm)
    {
        Real result;
        Real aa,caux,caux1,caux2;
        Real xx,yy,rho;

        xx=(a-b*(tt-p))/std::sqrt(2.0*(tt-p));
        caux=-PHID(xx)*exp(-0.5*a*b);

        xx=(a+b*(tt-p))/std::sqrt(2.0*(tt-p));
        yy=(a+b*(tt-s))/std::sqrt(2.0*(tt-s));
        rho=std::sqrt((tt-p)/(tt-s));
        caux1=ND2(-xx,-yy,rho);
        caux1=exp(0.5*a*b)*caux1;

        xx=(a-b*(tt-p))/std::sqrt(2.0*(tt-p));
        yy=(a-b*(tt-s))/std::sqrt(2.0*(tt-s));
        rho=std::sqrt((tt-p)/(tt-s));
        caux2=ND2(-xx,-yy,rho);
        caux2=exp(-0.5*a*b)*caux2;

        aa=exp((b*b-(1.0-gm)*(1.0-gm))*(tt-s)/4.0);

        result=(caux+caux1+caux2)*aa;
        return(result);
    }


    /*
      !!
      !! Function D_L(s,p,a,b,c,gm)
      !!
    */
    Real dll(Real s,Real p,Real tt,Real a,Real b,Real c,Real gm)
    {
        Real result;
        Real aa,caux,caux1;
        Real sigmarho[4],limit[4],epsi;

        epsi=1.e-12;
        limit[1]=(a+b*(tt-p))/std::sqrt(2.0*(tt-p));
        limit[2]=(a+b*(tt-s))/std::sqrt(2.0*(tt-s));
        limit[3]=(a+b*tt+c)/std::sqrt(2.0*tt);
        sigmarho[1]=std::sqrt((tt-p)/(tt-s));
        sigmarho[2]=std::sqrt((tt-p)/tt);
        sigmarho[3]=std::sqrt((tt-s)/tt);

        caux=exp(0.5*a*b)*tvtl(0,limit,sigmarho,epsi);

        limit[1]=(a-b*(tt-p))/std::sqrt(2.0*(tt-p));
        limit[2]=(-a+b*(tt-s))/std::sqrt(2.0*(tt-s));
        limit[3]=(-a+b*tt+c)/std::sqrt(2.0*tt);
        sigmarho[1]=-std::sqrt((tt-p)/(tt-s));
        sigmarho[2]=-std::sqrt((tt-p)/tt);
        sigmarho[3]=std::sqrt((tt-s)/tt);

        caux1=-exp(-0.5*a*b)*tvtl(0,limit,sigmarho,epsi);

        aa=exp((b*b-(1.0-gm)*(1.0-gm))*(tt-s)/4.0);


        result=(caux+caux1)*aa;

        return(result);
    }

    /*
      !!
      !! Derivative with respect to a of the function D_F(s,p,tt,a,b,gm)
      !!
    */
    Real ddff(Real s, Real p,Real tt,Real a,Real b,Real gm)
    {
        Real aa,caux,caux1,caux2,caux3,caux4;
        Real xx,yy,rho;
        Real result;

        xx=(a-b*(tt-p))/std::sqrt(2.0*(tt-p));
        caux=PHID(xx)*exp(-0.5*a*b);

        xx=(a+b*(tt-p))/std::sqrt(2.0*(tt-p));
        yy=(a+b*(tt-s))/std::sqrt(2.0*(tt-s));
        rho=std::sqrt((tt-p)/(tt-s));
        caux1=ND2(-xx,-yy,rho);
        caux1=exp(0.5*a*b)*caux1;

        xx=(a-b*(tt-p))/std::sqrt(2.0*(tt-p));
        yy=(a-b*(tt-s))/std::sqrt(2.0*(tt-s));
        rho=std::sqrt((tt-p)/(tt-s));
        caux2=ND2(-xx,-yy,rho);
        caux2=-exp(-0.5*a*b)*caux2;

        caux=0.5*b*(caux+caux1+caux2);

        xx=(a+b*(tt-p))/std::sqrt(2.0*(tt-p));
        yy=b*std::sqrt(p-s)/sqrtTwo;
        caux1=exp(-0.5*xx*xx)*exp(0.5*a*b)*PHID(yy)/(2.0*std::sqrt(PI*(tt-p)));


        xx=(a+b*(tt-s))/std::sqrt(2.0*(tt-s));
        yy=a*std::sqrt(p-s)/std::sqrt(2.0*(tt-p)*(tt-s));
        caux2=exp(-0.5*xx*xx)*exp(0.5*a*b)*PHID(yy)/(2.0*std::sqrt(PI*(tt-s)));

        xx=(a-b*(tt-p))/std::sqrt(2.0*(tt-p));
        yy=b*std::sqrt(p-s)/sqrtTwo;
        caux3=-exp(-0.5*xx*xx)*exp(-0.5*a*b)*PHID(yy)/(2.0*std::sqrt(PI*(tt-p)));


        xx=(a-b*(tt-s))/std::sqrt(2.0*(tt-s));
        yy=a*std::sqrt(p-s)/std::sqrt(2.0*(tt-p)*(tt-s));
        caux4=exp(-0.5*xx*xx)*exp(-0.5*a*b)*PHID(yy)/(2.0*std::sqrt(PI*(tt-s)));



        aa=exp((b*b-(1.0-gm)*(1.0-gm))*(tt-p)/4.0);


        result=(caux+caux1+caux2+caux3+caux4)*aa;
        return(result);
    }

    /*
      !!
      !! Derivative with respect to a of the function D_L(s,p,tt,a,b,c,gm)
      !!
    */
    Real ddll(Real s,Real p,Real tt, Real ax, Real bx,Real c, Real gm)
    {
        Real result;
        Real aa,caux,caux1;
        Real sigmarho[4],sigma[4],limit[4];
        int idx;
        Real epsi;

        epsi=1.e-12;
        limit[1]=(ax+bx*(tt-p))/std::sqrt(2.0*(tt-p));
        limit[2]=(ax+bx*(tt-s))/std::sqrt(2.0*(tt-s));
        limit[3]=(ax+bx*tt+c)/std::sqrt(2.0*tt);
        sigmarho[1]=std::sqrt((tt-p)/(tt-s));
        sigmarho[2]=std::sqrt((tt-p)/tt);
        sigmarho[3]=std::sqrt((tt-s)/tt);
        sigma[1]=std::sqrt(1.0-sigmarho[1]*sigmarho[1]);
        sigma[2]=std::sqrt(1.0-sigmarho[2]*sigmarho[2]);
        sigma[3]=std::sqrt(1.0-sigmarho[3]*sigmarho[3]);

        caux=0.5*bx*tvtl(0,limit,sigmarho,epsi);


        idx=1;
        caux=caux+derivn3(limit,sigmarho,sigma,idx)/std::sqrt(2.0*(tt-p));

        idx=2;
        caux=caux+derivn3(limit,sigmarho,sigma,idx)/std::sqrt(2.0*(tt-s));

        idx=3;
        caux=caux+derivn3(limit,sigmarho,sigma,idx)/std::sqrt(2.0*tt);

        caux=exp(0.5*ax*bx)*caux;

        limit[1]=(ax-bx*(tt-p))/std::sqrt(2.0*(tt-p));
        limit[2]=(-ax+bx*(tt-s))/std::sqrt(2.0*(tt-s));
        limit[3]=(-ax+bx*tt+c)/std::sqrt(2.0*tt);
        sigmarho[1]=-std::sqrt((tt-p)/(tt-s));
        sigmarho[2]=-std::sqrt((tt-p)/tt);
        sigmarho[3]=std::sqrt((tt-s)/tt);
        sigma[1]=std::sqrt(1.0-sigmarho[1]*sigmarho[1]);
        sigma[2]=std::sqrt(1.0-sigmarho[2]*sigmarho[2]);
        sigma[3]=std::sqrt(1.0-sigmarho[3]*sigmarho[3]);

        caux1=0.5*bx*tvtl(0,limit,sigmarho,epsi);

        idx=1;
        caux1=caux1-derivn3(limit,sigmarho,sigma,idx)/std::sqrt(2.0*(tt-p));

        idx=2;
        caux1=caux1+derivn3(limit,sigmarho,sigma,idx)/std::sqrt(2.0*(tt-s));


        idx=3;
        caux1=caux1+derivn3(limit,sigmarho,sigma,idx)/std::sqrt(2.0*tt);

        caux1=exp(-0.5*ax*bx)*caux1;


        aa=exp((bx*bx-(1.0-gm)*(1.0-gm))*(tt-s)/4.0);

        result=(caux+caux1)*aa;
        return(result);
    }

    /*
      !!
      !!   Derivative with respect to a of the function D_E(s,p,tt,a,b,gm)
      !!
    */
    Real ddvv(Real s, Real p, Real tt, Real a, Real b, Real gm)
    {
        Real result;
        Real aa,caux,caux1,caux2,caux6;
        Real caux3,caux4,caux5,aux;
        Real xx,yy,rho;

        aa=(a*p+b*(tt-p))/std::sqrt(2.0*p*tt*(tt-p));
        caux=PHID(aa);

        aa=exp(-(a-b)*(a-b)/(4.0*tt))/tt;

        caux=0.5*aa*caux*(a-b);

        xx=(a*p+b*(tt-p))/std::sqrt(2.0*tt*p*(tt-p));
        yy=(a*s+b*(tt-s))/std::sqrt(2.0*tt*s*(tt-s));
        rho=std::sqrt((s*(tt-p))/(p*(tt-s)));
        caux1=ND2(-xx,-yy,rho);
        caux1=-0.5*aa*caux1*(a-b);


        aa=exp(-(a+b)*(a+b)/(4.0*tt))/tt;

        xx=(a*p-b*(tt-p))/std::sqrt(2.0*tt*p*(tt-p));
        yy=(a*s-b*(tt-s))/std::sqrt(2.0*tt*s*(tt-s));
        rho=std::sqrt((s*(tt-p))/(p*(tt-s)));
        caux2=ND2(-xx,-yy,rho);
        caux2=-0.5*aa*caux2*(a+b);

        aa=-b*std::sqrt((p-s)/std::sqrt(2.0*p*s));
        aux=std::sqrt(p/(PI*tt*(tt-p)))*PHID(aa);

        xx=(a+b)*(a+b)/(4.0*tt);
        yy=(a*p-b*(tt-p))*(a*p-b*(tt-p))/(4.0*p*tt*(tt-p));
        caux3=aux*exp(-xx)*exp(-yy)/2.0;


        xx=(a-b)*(a-b)/(4.0*tt);
        yy=(a*p+b*(tt-p))*(a*p+b*(tt-p))/(4.0*p*tt*(tt-p));
        caux4=aux*exp(-xx)*exp(-yy)/2.0;

        aa=a*std::sqrt((p-s)/std::sqrt(2.0*(tt-p)*(tt-s)));
        aux=std::sqrt(s/(PI*tt*(tt-s)))*PHID(aa);

        xx=(a+b)*(a+b)/(4.0*tt);
        yy=(a*s-b*(tt-s))*(a*s-b*(tt-s))/(4.0*s*tt*(tt-s));
        caux5=aux*exp(-xx)*exp(-yy)/2.0;

        xx=(a-b)*(a-b)/(4.0*tt);
        yy=(a*s+b*(tt-s))*(a*s+b*(tt-s))/(4.0*s*tt*(tt-s));
        caux6=aux*exp(-xx)*exp(-yy)/2.0;

        aux=exp((1.0-gm)*(1.0-gm)*tt/4.0)*std::sqrt(tt);

        result=(caux+caux1+caux2+caux3+caux4+caux5+caux6)/(aux*2.0*sqrtPi);
        return(result);
    }

    /*
      !!
      !! Derivn3 computes the derivatives of the trivariate cumulative normal
      !! distribution with respect to one of the integration limits
      !!
    */
    Real derivn3(Real limit[4], Real sigmarho[4], Real sigma[4], int idx)
    {
        Real aa;
        Real xx,yy,rho,sc;
        Real deriv;
        sc=sqrtTwoPi;

        if(idx==1)
            {
                aa=exp(-0.5*limit[1]*limit[1]);
                xx=(limit[3]-sigmarho[2]*limit[1])/sigma[2];
                yy=(limit[2]-sigmarho[1]*limit[1])/sigma[1];
                rho=(sigmarho[3]-sigmarho[1]*sigmarho[2])/(sigma[1]*sigma[2]);
                deriv=aa*ND2(-xx,-yy,rho)/sc;
            }
        else
            {
                if(idx==2)
                    {
                        aa=exp(-0.5*limit[2]*limit[2]);
                        xx=(limit[1]-sigmarho[1]*limit[2])/sigma[1];
                        yy=(limit[3]-sigmarho[3]*limit[2])/sigma[3];
                        rho=(sigmarho[2]-sigmarho[1]*sigmarho[3])/(sigma[1]*sigma[3]);
                        deriv=aa*ND2(-xx,-yy,rho)/sc;
                    }
                else
                    {
                        //!!! idx=3
                        aa=exp(-0.5*limit[3]*limit[3]);

                        xx=(limit[1]-sigmarho[2]*limit[3])/sigma[2];
                        yy=(limit[2]-sigmarho[3]*limit[3])/sigma[3];
                        rho=(sigmarho[1]-sigmarho[2]*sigmarho[3])/(sigma[2]*sigma[3]);
                        deriv=aa*ND2(-xx,-yy,rho)/sc;
                    }

            }
        return(deriv);
    }


    Real BVTL(int NU, Real DH, Real DK, Real RRR );
    Real TVTMFN(Real X, Real H1, Real H2, Real H3,
                  Real R23, Real RUA, Real RUB, Real AR,
                  Real RUC, int NUC);
    template <class Integrand>
    Real KRNRDT(Real A, Real B, const Integrand& integrand, Real& error);

    template <class Integrand>
    Real ADONET(Real A, Real B, Real TOL, const Integrand& integrand);

    Real tvtl(int NU, const Real limit[4], const Real sigmarho[4], Real epsi) {
        /*
          A function for computing trivariate normal and t-probabilities.

          This function uses algorithms developed from the ideas
          described in the papers:
          R.L. Plackett, Biometrika 41(1954), pp. 351-360.
          Z. Drezner, Math. Comp. 62(1994), pp. 289-294.
          and uses adaptive integration.

          The software given here is based on algorithms described in
          the paper A. Genz: "Numerical Computation of Rectangular
          Bivariate and Trivariate Normal and t Probabilities",
          Statistics and Computing 14 (2004) 251-260.

          This software has been developed by M.C. Recchioni based on
          previous software developed by
          Alan Genz
          Department of Mathematics
          Washington State University
          Pullman, WA 99164-3113
          Email : alangenz@wsu.edu
          The software developed by A. Genz is available free of
          charge in the website:
          www.math.wsu.edu/faculty/genz/software/software.html

          The software calculates the probability that
          X(I) < H(I), for I = 1,2,3

          NU        INTEGER degrees of freedom; use NU = 0 for normal cases.
          LIMIT     REAL array of uppoer limits for probability distribution
          SIGMARHO  REAL array of three correlation coefficients, should
          contain the lower left portion of the correlation matrix.
          SIGMARHO should contains the values r21, r31, r23 in that order.
          EPSI      REAL required absolute accuracy; maximum accuracy for most
          computations is approximately 1D-14

        */

        Real result;
        const Real ONE=1.0, ZRO=0.0;
        Real EPS, TVT;
        Real PT, H1, H2, H3, R12, R13, R23, RUA, RUB, AR, RUC;
        EPS = max( 1.e-14, epsi );
        PT=halfPi;

        H1 = limit[1];
        H2 = limit[2];
        H3 = limit[3];
        R12 = sigmarho[1];
        R13 = sigmarho[2];
        R23 = sigmarho[3];
        /*
         *     Sort R's and check for special cases
         */
        if ( fabs(R12) > fabs(R13) ) {
            H2 = H3;
            H3 = limit[2];
            R12 = R13;
            R13 = sigmarho[1];
        }

        if ( fabs(R13) > fabs(R23) ) {
            H1 = H2;
            H2 = limit[1];
            R23 = R13;
            R13 = sigmarho[3];
        }

        TVT = 0.0;
        if ( (fabs(H1) + fabs(H2) + fabs(H3)) < EPS ) TVT = ( 1 + ( asin(R12) + asin(R13) + asin(R23) )/PT )/8.0;

        else  if ( (NU < 1) && ( (fabs(R12) + fabs(R13)) < EPS) )  TVT = PHID(H1)*BVTL( NU, H2, H3, R23 );

        else  if ( (NU < 1) && ((fabs(R13) + fabs(R23))< EPS) ) TVT = PHID(H3)*BVTL( NU, H1, H2, R12 );

        else if( (NU < 1) && ((fabs(R12) + fabs(R23))< EPS) ) TVT = PHID(H2)*BVTL( NU, H1, H3, R13 );

        else if ( (1.0 - R23)< EPS ) TVT = BVTL( NU, H1, min( H2, H3 ), R12 );

        else  if ( (R23 + 1.0) <EPS ) {
            if  ( H2 > -H3 ) TVT = BVTL( NU, H1, H2, R12 ) - BVTL( NU, H1, -H3, R12 );}
        else
            {
                /*
                 *        Compute singular TVT value
                 */
                if ( NU < 1 ) TVT = BVTL( NU, H2, H3, R23 )*PHID(H1);

                else if ( R23 > 0 ) TVT = BVTL( NU, H1, min( H2, H3 ), ZRO );

                else if ( H2 > -H3 ) TVT = BVTL( NU, H1, H2, ZRO ) - BVTL( NU, H1, -H3, ZRO );

                /*
                 *        Use numerical integration to compute probability
                 *
                 */
                RUA = asin( R12 );
                RUB = asin( R13 );
                AR = asin( R23);
                RUC = SIGN( PT, AR ) - AR;
                const auto integrand = [&](Real X) {
                    return TVTMFN(X, H1, H2, H3, R23, RUA, RUB, AR, RUC, NU);
                };
                TVT = TVT + ADONET(ZRO, ONE, EPS, integrand) / (4.0 * PT);
            }
        result = max( ZRO, min( TVT, ONE ) );

        return(result);
    }

    void SINCS(Real v1,Real& v2, Real& v3);
    Real PNTGND(int , Real ,Real ,Real ,
                  Real ,Real ,Real ,Real );

    Real TVTMFN(Real X, Real H1, Real H2, Real H3, Real R23,
                  Real RUA, Real RUB, Real AR,Real RUC, int NUC ){
        /*
          Computes Plackett formula integrands
        */

        Real R12=0.0, RR2=0, R13=0.0, RR3=0.0, R=0.0, RR=0.0;
        const Real ZRO = 0.0;
        Real result = 0.0;

        SINCS( RUA*X, R12, RR2 );
        SINCS( RUB*X, R13, RR3 );

        if ( fabs(RUA)> 0 )  result += RUA*PNTGND( NUC, H1,H2,H3, R13,R23,R12,RR2);
        if( fabs(RUB)>0 ) result += RUB*PNTGND( NUC, H1,H3,H2, R12,R23,R13,RR3 ) ;
        if ( NUC > 0 )
            {
                SINCS( AR + RUC*X, R, RR );
                result -= RUC*PNTGND( NUC, H2, H3, H1, ZRO, ZRO, R, RR );
            }
        return(result);
    }
    //


    void SINCS(Real X, Real& SX, Real& CS )
    {
        /*
          Computes SIN(X), COS(X)^2, with series approx. for |X| near PI/2
        */
        constexpr double PT = 1.57079632679489661923132169163975;
        Real EE;
        EE = (PT - fabs(X))*(PT - fabs(X));

        if ( EE < 5e-5 )
            {
                SX = SIGN( 1 - EE*( 1 - EE/12 )/2, X );
                CS = EE*( 1 - EE*( 1 - 2*EE/15 )/3 );
            }
        else
            {
                SX = sin(X);
                CS = 1 - SX*SX;
            }
    }
    //

    template <class Integrand>
    Real ADONET(Real A, Real B, Real TOL, const Integrand& integrand) {
        //
        //     One Dimensional Globally Adaptive Integration Function
        //
        const Size NL=100;
        Size I, IM, IP;
        Real EI[101], AI[101], BI[101], FI[101], FIN=0.0;
        Real result,ERR;


        AI[1] = A;
        BI[1] = B;
        ERR = 1;
        IP = 1;
        IM = 1;
        while ( ((4*ERR)> TOL) && (IM< NL) )
            {
                IM = IM + 1;
                BI[IM] = BI[IP];
                AI[IM] = (AI[IP] + BI[IP] )/2.0;
                BI[IP] = AI[IM];
                FI[IP] = KRNRDT(AI[IP], BI[IP], integrand, EI[IP]);
                FI[IM] = KRNRDT(AI[IM], BI[IM], integrand, EI[IM]);

                ERR = 0.0;
                FIN = 0.0;
                for(I = 1; I<=IM; I++)
                    {
                        if( EI[I] > EI[IP]) IP = I;
                        FIN = FIN + FI[I];
                        ERR = ERR + EI[I]*EI[I];
                    }
                ERR = std::sqrt(ERR);
            }
        result=FIN;
        //   ADONET = FIN
        return(result);
    }
    //

    template <class Integrand>
    Real KRNRDT(Real A, Real B, const Integrand& integrand, Real& ERR) {

        //
        //     Kronrod Rule
        //
        Real T, CEN, FC, WID, RESG, RESK;

        Real result;
        //
        //        The abscissae and weights are given for the interval (-1,1);
        //        only positive abscissae and corresponding weights are given.
        //
        //        XGK    - abscissae of the 2N+1-point Kronrod rule:
        //                 XGK(2), XGK(4), ...  N-point Gauss rule abscissae;
        //                 XGK(1), XGK(3), ...  optimally added abscissae.
        //        WGK    - weights of the 2N+1-point Kronrod rule.
        //        WG     - weights of the N-point Gauss rule.
        //
        int J, N=11;

        static const Real WG[7] = {
            0.0, 0.2729250867779007, 0.05566856711617449,
            0.1255803694649048, 0.1862902109277352,
            0.2331937645919914, 0.2628045445102478
        };
        static const Real XGK[13] = {
            0.0, 0.0000000000000000, 0.9963696138895427,
            0.9782286581460570, 0.9416771085780681,
            0.8870625997680953, 0.8160574566562211,
            0.7301520055740492, 0.6305995201619651,
            0.5190961292068118, 0.3979441409523776,
            0.2695431559523450, 0.1361130007993617
        };
        static const Real WGK[13] = {
            0.0, 0.1365777947111183, 0.00976544104596129,
            0.02715655468210443, 0.04582937856442671,
            0.06309742475037484, 0.07866457193222764,
            0.09295309859690074, 0.1058720744813894,
            0.1167395024610472, 0.1251587991003195,
            0.1312806842298057, 0.1351935727998845
        };
        /*
          Major variables

          CEN  - mid point of the interval
          WID  - half-length of the interval
          RESG - result of the N-point Gauss formula
          RESK - result of the 2N+1-point Kronrod formula
          Compute the 2N+1-point Kronrod approximation to
          the integral, and estimate the absolute error.
        */
        WID = ( B - A )/2.0;
        CEN = ( B + A )/2.0;

        FC = integrand(CEN);

        RESG = FC*WG[0+1];
        RESK = FC*WGK[0+1];

        for (J = 1; J<= N; J++)
            {
                T = WID*XGK[J+1];
                FC = integrand(CEN-T) + integrand(CEN+T);
                RESK = RESK + WGK[J+1]*FC;
                if((J-2*int(J/2)) == 0 ) RESG = RESG + WG[1+J/2]*FC;
            }
        result = WID*RESK;
        ERR = fabs( WID*( RESK - RESG ) );
        return(result);
    }

    //
    Real  STUDNT(int NU, Real T )
    {
        /*
          Student t Distribution Function
        */
        const Real ZRO=0.0, ONE=1.0;
        Real CSSTHE, SNTHE, POLYN, TT, TS, RN;
        Real result;


        if ( NU < 1 ) result= PHID( T );
        else if ( NU == 1 ) result = ( 1 + 2.0*atan(T)/PI )/2.0;
        else if ( NU == 2 ) result = ( 1 + T/std::sqrt(2.0 + T*T))/2.0;
        else
            {
                TT = T*T;
                CSSTHE = 1/( 1 + TT/double(NU) );
                POLYN = 1;
                for (int J = NU-2; J >= 2; J -= 2)
                    {
                        POLYN = 1.0 + ( J - 1.0 )*CSSTHE*POLYN/(double)J;
                    }
                if ((NU-2*int(NU/2) ) == 1 )
                    {
                        RN = NU;
                        TS = T/std::sqrt(RN);
                        result = ( 1.0 + 2.0*( atan(TS) + TS*CSSTHE*POLYN )/PI )/2.0;
                    }
                else
                    {
                        SNTHE = T/std::sqrt(NU+TT);
                        result = ( 1 + SNTHE*POLYN )/2.0;
                    }
                result = max( ZRO, min( result, ONE ) );
            }
        return(result);
    }

    //
    Real BVTL(int NU, Real DH, Real DK, Real R )
    {
        /*
          A function for computing bivariate t probabilities.
          This function is based on the method described by
          Dunnett, C.W. and M. Sobel, (1954),
          A bivariate generalization of Student's t-distribution
          with tables for certain special cases,
          Biometrika 41, pp. 153-169.
          The software given here has been developed by M.C. Recchioni based on previous
          software developed by
          Alan Genz
          Department of Mathematics
          Washington State University
          Pullman, WA 99164-3113
          Email : alangenz@wsu.edu
          The software developed by A. Genz is available free of charge in
          the website: www.math.wsu.edu/faculty/genz/software/software.html
          ***

          BVTL - calculate the probability that X < DH and Y < DK.

          parameters

          NU number of degrees of freedom
          DH 1st lower integration limit
          DK 2nd lower integration limit
          R   correlation coefficient
        */
        Real TPI, ORS, HRK, KRH, BVT, SNU;
        Real GMPH, GMPK, XNKH, XNHK, QHRK, HKN, HPK, HKRN;
        Real BTNCKH, BTNCHK, BTPDKH, BTPDHK, ONE, EPS;
        Real result;
        ONE = 1;
        EPS = 1e-15;
        if ( NU <1 ) result = ND2( -DH, -DK, R );

        else if ( (1 - R)<= EPS ) result = STUDNT( NU, min( DH, DK ) );

        else  if( (R + 1)<=EPS )
            {
                if( DH > -DK ) result = STUDNT( NU, DH ) - STUDNT( NU, -DK );
                else
                    result = 0.0;
            }
        else
            {
                const int HS = static_cast<int>(SIGN(ONE, DH - R*DK));
                const int KS = static_cast<int>(SIGN(ONE, DK - R*DH));
                TPI = twoPi;
                SNU = (double)NU;
                SNU = std::sqrt(SNU);
                ORS = 1.0 - R*R;
                HRK = DH - R*DK;
                KRH = DK - R*DH;
                if((fabs(HRK) + ORS)> 0 )
                    {
                        XNHK = HRK*HRK/( HRK*HRK + ORS*( NU + DK*DK ) );
                        XNKH = KRH*KRH/( KRH*KRH+ ORS*( NU + DH*DH ) );
                    }
                else
                    {
                        XNHK = 0.0;
                        XNKH = 0.0;
                    }

                if((NU-2*(int)(NU/2))==0 )
                    {
                        BVT = atan2( std::sqrt(ORS), -R )/TPI;
                        GMPH = DH/std::sqrt(16*( NU + DH*DH ));
                        GMPK = DK/std::sqrt(16*( NU + DK*DK));
                        BTNCKH = 2*atan2( std::sqrt(XNKH), std::sqrt(1-XNKH) )/PI;
                        BTPDKH = 2*std::sqrt(XNKH*(1-XNKH))/PI;
                        BTNCHK = 2*atan2( std::sqrt(XNHK), std::sqrt(1-XNHK) )/PI;
                        BTPDHK = 2*std::sqrt(XNHK*(1-XNHK))/PI;
                        for (int J = 1; J <= NU/2; ++J)
                            {
                                BVT = BVT + GMPH*( 1 + KS*BTNCKH );
                                BVT = BVT + GMPK*( 1 + HS*BTNCHK );
                                BTNCKH = BTNCKH + BTPDKH;
                                BTPDKH = 2*J*BTPDKH*( 1 - XNKH )/( 2*J + 1 );
                                BTNCHK = BTNCHK + BTPDHK;
                                BTPDHK = 2*J*BTPDHK*( 1 - XNHK )/( 2*J + 1 );
                                GMPH = GMPH*( 2*J - 1 )/( 2*J*( 1 + DH*DH/NU ) );
                                GMPK = GMPK*( 2*J - 1 )/( 2*J*( 1 + DK*DK/NU ) );
                            }
                    }
                else
                    {
                        QHRK = std::sqrt(DH*DH + DK*DK - 2*R*DH*DK + NU*ORS) ;
                        HKRN = DH*DK + R*NU ;
                        HKN = DH*DK - NU;
                        HPK = DH + DK;
                        BVT = atan2( -SNU*( HKN*QHRK + HPK*HKRN ),HKN*HKRN-NU*HPK*QHRK )/TPI;
                        if ( BVT < -EPS ) BVT = BVT + 1;
                        GMPH = DH/( TPI*SNU*( 1 + DH*DH/NU ) );
                        GMPK = DK/( TPI*SNU*( 1 + DK*DK/NU ) );
                        BTNCKH = std::sqrt(XNKH);
                        BTPDKH = BTNCKH;
                        BTNCHK = std::sqrt(XNHK);
                        BTPDHK = BTNCHK;
                        for (int J = 1; J <= (NU - 1)/2; ++J)
                            {
                                BVT = BVT + GMPH*( 1 + KS*BTNCKH );
                                BVT = BVT + GMPK*( 1 + HS*BTNCHK );
                                BTPDKH = ( 2*J - 1 )*BTPDKH*( 1 - XNKH )/( 2*J );
                                BTNCKH = BTNCKH + BTPDKH;
                                BTPDHK = ( 2*J - 1 )*BTPDHK*( 1 - XNHK )/( 2*J );
                                BTNCHK = BTNCHK + BTPDHK;
                                GMPH = 2*J*GMPH/( ( 2*J + 1 )*( 1 + DH*DH/NU ) );
                                GMPK = 2*J*GMPK/( ( 2*J + 1 )*( 1 + DK*DK/NU ) );
                            }
                    }
                result = BVT;
            }

        return(result);

    }




      Real PNTGND(int NUC, Real BA, Real BB, Real BC, Real RA, Real RB, Real R, Real RR) {
          /*
            Computes Plackett formula integrand
          */
          Real DT, FT, BT,result;

          result = 0.0;
          DT = RR*( RR - (RA-RB)*(RA-RB) - 2*RA*RB*( 1 - R ) );
          if( DT > 0 ) {
              BT = ( BC*RR + BA*( R*RB - RA ) + BB*( R*RA -RB ) )/std::sqrt(DT);
              FT = std::sqrt(BA - R*BB)/RR + BB*BB;
              if( NUC<1 ) {
                  if ( (BT > -10) && (FT <100) ) {
                      result = exp( -FT/2 );
                      if ( BT <10 ) result= result*PHID(BT);
                  } else {
                      FT = std::sqrt(1 + FT/NUC);
                      result = STUDNT( NUC, BT/FT )/std::pow(FT,NUC);
                  }
              }
          }
          return(result);
      }



    //***********************************************************
    Real ND2(Real a, Real b, Real rho ){
        /*
         *     A function for computing bivariate normal probabilities.
         *     This function is based on the method described by
         *     Z. Drezner and G.O. Wesolowsky, (1989),
         *     On the computation of the bivariate normal integral,
         *     Journal of Statist. Comput. Simul. 35, pp. 101-107,
         *     with major modifications for double precision, and for |R| close to 1.
         *     The software given here has been developed by M.C. Recchioni based on previous
         *     software developed by:
         *     Alan Genz
         *     Department of Mathematics
         *     Washington State University
         *     Pullman, WA 99164-3113
         *     Email : alangenz@wsu.edu
         *     The software developed by A. Genz is available free of charge in the website:
         *     www.math.wsu.edu/faculty/genz/software/software.html
         *
         *
         *      ND2 calculates the probability that X > DH and Y > DK.
         *      Note: Prob( X < DH, Y < DK ) = ND2( -DH, -DK, R ).
         *
         * Parameters
         *
         *   DH  DOUBLE PRECISION, integration limit
         *   DK  DOUBLE PRECISION, integration limit
         *   R   DOUBLE PRECISION, correlation coefficient
         */
        Real result, DK, DH, R;
        int I, IS, LG, NG;

        static const Real XL[11][4] = {
            { 0.0, 0.0, 0.0, 0.0 },
            { 0.0, -0.9324695142031522, -0.9815606342467191, -0.9931285991850949 },
            { 0.0, -0.6612093864662647, -0.9041172563704750, -0.9639719272779138 },
            { 0.0, -0.2386191860831970, -0.7699026741943050, -0.9122344282513259 },
            { 0.0, 0.0, -0.5873179542866171, -0.8391169718222188 },
            { 0.0, 0.0, -0.3678314989981802, -0.7463319064601508 },
            { 0.0, 0.0, -0.1252334085114692, -0.6360536807265150 },
            { 0.0, 0.0, 0.0, -0.5108670019508271 },
            { 0.0, 0.0, 0.0, -0.3737060887154196 },
            { 0.0, 0.0, 0.0, -0.2277858511416451 },
            { 0.0, 0.0, 0.0, -0.07652652113349733 }
        };
        static const Real WL[11][4] = {
            { 0.0, 0.0, 0.0, 0.0 },
            { 0.0, 0.1713244923791705, 0.0471753363865111, 0.01761400713915212 },
            { 0.0, 0.3607615730481384, 0.1069393259953183, 0.04060142980038694 },
            { 0.0, 0.4679139345726904, 0.1600783285433464, 0.06267204833410906 },
            { 0.0, 0.0, 0.2031674267230659, 0.08327674157670475 },
            { 0.0, 0.0, 0.2334925365383547, 0.1019301198172404 },
            { 0.0, 0.0, 0.2491470458134029, 0.1181945319615184 },
            { 0.0, 0.0, 0.0, 0.1316886384491766 },
            { 0.0, 0.0, 0.0, 0.1420961093183821 },
            { 0.0, 0.0, 0.0, 0.1491729864726037 },
            { 0.0, 0.0, 0.0, 0.1527533871307259 }
        };
        Real AS, AA, BB, C, D, RS, XS, BVN;
        Real SN, ASR, H, K, BS, HS, HK;

        R=rho;
        DH=a;
        DK=b;

        if( fabs(R) < 0.3 ) {
            NG = 1;
            LG = 3; }
        else if ( fabs(R) < 0.75 ) {
            NG = 2;
            LG = 6;}
        else{
            NG = 3;
            LG = 10;
        }
        H = DH;
        K = DK;
        HK = H*K;

        BVN = 0.0 ;
        if( fabs(R) < 0.925 ) {
            if( fabs(R) > 0 ) {
                HS = ( H*H + K*K )/2;
                ASR = asin(R);
                for (I = 1;I<= LG; I++){
                    for( IS = -1; IS<= 1; IS=IS+2){
                        SN = sin( ASR*(  IS*XL[I][NG] + 1 )/2 );
                        BVN = BVN + WL[I][NG]*exp( ( SN*HK-HS )/( 1.0-SN*SN ) );

                    }
                }
                BVN = BVN*ASR/( 2*twoPi );

            }

            BVN = BVN + PHID(-H)*PHID(-K);

        }
        else
            {
                if ( R < 0 ) {
                    K = -K;
                    HK = -HK;
                }

                if( fabs(R) <1 ) {
                    AS = ( 1 - R )*( 1 + R );
                    AA = std::sqrt(AS);

                    BS = (H-K)*(H-K);
                    C = ( 4 - HK )/8 ;
                    D = ( 12 - HK )/16;
                    ASR = -( BS/AS + HK )/2;
                    if( ASR > -100 ) BVN = AA*exp(ASR)*( 1 - C*( BS - AS )*( 1 - D*BS/5 )/3 + C*D*AS*AS/5 );
                    if( -HK<100 ){
                        BB = std::sqrt(BS);
                        BVN = BVN - exp( -HK/2 )*sqrtTwoPi*PHID(-BB/AA)*BB*( 1 - C*BS*( 1 - D*BS/5 )/3 );
                    }
                    AA = AA/2   ;
                    for (I = 1; I<= LG;I++){
                        for( IS = -1; IS<=1; IS=IS+2){
                            XS =(AA*(IS*XL[I][NG]+1))*(AA*(IS*XL[I][NG]+1));
                            RS = std::pow( (1 - XS),2 );
                            ASR = -( BS/XS + HK )/2;
                            if ( ASR > -100 ) {

                                BVN = BVN + AA*WL[I][NG]*exp( ASR )*(exp( -HK*( 1 - RS )/( 2*( 1 + RS ) ) )/RS- ( 1 + C*XS*( 1 + D*XS ) ) );


                            }
                        }
                    }
                    BVN = -BVN/twoPi;
                }
                if ( R > 0 )  {

                    BVN =  BVN + PHID( -max( H, K ) );

                }
                else
                    {
                        BVN = -BVN;
                        if( K > H ) BVN = BVN + PHID(K) - PHID(H);
                    }
            }


        result=BVN;

        return(result);

    }

        struct integr_adapter {
            ext::shared_ptr<YieldTermStructure> r;
            explicit integr_adapter(const ext::shared_ptr<GeneralizedBlackScholesProcess>& process)
            : r(*(process->riskFreeRate())) {}
            Real operator()(Real t1,Real t2) const {
                return r->forwardRate(t1,t2,Continuous) * (t2-t1);
            }
        };

        struct integalpha_adapter {
            ext::shared_ptr<YieldTermStructure> r;
            ext::shared_ptr<YieldTermStructure> q;
            explicit integalpha_adapter(
                const ext::shared_ptr<GeneralizedBlackScholesProcess>& process)
            : r(*(process->riskFreeRate())), q(*(process->dividendYield())) {}
            Real operator()(Real t1,Real t2) const {
                Real alpha = r->forwardRate(t1,t2,Continuous).rate()
                           - q->forwardRate(t1,t2,Continuous).rate();
                return alpha * (t2-t1);
            }
        };

        struct alpha_adapter {
            ext::shared_ptr<YieldTermStructure> r;
            ext::shared_ptr<YieldTermStructure> q;
            explicit alpha_adapter(const ext::shared_ptr<GeneralizedBlackScholesProcess>& process)
            : r(*(process->riskFreeRate())), q(*(process->dividendYield())) {}
            Real operator()(Real t) const {
                return r->forwardRate(t,t,Continuous).rate()
                     - q->forwardRate(t,t,Continuous).rate();
            }
        };

        struct sigmaq_adapter {
            ext::shared_ptr<BlackVolTermStructure> v;
            Real s;
            explicit sigmaq_adapter(const ext::shared_ptr<GeneralizedBlackScholesProcess>& process)
            : v(*(process->blackVolatility())), s(process->x0()) {}
            Real operator()(Real t) const {
                Real sigma = v->blackForwardVol(t,t,s,true);
                return sigma*sigma;
            }
        };

        struct integs_adapter {
            ext::shared_ptr<BlackVolTermStructure> v;
            Real s;
            explicit integs_adapter(const ext::shared_ptr<GeneralizedBlackScholesProcess>& process)
            : v(*(process->blackVolatility())), s(process->x0()) {}
            Real operator()(Real t1,Real t2) const {
                return v->blackForwardVariance(t1,t2,s,true);
            }
        };

    }


    PerturbativeBarrierOptionEngine::PerturbativeBarrierOptionEngine(
                ext::shared_ptr<GeneralizedBlackScholesProcess> process, Natural order,
                bool zeroGamma, Size npoint, Size npoint2)
        : process_(std::move(process)), order_(order), zeroGamma_(zeroGamma),
            npoint_(npoint), npoint2_(npoint2) {
        registerWith(process_);
    }

    void PerturbativeBarrierOptionEngine::calculate() const {

        QL_REQUIRE(arguments_.barrierType == Barrier::UpOut,
                   "this engine only manages up-and-out options");

        QL_REQUIRE(arguments_.rebate == 0.0,
                   "this engine does not manage non-null rebates");

        ext::shared_ptr<PlainVanillaPayoff> payoff =
            ext::dynamic_pointer_cast<PlainVanillaPayoff>(arguments_.payoff);
        QL_REQUIRE(payoff && payoff->optionType() == Option::Put,
                   "this engine only manages put options");

        Real stock = process_->x0();
        Real kprice = payoff->strike();
        Real hbarr = arguments_.barrier;

        Time tauMin = 0.0;
        Time tauMax = process_->time(arguments_.exercise->lastDate());

        QL_REQUIRE(order_ <= 2, "order must be <= 2");
        QL_REQUIRE(npoint_ > 0 && npoint2_ > 0,
               "integration point counts must be positive");

        int igm = zeroGamma_ ? 0 : 1;
        
        results_.value = BarrierUPD(kprice, stock, hbarr,
                                    tauMin, tauMax, order_, igm, npoint_, npoint2_,
                                    integr_adapter(process_),
                                    integalpha_adapter(process_),
                                    integs_adapter(process_),
                                    alpha_adapter(process_),
                                    sigmaq_adapter(process_));
    }

}

