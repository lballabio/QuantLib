/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Lorella Fatone
 Copyright (C) 2008 Francesca Mariani
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
#include <ql/experimental/varianceoption/integralhestonvarianceoptionengine.hpp>
#include <functional>
#include <complex>
#include <utility>
#include <memory>

namespace QuantLib {

    namespace {

    /*
     *****************************************************************
     **
     ** Parameters defining the initial condition of the Heston model
     ** and the European call option
     **
     *****************************************************************
     */
    /*
     *****************************************************************
     ** Assign: v0, eprice, tau, rtax
     ******************************************************************
     ******************************************************************
     **     v0: initial variance
     ** eprice: realized variance strike price
     **    tau: time to maturity
     *    rtax: risk free interest rate
     ****************************************************************
     */

    typedef std::complex<Real> Complex;

    Real IvopOneDim(Real eps, Real chi, Real theta, Real /*rho*/,
                      Real v0, Real eprice, Time tau, Real rtax)
    {
        Real ss=0.0;
        std::unique_ptr<double[]> xiv(new double[2048*2048+1]);
        double nris=0.0;
        int j=0,mm=0;
        double pi=0,pi2=0;
        double dstep=0;
        Real option=0, impart=0;

        std::unique_ptr<Complex[]> ff(new Complex[2048*2048]);
        Complex xi;
        Complex ui,beta,zita,gamma,csum,vero;
        Complex contrib, caux, caux1,caux2,caux3;

        ui=Complex(0.0,1.0);

        /*
         **********************************************************
         **   i0: initial integrated variance i0=0
         **********************************************************
         */
        Real i0=0.0;
        //s=2.0*chi*theta/(eps*eps)-1.0;

        //s=s+1;

        /*
         *************************************************
         ** Start integration procedure
         *************************************************
         */

        pi= 3.14159265358979324;
        pi2=2.0*pi;
        Real s=2.0*chi*theta/(eps*eps)-1.0;
        /*
         ****************************************
         ** Note that s must be greater than zero
         ****************************************
         */

        if(s<=0)
        {
            QL_FAIL("this parameter must be greater than zero-> " << s);
        }

        ss=s+1;

        /*
         *************************************************
         ** Start integration procedure
         *************************************************

         **************************************************************
         ** The oscillatory integral that approximates the price of
         ** the realized variance option is computed using the method
         ** proposed by Bailey, Swarztrauber in the paper published in
         ** Siam Journal on Scientific Computing Vol 15(5) 1994
         ** p. 1105-1110
         **************************************************************

         **************************************************************
         ** dstep: real number, generally a power of two, that must be
         **        assigned to determine the grid of
         **        integration. Hint: dstep=256 or 512 (dstep<=2048)
         **************************************************************
         */
        dstep=256.0;
        nris=std::sqrt(pi2)/dstep;
        mm=(int)(pi2/(nris*nris));

        /*
         ******************************************
         **  Definition of the integration grid  **
         ******************************************
         */
        for (j=0;j<=mm-1;j++)
        {
            xiv[j+1]=(j-mm/2.0)*nris;
        }

        for (j=0;j<=mm-1;j++)
        {
            xi=xiv[j+1];
            caux=chi*chi;
            caux1=2.0*eps*eps;
            caux1=caux1*xi;
            caux1=caux1*ui;
            caux2=caux1+caux;

            zita=0.5*std::sqrt(caux2);

            caux1=std::exp(-2.0*tau*zita);

            beta=0.5*chi+zita;
            beta=beta+caux1*(zita-0.5*chi);
            gamma=1.0-caux1;

            caux=-ss*tau;
            caux2=caux*(zita-0.5*chi);
            caux=ss*std::log(2.0*(zita/beta));
            caux3=-v0*ui*xi*(gamma/beta);
            caux=caux+caux3;
            caux=caux+caux2;

            ff[j+1]=std::exp(caux);
            if(std::sqrt(std::imag(xi)*std::imag(xi)+std::real(xi)*std::real(xi))>1.e-06)
            {
                contrib=-eprice/(ui*xi);
                caux=ui*xi;
                caux=caux*eprice;
                caux=std::exp(caux);
                caux=caux-1.0;
                caux2=ui*xi*ui*xi;
                contrib=contrib+caux/caux2;
            }
            else
            {
                contrib=eprice*eprice*0.5;
            }
            ff[j+1]=ff[j+1]*contrib;
        }
        csum=0.0;
        for (j=0;j<=mm-1;j++)
        {
            caux=std::pow(-1.0,j);
            caux2=-2.0*pi*(double)mm*(double)j*0.5/(double)mm;
            caux3=ui*caux2;
            csum=csum+ff[j+1]*caux*std::exp(caux3);
        }
        csum=csum*std::sqrt(std::pow(-1.0,mm))*nris/pi2;
        vero=i0-eprice+theta*tau+(1.0-std::exp(-chi*tau))*(v0-theta)/chi;
        csum=csum+vero;
        option=std::exp(-rtax*tau)*std::real(csum);
        impart=std::imag(csum);
        QL_ENSURE(impart <= 1e-12,
                  "imaginary part option (must be zero) = " << impart);
        return option;
    }



