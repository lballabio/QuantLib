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

#define PI 3.14159265358979324

namespace QuantLib {

    namespace {

    Real ND2(Real a, Real b, Real rho);

    Real H1, H2,  H3, R23, RUA, RUB, AR, RUC;
    int NUC;

    // standard normal cumulative distribution function
    Real PHID(Real Z);

    // Functions used to compute the first order approximation
    Real ff(Real p,Real tt,Real a, Real b, Real gm);
    Real v(Real p, Real tt,Real a,Real b,Real gm);
    Real llold(Real p,Real tt, Real a, Real b,
                 Real c, Real gm);

    // Functions used to compute the second order approximation
    Real derivn3(Real limit[4],Real sigmarho[4], int idx);
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

    Real BarrierUPD(Real kprice,
                    Real stock,
                    Real hbarr,
                    Real taumin,
                    Real taumax,
                    int iord,
                    int igm,
                    const std::function<Real(Real, Real)>& integr,
                    const std::function<Real(Real, Real)>& integalpha,
                    const std::function<Real(Real, Real)>& integs,
                    const std::function<Real(Real)>& alpha,
                    const std::function<Real(Real)>& sigmaq) {
        Real v0=0.0, v1=0.0, v1p=0.0, v2p=0.0, v2pp=0.0, gm=0.0;
        int i=0,j=0;
        Real tmp=0.0, e1=0.0, e2=0.0, e3=0.0, e4=0.0;
        Real xstar=0.0, s0=0.0;
        Real sigmat=0.0, disc=0.0, d1=0.0,d2=0.0,d3=0.0,d4=0.0;
        Real et=0.0,tt=0.0, dt=0.0,p=0.0;
        int npoint,npoint2;
        static double pi= 3.14159265358979324;
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
        d1=(xstar-log(s0)+(1.0-gm)*0.5*sigmat)/sqrt(sigmat);
        d2=(xstar+log(s0)+(1.0-gm)*0.5*sigmat)/sqrt(sigmat);
        d3=(xstar-log(s0)-(1.0+gm)*0.5*sigmat)/sqrt(sigmat);
        d4=(xstar+log(s0)-(1.0+gm)*0.5*sigmat)/sqrt(sigmat);

        e1=PHID(d1);
        e2=PHID(d2);
        e3=PHID(d3);
        e4=PHID(d4);

        v0=kprice*e1-kprice*std::pow(s0,(1.0-gm))*e2;
        v0=v0+exp(gm*0.5*sigmat)*(-hbarr*s0*e3+hbarr*std::pow(s0,-gm)*e4);
        v0=v0*exp(disc);

        if(iord==0) return v0;

        /*
          !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
          !!                                           !!
          !! Compute the first order term  P_1         !!
          !!                                           !!
          !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        */

        npoint=1000;
        npoint2=100;

        dt=(taumax-taumin)/double(npoint);

        tt=0.5*integs(taumin,taumax);

        x=log(s0);
        et=exp(0.5*(1.0-gm)*x);

        dsqpi=std::pow(pi,0.5);

        v1=0.0;
        for( i=1;i<=npoint;i++) {
            v1p=0.0;
            tmp=taumin+dt*double(2*i-1)*0.5;
            p=0.5*integs(tmp,taumax);
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
            auxnew=-exp(gm*p)*hbarr*ccaux;
            v1p=v1p+auxnew;

            b=(gm+1.0);
            c=-xstar;
            ccaux=llold(p,tt,x,b,c,gm)-llold(p,tt,-x,b,c,gm);
            auxnew=exp(gm*p)*hbarr*gm*ccaux;
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
            auxnew=-exp(gm*p)*gm*hbarr*(ff(p,tt,x,b,gm)-ff(p,tt,-x,b,gm));
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

        Real v2,dtp, tmp1,s,caux2;
        v2=0.0;

        for(i=1;i<=npoint;i++) {
            v2p=0.0;
            tmp=taumin+dt*(double)(2*i-1)*0.5;
            p=0.5*integs(tmp,taumax);

            dtp=(taumax-tmp)/(double)(npoint2);

            for(j=1;j<=npoint2; j++) {
                tmp1=tmp+dtp*(double)(2*j-1)*0.50;
                s=0.50*integs(tmp1,taumax);

                caux=dll(s,p,tt,-x,-1.0+gm,-xstar,gm)-dll(s,p,tt,x,-1.0+gm,-xstar,gm);
                v2pp=caux*kprice*(1.0-gm);

                caux=dll(s,p,tt,-x,-1.0-gm,xstar,gm)-dll(s,p,tt,x,-1.0-gm,xstar,gm);
                v2pp=v2pp-exp(gm*s)*hbarr*caux;

                caux=dll(s,p,tt,-x,1.0+gm,-xstar,gm)-dll(s,p,tt,x,1.0+gm,-xstar,gm);
                v2pp=v2pp+exp(gm*s)*gm*hbarr*caux;

                caux=+dvv(s,p,tt,-x,xstar,gm)-dvv(s,p,tt,x,xstar,gm);
                caux=caux+(dvv(s,p,tt,-x,-xstar,gm)-dvv(s,p,tt,x,-xstar,gm));
                caux2=hbarr*exp(0.5*(1.0+gm)*xstar)-kprice*exp(-0.5*(1.0-gm)*xstar);
                v2pp=v2pp+caux2*caux;

                caux=dff(s,p,tt,-x,-1.0+gm,gm)-dff(s,p,tt,x,-1.0+gm,gm);
                v2pp=v2pp-(1.0-gm)*kprice*caux;

                caux=dff(s,p,tt,-x,1.0+gm,gm)-dff(s,p,tt,x,1.0+gm,gm);
                v2pp=v2pp-exp(gm*s)*gm*hbarr*caux;

                v2pp=v2pp*0.5*(1.0-gm);

                caux=-ddll(s,p,tt,-x,-1.0+gm,-xstar,gm)+ddll(s,p,tt,x,-1.0+gm,-xstar,gm);
                v2pp=v2pp+caux*kprice*(1.0-gm);

                caux=-ddll(s,p,tt,-x,-1.0-gm,xstar,gm)+ddll(s,p,tt,x,-1.0-gm,xstar,gm);
                v2pp=v2pp-exp(gm*s)*hbarr*caux;

                caux=-ddll(s,p,tt,-x,1.0+gm,-xstar,gm)+ddll(s,p,tt,x,1.0+gm,-xstar,gm);
                v2pp=v2pp+exp(gm*s)*gm*hbarr*caux;

                caux=-ddvv(s,p,tt,-x,xstar,gm)+ddvv(s,p,tt,x,xstar,gm);
                caux=caux+(-dvv(s,p,tt,-x,-xstar,gm)+dvv(s,p,tt,x,-xstar,gm));
                caux2=hbarr*exp(0.5*(1.0+gm)*xstar)-kprice*exp(-0.5*(1-gm)*xstar);

                v2pp=v2pp+caux2*caux;

                caux=-ddff(s,p,tt,-x,-1+gm,gm)+ddff(s,p,tt,x,-1+gm,gm);
                v2pp=v2pp-(1.0-gm)*kprice*caux;

                caux=-ddff(s,p,tt,-x,1.0+gm,gm)+ddff(s,p,tt,x,1.0+gm,gm);
                v2pp=v2pp-exp(gm*s)*gm*hbarr*caux;

                v2p=v2p+(alpha(tmp1)-gm*0.5*sigmaq(tmp1))*v2pp;
            }

            v2=v2+v2p*(alpha(tmp)-gm*0.5*sigmaq(tmp))*dtp;
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
        Real P, EXPNTL, CUTOFF, ROOTPI, ZABS;

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
        ROOTPI = 2.506628274631001;
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
                P = EXPNTL/(ZABS + 1/(ZABS + 2/(ZABS + 3/(ZABS + 4/(ZABS + 0.65)))))/ROOTPI;

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
        Real ppi= 3.14159265358979324;

        aa=-(b*p-b*tt+a)/std::pow(2.0*(tt-p),0.5);

        caux=2.0*std::pow(ppi,0.5)*PHID(aa);
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

        aa=-(p*(a-b)+b*tt)/std::pow(2.0*p*tt*(tt-p),0.5);
        caux=PHID(aa);

        aa=exp(std::pow((a-b),2)/(4.0*tt))*exp(std::pow((1.0-gm),2)*tt/4.0)*std::pow(tt,0.5);
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
        Real ppi= 3.14159265358979324;
        Real aa;

        xx=(-a+b*(tt-p))/std::pow(2.0*(tt-p),0.5);
        yy=(-a+b*tt+c)/std::pow(2.0*tt,0.5);
        rho=std::pow((tt-p)/tt,0.5);
        aa=b*b-(1.0-gm)*(1.0-gm);
        aa=aa/4.0;
        caux=ND2(-xx,-yy,rho);

        bvnd=2.0*std::pow(ppi,0.5)*exp(-a*b*0.5)*exp(aa*(tt-p))*caux;
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
        double ppi= 3.14159265358979324;
        Real result;
        Real aa,caux,caux1,caux2;
        Real xx,yy,rho;

        aa=(a*p+b*(tt-p))/std::pow(2.0*p*tt*(tt-p),0.5);
        caux=PHID(aa);

        aa=exp((a-b)*(a-b)/(4.0*tt))*exp(std::pow((1.0-gm),2)*tt/4.0)*std::pow(tt,0.5);
        caux=-caux/aa;

        xx=(a*p+b*(tt-p))/std::pow(2.0*tt*p*(tt-p),0.5);
        yy=(a*s+b*(tt-s))/std::pow(2.0*tt*s*(tt-s),0.5);
        rho=std::pow((s*(tt-p))/(p*(tt-s)),0.5);
        caux1=ND2(-xx,-yy,rho);
        caux1=caux1/aa;


        aa=exp((a+b)*(a+b)/(4.0*tt))*exp(std::pow((1.0-gm),2)*tt/4.0)*std::pow(tt,0.5);

        xx=(a*p-b*(tt-p))/std::pow(2.0*tt*p*(tt-p),0.5);
        yy=(a*s-b*(tt-s))/std::pow(2.0*tt*s*(tt-s),0.5);
        rho=std::pow((s*(tt-p))/(p*(tt-s)),0.5);
        caux2=ND2(-xx,-yy,rho);
        caux2=caux2/aa;
        result=(caux+caux1+caux2)/(2.0*std::pow(ppi,0.5));
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

        xx=(a-b*(tt-p))/std::pow(2.0*(tt-p),0.5);
        caux=-PHID(xx)*exp(-0.5*a*b);

        xx=(a+b*(tt-p))/std::pow(2.0*(tt-p),0.5);
        yy=(a+b*(tt-s))/std::pow(2.0*(tt-s),0.5);
        rho=std::pow((tt-p)/(tt-s),0.5);
        caux1=ND2(-xx,-yy,rho);
        caux1=exp(0.5*a*b)*caux1;

        xx=(a-b*(tt-p))/std::pow(2.0*(tt-p),0.5);
        yy=(a-b*(tt-s))/std::pow(2.0*(tt-s),0.5);
        rho=std::pow((tt-p)/(tt-s),0.5);
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
        limit[1]=(a+b*(tt-p))/std::pow(2.0*(tt-p),0.5);
        limit[2]=(a+b*(tt-s))/std::pow(2.0*(tt-s),0.5);
        limit[3]=(a+b*tt+c)/std::pow(2.0*tt,0.5);
        sigmarho[1]=std::pow((tt-p)/(tt-s),0.5);
        sigmarho[2]=std::pow((tt-p)/tt,0.5);
        sigmarho[3]=std::pow((tt-s)/tt,0.5);

        caux=exp(0.5*a*b)*tvtl(0,limit,sigmarho,epsi);

        limit[1]=(a-b*(tt-p))/std::pow(2.0*(tt-p),0.5);
        limit[2]=(-a+b*(tt-s))/std::pow(2.0*(tt-s),0.5);
        limit[3]=(-a+b*tt+c)/std::pow(2.0*tt,0.5);
        sigmarho[1]=-std::pow((tt-p)/(tt-s),0.5);
        sigmarho[2]=-std::pow((tt-p)/tt,0.5);
        sigmarho[3]=std::pow((tt-s)/tt,0.5);

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
        double ppi= 3.14159265358979324;

        xx=(a-b*(tt-p))/std::pow(2.0*(tt-p),0.5);
        caux=PHID(xx)*exp(-0.5*a*b);

        xx=(a+b*(tt-p))/std::pow(2.0*(tt-p),0.5);
        yy=(a+b*(tt-s))/std::pow(2.0*(tt-s),0.5);
        rho=std::pow((tt-p)/(tt-s),0.5);
        caux1=ND2(-xx,-yy,rho);
        caux1=exp(0.5*a*b)*caux1;

        xx=(a-b*(tt-p))/std::pow(2.0*(tt-p),0.5);
        yy=(a-b*(tt-s))/std::pow(2.0*(tt-s),0.5);
        rho=std::pow((tt-p)/(tt-s),0.5);
        caux2=ND2(-xx,-yy,rho);
        caux2=-exp(-0.5*a*b)*caux2;

        caux=0.5*b*(caux+caux1+caux2);

        xx=(a+b*(tt-p))/std::pow(2.0*(tt-p),0.5);
        yy=b*std::pow((p-s),0.5)/std::pow(2.0,0.5);
        caux1=exp(-0.5*xx*xx)*exp(0.5*a*b)*PHID(yy)/(2.0*std::pow(ppi*(tt-p),0.5));


        xx=(a+b*(tt-s))/std::pow(2.0*(tt-s),0.5);
        yy=a*std::pow((p-s),0.5)/std::pow(2.0*(tt-p)*(tt-s),0.5);
        caux2=exp(-0.5*xx*xx)*exp(0.5*a*b)*PHID(yy)/(2.0*std::pow(ppi*(tt-s),0.5));

        xx=(a-b*(tt-p))/std::pow(2.0*(tt-p),0.5);
        yy=b*std::pow((p-s),0.5)/std::pow(2.0,0.5);
        caux3=-exp(-0.5*xx*xx)*exp(-0.5*a*b)*PHID(yy)/(2.0*std::pow(ppi*(tt-p),0.5));


        xx=(a-b*(tt-s))/std::pow(2.0*(tt-s),0.5);
        yy=a*std::pow((p-s),0.5)/std::pow(2.0*(tt-p)*(tt-s),0.5);
        caux4=exp(-0.5*xx*xx)*exp(-0.5*a*b)*PHID(yy)/(2.0*std::pow(ppi*(tt-s),0.5));



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
        static Real result;
        static Real aa,caux,caux1;
        static Real sigmarho[4],limit[4];
        static int idx;
        Real epsi;

        epsi=1.e-12;
        limit[1]=(ax+bx*(tt-p))/std::pow(2.0*(tt-p),0.5);
        limit[2]=(ax+bx*(tt-s))/std::pow(2.0*(tt-s),0.5);
        limit[3]=(ax+bx*tt+c)/std::pow(2.0*tt,0.5);
        sigmarho[1]=std::pow((tt-p)/(tt-s),0.5);
        sigmarho[2]=std::pow((tt-p)/tt,0.5);
        sigmarho[3]=std::pow((tt-s)/tt,0.5);

        caux=0.5*bx*tvtl(0,limit,sigmarho,epsi);


        idx=1;
        caux=caux+derivn3(limit,sigmarho,idx)/std::pow(2.0*(tt-p),0.5);

        idx=2;
        caux=caux+derivn3(limit,sigmarho,idx)/std::pow(2.0*(tt-s),0.5);

        idx=3;
        caux=caux+derivn3(limit,sigmarho,idx)/std::pow(2.0*tt,0.5);

        caux=exp(0.5*ax*bx)*caux;

        limit[1]=(ax-bx*(tt-p))/std::pow(2.0*(tt-p),0.5);
        limit[2]=(-ax+bx*(tt-s))/std::pow(2.0*(tt-s),0.5);
        limit[3]=(-ax+bx*tt+c)/std::pow(2.0*tt,0.5);
        sigmarho[1]=-std::pow((tt-p)/(tt-s),0.5);
        sigmarho[2]=-std::pow((tt-p)/tt,0.5);
        sigmarho[3]=std::pow((tt-s)/tt,0.5);

        caux1=0.5*bx*tvtl(0,limit,sigmarho,epsi);

        idx=1;
        caux1=caux1-derivn3(limit,sigmarho,idx)/std::pow(2.0*(tt-p),0.5);

        idx=2;
        caux1=caux1+derivn3(limit,sigmarho,idx)/std::pow(2.0*(tt-s),0.5);


        idx=3;
        caux1=caux1+derivn3(limit,sigmarho,idx)/std::pow(2.0*tt,0.5);

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
        static Real result;
        static Real aa,caux,caux1,caux2,caux6;
        static Real caux3,caux4,caux5,aux;
        static Real xx,yy,rho;
        static double ppi= 3.14159265358979324;

        aa=(a*p+b*(tt-p))/std::pow(2.0*p*tt*(tt-p),0.5);
        caux=PHID(aa);

        aa=exp(-(a-b)*(a-b)/(4.0*tt))/tt;

        caux=0.5*aa*caux*(a-b);

        xx=(a*p+b*(tt-p))/std::pow(2.0*tt*p*(tt-p),0.5);
        yy=(a*s+b*(tt-s))/std::pow(2.0*tt*s*(tt-s),0.5);
        rho=std::pow((s*(tt-p))/(p*(tt-s)),0.5);
        caux1=ND2(-xx,-yy,rho);
        caux1=-0.5*aa*caux1*(a-b);


        aa=exp(-(a+b)*(a+b)/(4.0*tt))/tt;

        xx=(a*p-b*(tt-p))/std::pow(2.0*tt*p*(tt-p),0.5);
        yy=(a*s-b*(tt-s))/std::pow(2.0*tt*s*(tt-s),0.5);
        rho=std::pow((s*(tt-p))/(p*(tt-s)),0.5);
        caux2=ND2(-xx,-yy,rho);
        caux2=-0.5*aa*caux2*(a+b);

        aa=-b*std::pow((p-s)/std::pow(2.0*p*s,0.5),0.5);
        aux=std::pow(p/(ppi*tt*(tt-p)),0.5)*PHID(aa);

        xx=(a+b)*(a+b)/(4.0*tt);
        yy=std::pow((a*p-b*(tt-p)),2)/(4.0*p*tt*(tt-p));
        caux3=aux*exp(-xx)*exp(-yy)/2.0;


        xx=(a-b)*(a-b)/(4.0*tt);
        yy=std::pow((a*p+b*(tt-p)),2)/(4.0*p*tt*(tt-p));
        caux4=aux*exp(-xx)*exp(-yy)/2.0;

        aa=a*std::pow((p-s)/std::pow(2.0*(tt-p)*(tt-s),0.5),0.5);
        aux=std::pow(s/(ppi*tt*(tt-s)),0.5)*PHID(aa);

        xx=(a+b)*(a+b)/(4.0*tt);
        yy=std::pow((a*s-b*(tt-s)),2)/(4.0*s*tt*(tt-s));
        caux5=aux*exp(-xx)*exp(-yy)/2.0;

        xx=(a-b)*(a-b)/(4.0*tt);
        yy=std::pow((a*s+b*(tt-s)),2)/(4.0*s*tt*(tt-s));
        caux6=aux*exp(-xx)*exp(-yy)/2.0;

        aux=exp((1.0-gm)*(1.0-gm)*tt/4.0)*std::pow(tt,0.5);

        result=(caux+caux1+caux2+caux3+caux4+caux5+caux6)/(aux*2.0*std::pow(ppi,0.5));
        return(result);
    }

    /*
      !!
      !! Derivn3 computes the derivatives of the trivariate cumulative normal
      !! distribution with respect to one of the integration limits
      !!
    */
    Real derivn3(Real limit[4],Real sigmarho[4], int idx)
    {
        static Real aa;
        static Real xx,yy,rho,sc;
        static double  ppi= 3.14159265358979324;
        static Real deriv;
        sc=std::pow(2.0*ppi,0.5);

        if(idx==1)
            {
                aa=exp(-0.5*std::pow(limit[1],2));
                xx=(limit[3]-sigmarho[2]*limit[1])/std::pow((1.0-std::pow(sigmarho[2],2)),0.5);
                yy=(limit[2]-sigmarho[1]*limit[1])/std::pow((1.0-std::pow(sigmarho[1],2)),0.5);
                rho=(sigmarho[3]-sigmarho[1]*sigmarho[2])/std::pow((1.0-sigmarho[1]*sigmarho[1])*(1.0-sigmarho[2]*sigmarho[2]),0.5);
                deriv=aa*ND2(-xx,-yy,rho)/sc;
            }
        else
            {
                if(idx==2)
                    {
                        aa=exp(-0.5*limit[2]*limit[2]);
                        xx=(limit[1]-sigmarho[1]*limit[2])/std::pow((1.0-std::pow(sigmarho[1],2)),0.5);
                        yy=(limit[3]-sigmarho[3]*limit[2])/std::pow((1.0-std::pow(sigmarho[3],2)),0.5);
                        rho=(sigmarho[2]-sigmarho[1]*sigmarho[3])/ \
                            std::pow((1.0-sigmarho[1]*sigmarho[1])*(1.0-sigmarho[3]*sigmarho[3]),0.5);
                        deriv=aa*ND2(-xx,-yy,rho)/sc;
                    }
                else
                    {
                        //!!! idx=3
                        aa=exp(-0.5*limit[3]*limit[3]);

                        xx=(limit[1]-sigmarho[2]*limit[3])/std::pow((1.0-std::pow(sigmarho[2],2)),0.5);
                        yy=(limit[2]-sigmarho[3]*limit[3])/std::pow((1.0-std::pow(sigmarho[3],2)),0.5);
                        rho=(sigmarho[1]-sigmarho[2]*sigmarho[3])/ \
                            std::pow((1.0-sigmarho[2]*sigmarho[2])*(1.0-sigmarho[3]*sigmarho[3]),0.5);
                        deriv=aa*ND2(-xx,-yy,rho)/sc;
                    }

            }
        return(deriv);
    }


    Real BVTL(int NU, Real DH, Real DK, Real RRR );
    Real TVTMFN(Real X, Real H1, Real H2, Real H3,
                  Real R23, Real RUA, Real RUB, Real AR,
                  Real RUC, int NUC);
    Real ADONET(Real ZRO,Real ONE,Real EPS,
                  Real(*TVTMFN)(Real X, Real H1, Real H2,
                                  Real H3, Real R23, Real RUA,
                                  Real RUB, Real AR, Real RUC,
                                  int NUC));

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

        static Real result;
        static Real  ONE=1.0, ZRO=0.0, EPS,  TVT;
        static Real PT, R12, R13;
        static double ppi= 3.14159265358979324;
        EPS = max( 1.e-14, epsi );
        PT=ppi/2.0;

        NUC = NU;
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
                TVT = TVT + ADONET(ZRO, ONE, EPS, TVTMFN) / (4.0 * PT);
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

        static Real R12=0.0, RR2=0, R13=0.0, RR3=0.0, R=0.0, RR=0.0;
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
        static Real PT, EE;
        PT = 1.57079632679489661923132169163975;
        EE = std::pow(( PT - fabs(X) ),2);

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

    Real KRNRDT(Real, Real,
                  Real(*TVTMFN)(Real, Real, Real, Real,
                                  Real, Real, Real, Real, Real, int),
                  Real& );

    Real ADONET(Real A,Real B, Real TOL,Real(*TVTMFN)(Real X, Real H1, Real H2, Real H3, Real R23, Real RUA, Real RUB, Real AR, Real RUC, int NUC)){
        //
        //     One Dimensional Globally Adaptive Integration Function
        //
        int NL=100, I, IM, IP;
        static Real EI[101], AI[101], BI[101], FI[101], FIN;
        static Real result,ERR;


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
                FI[IP] = KRNRDT( AI[IP], BI[IP], *TVTMFN, EI[IP] );
                FI[IM] = KRNRDT( AI[IM], BI[IM], *TVTMFN, EI[IM] );

                ERR = 0.0;
                FIN = 0.0;
                for(I = 1; I<=IM; I++)
                    {
                        if( EI[I] > EI[IP]) IP = I;
                        FIN = FIN + FI[I];
                        ERR = ERR + EI[I]*EI[I];
                    }
                ERR = std::pow( ERR,0.5 );
            }
        result=FIN;
        //   ADONET = FIN
        return(result);
    }
    //

    Real KRNRDT(Real A, Real B,Real(*TVTMFN)(Real X, Real H1, Real H2, Real H3, Real R23, Real RUA, Real RUB, Real AR, Real RUC, int NUC),Real& ERR ){

        //
        //     Kronrod Rule
        //
        static Real  T, CEN, FC, WID, RESG, RESK;

        static Real result;
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

        static Real  WG[7], WGK[13], XGK[13];

        WG[1]= 0.2729250867779007;
        WG[2]=0.05566856711617449;
        WG[3]=0.1255803694649048;
        WG[4]=0.1862902109277352;
        WG[5]= 0.2331937645919914;
        WG[6]= 0.2628045445102478;
        //
        XGK[1]= 0.0000000000000000;
        XGK[2]= 0.9963696138895427;
        XGK[3]= 0.9782286581460570;
        XGK[4]= 0.9416771085780681;
        XGK[5]= 0.8870625997680953;
        XGK[6]= 0.8160574566562211;
        XGK[7]= 0.7301520055740492;
        XGK[8]= 0.6305995201619651;
        XGK[9]= 0.5190961292068118;
        XGK[10]= 0.3979441409523776;
        XGK[11]= 0.2695431559523450;
        XGK[12]= 0.1361130007993617;
        //
        WGK[1]=0.1365777947111183;
        WGK[2]=0.9765441045961290e-02;
        WGK[3]=0.2715655468210443e-01;
        WGK[4]=0.4582937856442671e-01;
        WGK[5]=0.6309742475037484e-01;
        WGK[6]=0.7866457193222764e-01;
        WGK[7]=0.9295309859690074e-01;
        WGK[8]=0.1058720744813894;
        WGK[9]=0.1167395024610472;
        WGK[10]=0.1251587991003195;
        WGK[11]=0.1312806842298057;
        WGK[12]=0.1351935727998845;
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

        FC = TVTMFN(CEN,H1, H2,  H3, R23, RUA, RUB, AR, RUC, NUC);

        RESG = FC*WG[0+1];
        RESK = FC*WGK[0+1];

        for (J = 1; J<= N; J++)
            {
                T = WID*XGK[J+1];
                FC = TVTMFN(CEN-T,H1, H2,  H3, R23, RUA, RUB, AR, RUC, NUC )+TVTMFN(CEN+T,H1, H2,  H3, R23, RUA, RUB, AR, RUC, NUC );
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
        static int J;
        static Real  ZRO=0.0, ONE=1.0;
        static Real  CSSTHE, SNTHE, POLYN, TT, TS, RN;
        static Real result;


        if ( NU < 1 ) result= PHID( T );
        else if ( NU == 1 ) result = ( 1 + 2.0*atan(T)/PI )/2.0;
        else if ( NU == 2 ) result = ( 1 + T/std::pow(( 2.0 + T*T),0.5))/2.0;
        else
            {
                TT = T*T;
                CSSTHE = 1/( 1 + TT/double(NU) );
                POLYN = 1;
                for( J = NU-2; J>= 2; J=J-2)
                    {
                        POLYN = 1.0 + ( J - 1.0 )*CSSTHE*POLYN/(double)J;
                    }
                if ((NU-2*int(NU/2) ) == 1 )
                    {
                        RN = NU;
                        TS = T/std::pow(RN,0.5);
                        result = ( 1.0 + 2.0*( atan(TS) + TS*CSSTHE*POLYN )/PI )/2.0;
                    }
                else
                    {
                        SNTHE = T/std::pow(( NU + TT ),0.5);
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
        static int  J, HS, KS;
        static Real  TPI, ORS, HRK, KRH, BVT, SNU;
        static Real  GMPH, GMPK, XNKH, XNHK, QHRK, HKN, HPK, HKRN;
        static Real  BTNCKH, BTNCHK, BTPDKH, BTPDHK, ONE, EPS;
        static Real result;
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
                TPI = 2.0*PI;
                SNU = (double)NU;
                SNU = std::pow(SNU,0.5);
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

                HS =(int)SIGN( ONE, DH - R*DK );
                KS =(int)SIGN( ONE, DK - R*DH );
                if((NU-2*(int)(NU/2))==0 )
                    {
                        BVT = atan2( std::pow(ORS,0.5), -R )/TPI;
                        GMPH = DH/std::pow( 16*( NU + DH*DH ),0.5 );
                        GMPK = DK/std::pow( 16*( NU + DK*DK),0.5);
                        BTNCKH = 2*atan2( std::pow( XNKH,0.5 ), std::pow(( 1-XNKH),0.5) )/PI;
                        BTPDKH = 2*std::pow( XNKH*( 1 - XNKH ),0.5 )/PI;
                        BTNCHK = 2*atan2( std::pow( XNHK,0.5 ), std::pow((1 - XNHK),0.5) )/PI;
                        BTPDHK = 2*std::pow( XNHK*( 1 - XNHK ),0.5 )/PI;
                        for( J = 1; J<= NU/2;J++)
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
                        QHRK = std::pow((DH*DH + DK*DK - 2*R*DH*DK + NU*ORS),0.5 ) ;
                        HKRN = DH*DK + R*NU ;
                        HKN = DH*DK - NU;
                        HPK = DH + DK;
                        BVT = atan2( -SNU*( HKN*QHRK + HPK*HKRN ),HKN*HKRN-NU*HPK*QHRK )/TPI;
                        if ( BVT < -EPS ) BVT = BVT + 1;
                        GMPH = DH/( TPI*SNU*( 1 + DH*DH/NU ) );
                        GMPK = DK/( TPI*SNU*( 1 + DK*DK/NU ) );
                        BTNCKH = std::pow( XNKH,0.5 );
                        BTPDKH = BTNCKH;
                        BTNCHK = std::pow( XNHK,0.5 );
                        BTPDHK = BTNCHK;
                        for( J = 1;J<= ( NU - 1 )/2; J++)
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
          static Real DT, FT, BT,result;

          result = 0.0;
          DT = RR*( RR - std::pow(( RA - RB ),2) - 2*RA*RB*( 1 - R ) );
          if( DT > 0 ) {
              BT = ( BC*RR + BA*( R*RB - RA ) + BB*( R*RA -RB ) )/std::pow(DT,0.5);
              FT = std::pow(( BA - R*BB ),0.5)/RR + BB*BB;
              if( NUC<1 ) {
                  if ( (BT > -10) && (FT <100) ) {
                      result = exp( -FT/2 );
                      if ( BT <10 ) result= result*PHID(BT);
                  } else {
                      FT = std::pow((1 + FT/NUC),0.5);
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
        static double TWOPI = 6.283185307179586;
        static Real result, DK, DH, R;
        static int I, IS, LG, NG;

        static Real XL[11][4], WL[11][4], AS, AA, BB, C, D, RS, XS, BVN;
        static Real SN, ASR, H, K, BS, HS, HK;
        //  Gauss Legendre Points and Weights, N =  6
        //  DATA ( W(I,1), X(I,1), I = 1,3) /
        WL[1][1]=0.1713244923791705;
        XL[1][1]=-0.9324695142031522;
        WL[2][1]= 0.3607615730481384;
        XL[2][1]=-0.6612093864662647;
        WL[3][1]= 0.4679139345726904;
        XL[3][1]=-0.2386191860831970;

        //  Gauss Legendre Points and Weights, N = 12
        //  DATA ( W(I,2), X(I,2), I = 1,6) /
        WL[1][2]=0.4717533638651177e-01;
        XL[1][2]=-0.9815606342467191;
        WL[2][2]=  0.1069393259953183;
        XL[2][2]=-0.9041172563704750;
        WL[3][2]=  0.1600783285433464;
        XL[3][2]=-0.7699026741943050;
        WL[4][2]=  0.2031674267230659;
        XL[4][2]=-0.5873179542866171;
        WL[5][2]=  0.2334925365383547;
        XL[5][2]=-0.3678314989981802;
        WL[6][2] = 0.2491470458134029;
        XL[6][2]=-0.1252334085114692;

        //  Gauss Legendre Points and Weights, N = 20
        //  DATA ( W(I,3), X(I,3), I = 1, 10 ) /
        WL[1][3]=0.1761400713915212e-01;
        XL[1][3]=-0.9931285991850949;
        WL[2][3]=0.4060142980038694e-01;
        XL[2][3]=-0.9639719272779138;
        WL[3][3]=0.6267204833410906e-01;
        XL[3][3]=-0.9122344282513259;
        WL[4][3]=0.8327674157670475e-01;
        XL[4][3]=-0.8391169718222188;
        WL[5][3]=0.1019301198172404;
        XL[5][3]=-0.7463319064601508;
        WL[6][3]=0.1181945319615184;
        XL[6][3]=-0.6360536807265150;
        WL[7][3]=0.1316886384491766;
        XL[7][3]=-0.5108670019508271;
        WL[8][3]=0.1420961093183821;
        XL[8][3]=-0.3737060887154196;
        WL[9][3]=0.1491729864726037;
        XL[9][3]=-0.2277858511416451;
        WL[10][3]=0.1527533871307259;
        XL[10][3]=-0.7652652113349733e-01;

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
                BVN = BVN*ASR/( 2*TWOPI );

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
                    AA = std::pow(AS,0.5);

                    BS = std::pow(( H - K ),2);
                    C = ( 4 - HK )/8 ;
                    D = ( 12 - HK )/16;
                    ASR = -( BS/AS + HK )/2;
                    if( ASR > -100 ) BVN = AA*exp(ASR)*( 1 - C*( BS - AS )*( 1 - D*BS/5 )/3 + C*D*AS*AS/5 );
                    if( -HK<100 ){
                        BB = std::pow(BS,0.5);
                        BVN = BVN - exp( -HK/2 )*std::pow(TWOPI,0.5)*PHID(-BB/AA)*BB*( 1 - C*BS*( 1 - D*BS/5 )/3 );
                    }
                    AA = AA/2   ;
                    for (I = 1; I<= LG;I++){
                        for( IS = -1; IS<=1; IS=IS+2){
                            XS =std::pow( ( AA*(  IS*XL[I][NG] + 1 ) ),2)  ;
                            RS = std::pow( (1 - XS),2 );
                            ASR = -( BS/XS + HK )/2;
                            if ( ASR > -100 ) {

                                BVN = BVN + AA*WL[I][NG]*exp( ASR )*(exp( -HK*( 1 - RS )/( 2*( 1 + RS ) ) )/RS- ( 1 + C*XS*( 1 + D*XS ) ) );


                            }
                        }
                    }
                    BVN = -BVN/TWOPI;
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
        ext::shared_ptr<GeneralizedBlackScholesProcess> process, Natural order, bool zeroGamma)
    : process_(std::move(process)), order_(order), zeroGamma_(zeroGamma) {
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

        int igm = zeroGamma_ ? 0 : 1;
        
        results_.value = BarrierUPD(kprice, stock, hbarr,
                                    tauMin, tauMax, order_, igm,
                                    integr_adapter(process_),
                                    integalpha_adapter(process_),
                                    integs_adapter(process_),
                                    alpha_adapter(process_),
                                    sigmaq_adapter(process_));
    }

}