    Real IvopTwoDim(Real eps, Real chi, Real theta, Real /*rho*/,
                    Real v0, Time tau, Real rtax,
                    const std::function<Real(Real)>& payoff) {

        Real ss=0.0;
        std::unique_ptr<double[]> xiv(new double[2048*2048+1]);
        std::unique_ptr<double[]> ivet(new double[2048 * 2048 + 1]);
        double nris=0.0;
        int j=0,mm=0,k=0;
        double pi=0,pi2=0;

        double dstep=0;
        Real ip=0;
        Real payoffval=0;
        Real option=0/*, impart=0*/;

        Real sumr=0;//,sumi=0;
        Complex dxi,z;

        std::unique_ptr<Complex[]> ff(new Complex[2048*2048]);
        Complex xi;
        Complex ui,beta,zita,gamma,csum;
        Complex caux,caux1,caux2,caux3;

        ui=Complex(0.0,1.0);

        /*
         **********************************************************
         **   i0: initial integrated variance i0=0
         **********************************************************
         */
        Real i0=0.0;

        /*
         *************************************************
         ** Start integration procedure
         *************************************************
         */

        pi= 3.14159265358979324;
        pi2=2.0*pi;

        Real s=2.0*chi*theta/(eps*eps)-1.0;
        /*
         ****************************************
         ** Note that s must be greater than zero
         ****************************************
         */

        if(s<=0)
        {
            QL_FAIL("this parameter must be greater than zero-> " << s);
        }

        ss=s+1;

        /*
         *************************************************
         ** Start integration procedure
         *************************************************

         **************************************************************
         ** The oscillatory integral that approximates the price of
         ** the realized variance option is computed using the method
         ** proposed by Bailey, Swarztrauber in the paper published in
         ** Siam Journal on Scientific Computing Vol 15(5) 1994
         ** p. 1105-1110
         **************************************************************

         **************************************************************
         ** dstep: real number, generally a power of two that must be
         **        assigned to determine the grid of
         **        integration. Hint: dstep=256 or 512 (dstep<=2048)
         **************************************************************
         */
        dstep=64.0;
        nris=std::sqrt(pi2)/dstep;
        mm=(int)(pi2/(nris*nris));

        /*
         ******************************************
         **  Definition of the integration grid  **
         ******************************************
         */

        for (j=0;j<=mm-1;j++)
        {
            xiv[j+1]=(j-mm/2.0)*nris;
            ivet[j+1]=(j-mm/2.0)*pi2/((double)mm*nris);
        }

        for (j=0;j<=mm-1;j++)
        {
            xi=xiv[j+1];

            caux=chi*chi;
            caux1=2.0*eps*eps;
            caux1=caux1*xi;
            caux1=caux1*ui;
            caux2=caux1+caux;

            zita=0.5*std::sqrt(caux2);
            caux1=std::exp(-2.0*tau*zita);

            beta=0.5*chi+zita;
            beta=beta+caux1*(zita-0.5*chi);

            gamma=1.0-caux1;

            caux=-ss*tau;
            caux2=caux*(zita-0.5*chi);
            caux=ss*std::log(2.0*(zita/beta));
            caux3=-v0*ui*xi*(gamma/beta);
            caux=caux+caux3;
            caux=caux+caux2;
            ff[j+1]=std::exp(caux);
        }

        sumr=0.0;
        //sumi=0.0;
        for (k=0;k<=mm-1;k++)
        {
            ip=i0-ivet[k+1];
            payoffval=payoff(ip);

            dxi=2.0*pi*(double)k/(double)mm*ui;
            csum=0.0;
            for (j=0;j<=mm-1;j++)
            {
                z=-(double)j*dxi;
                caux=std::pow(-1.0,j);
                csum=csum+ff[j+1]*caux*std::exp(z);
            }
            csum=csum*std::pow(-1.0,k)*nris/pi2;

            sumr=sumr+payoffval*std::real(csum);
            //sumi=sumi+payoffval*std::imag(csum);
        }
        sumr=sumr*nris;
        //sumi=sumi*nris;

        option=std::exp(-rtax*tau)*sumr;
        //impart=sumi;
        //QL_ENSURE(impart <= 1e-3,
        //          "imaginary part option (must be close to zero) = " << impart);
        return option;
    }

    struct payoff_adapter {
        ext::shared_ptr<QuantLib::Payoff> payoff;
        explicit payoff_adapter(ext::shared_ptr<QuantLib::Payoff> payoff)
        : payoff(std::move(payoff)) {}
        Real operator()(Real S) const {
            return (*payoff)(S);
        }
    };

    }

    IntegralHestonVarianceOptionEngine::IntegralHestonVarianceOptionEngine(
        ext::shared_ptr<HestonProcess> process)
    : process_(std::move(process)) {
        registerWith(process_);
    }

    void IntegralHestonVarianceOptionEngine::calculate() const {

        QL_REQUIRE(process_->dividendYield().empty(),
                   "this engine does not manage dividend yields");

        Handle<YieldTermStructure> riskFreeRate = process_->riskFreeRate();

        Real epsilon = process_->sigma();
        Real chi = process_->kappa();
        Real theta = process_->theta();
        Real rho = process_->rho();
        Real v0 = process_->v0();

        Time tau = riskFreeRate->dayCounter().yearFraction(
                                        Settings::instance().evaluationDate(),
                                        arguments_.maturityDate);
        Rate r = riskFreeRate->zeroRate(arguments_.maturityDate,
                                        riskFreeRate->dayCounter(),
                                        Continuous);

        ext::shared_ptr<PlainVanillaPayoff> plainPayoff =
            ext::dynamic_pointer_cast<PlainVanillaPayoff>(arguments_.payoff);
        if ((plainPayoff != nullptr) && plainPayoff->optionType() == Option::Call) {
            // a specialization for Call options is available
            Real strike = plainPayoff->strike();
            results_.value = IvopOneDim(epsilon, chi, theta, rho,
                                        v0, strike, tau, r)
                * arguments_.notional;
        } else {
            results_.value = IvopTwoDim(epsilon, chi, theta, rho, v0, tau, r,
                                        payoff_adapter(arguments_.payoff))
                * arguments_.notional;
        }
    }

}

