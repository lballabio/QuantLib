/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2012 Peter Caspers

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

/*! \file adaptiverungekutta.hpp
    \brief Runge-Kutta ODE integration

    Runge Kutta method with adaptive stepsize as described in
    Numerical Recipes in C, Chapter 16.2
*/

#ifndef quantlib_adaptive_runge_kutta_hpp
#define quantlib_adaptive_runge_kutta_hpp

#include <ql/types.hpp>
#include <ql/errors.hpp>
#include <ql/functional.hpp>
#include <vector>
#include <cmath>
#include <complex>

namespace QuantLib {

    template <class T = Real>
    class AdaptiveRungeKutta {
      public:
        typedef std::function<std::vector<T>(const Real, const std::vector<T>&)> OdeFct;
        typedef std::function<T(const Real, const T)> OdeFct1d;

        /*! The class is constructed with the following inputs:
            - eps       prescribed error for the solution
            - h1        start step size
            - hmin      smallest step size allowed
        */

        AdaptiveRungeKutta(const Real eps = 1.0e-6, const Real h1 = 1.0e-4, const Real hmin = 0.0)
        : eps_(eps), h1_(h1), hmin_(hmin), b31(3.0 / 40.0), b32(9.0 / 40.0), b51(-11.0 / 54.0),
          b53(-70.0 / 27.0), b54(35.0 / 27.0), b61(1631.0 / 55296.0), b62(175.0 / 512.0),
          b63(575.0 / 13824.0), b64(44275.0 / 110592.0), b65(253.0 / 4096.0), c1(37.0 / 378.0),
          c3(250.0 / 621.0), c4(125.0 / 594.0), c6(512.0 / 1771.0), dc1(c1 - 2825.0 / 27648.0),
          dc3(c3 - 18575.0 / 48384.0), dc4(c4 - 13525.0 / 55296.0), dc5(-277.0 / 14336.0),
          dc6(c6 - 0.25) {}

        /*! Integrate the ode from \f$ x1 \f$ to \f$ x2 \f$ with
            initial value condition \f$ f(x1)=y1 \f$.

            The ode is given by a function \f$ F: R \times K^n
            \rightarrow K^n \f$ as \f$ f'(x) = F(x,f(x)) \f$, $K=R,
            C$ */
        std::vector<T> operator()(const OdeFct& ode, const std::vector<T>& y1, Real x1, Real x2);
        T operator()(const OdeFct1d& ode, T y1, Real x1, Real x2);

      private:
        void rkqs(std::vector<T>& y,
                  const std::vector<T>& dydx,
                  Real& x,
                  Real htry,
                  Real eps,
                  const std::vector<Real>& yScale,
                  Real& hdid,
                  Real& hnext,
                  const OdeFct& derivs);
        void rkck(const std::vector<T>& y,
                  const std::vector<T>& dydx,
                  Real x,
                  Real h,
                  std::vector<T>& yout,
                  std::vector<T>& yerr,
                  const OdeFct& derivs);

        const std::vector<T> yStart_;
        const Real eps_, h1_, hmin_;
        const Real a2 = 0.2, a3 = 0.3, a4 = 0.6, a5 = 1.0, a6 = 0.875, b21 = 0.2, b31, b32,
                   b41 = 0.3, b42 = -0.9, b43 = 1.2, b51, b52 = 2.5, b53, b54, b61, b62, b63, b64,
                   b65, c1, c3, c4, c6, dc1, dc3, dc4, dc5, dc6;
        const double ADAPTIVERK_MAXSTP = 10000, ADAPTIVERK_TINY = 1.0E-30, ADAPTIVERK_SAFETY = 0.9,
                     ADAPTIVERK_PGROW = -0.2, ADAPTIVERK_PSHRINK = -0.25,
                     ADAPTIVERK_ERRCON = 1.89E-4;
    };



    template<class T>
    std::vector<T> AdaptiveRungeKutta<T>::operator()(const OdeFct& ode,
                                                     const std::vector<T>& y1,
                                                     const Real x1,
                                                     const Real x2) {
        Size n = y1.size();
        std::vector<T> y(y1);
        std::vector<Real> yScale(n);
        Real x = x1;
        Real h = h1_* (x1<=x2 ? 1 : -1);
        Real hnext,hdid;

        for (Size nstp=1; nstp<=ADAPTIVERK_MAXSTP; nstp++) {
            std::vector<T> dydx=ode(x,y);
            for (Size i=0;i<n;i++)
                yScale[i] = std::abs(y[i])+std::abs(dydx[i]*h)+ADAPTIVERK_TINY;
            if ((x+h-x2)*(x+h-x1) > 0.0)
                h=x2-x;
            rkqs(y,dydx,x,h,eps_,yScale,hdid,hnext,ode);

            if ((x-x2)*(x2-x1) >= 0.0)
                return y;

            if (std::fabs(hnext) <= hmin_)
                QL_FAIL("Step size (" << hnext << ") too small ("
                        << hmin_ << " min) in AdaptiveRungeKutta");
            h=hnext;
        }
        QL_FAIL("Too many steps (" << ADAPTIVERK_MAXSTP
                << ") in AdaptiveRungeKutta");
    }

    namespace detail {

        template <class T>
        struct OdeFctWrapper {
            typedef typename AdaptiveRungeKutta<T>::OdeFct1d OdeFct1d;
            explicit OdeFctWrapper(const OdeFct1d& ode1d)
            : ode1d_(ode1d) {}
            std::vector<T> operator()(const Real x, const std::vector<T>& y) {
                std::vector<T> res(1,ode1d_(x,y[0]));
                return res;
            }
            const OdeFct1d& ode1d_;
        };

    }

    template<class T>
    T AdaptiveRungeKutta<T>::operator()(const OdeFct1d& ode,
                                        const T y1,
                                        const Real x1,
                                        const Real x2) {
        return operator()(detail::OdeFctWrapper<T>(ode),
                          std::vector<T>(1,y1),x1,x2)[0];
    }

    template<class T>
    void AdaptiveRungeKutta<T>::rkqs(std::vector<T>& y,
                                     const std::vector<T>& dydx,
                                     Real& x,
                                     const Real htry,
                                     const Real eps,
                                     const std::vector<Real>& yScale,
                                     Real& hdid,
                                     Real& hnext,
                                     const OdeFct& derivs) {
        Size n=y.size();
        Real errmax,xnew;
        std::vector<T> yerr(n),ytemp(n);

        Real h=htry;

        for(;;) {
            rkck(y,dydx,x,h,ytemp,yerr,derivs);
            errmax=0.0;
            for (Size i=0;i<n;i++)
                errmax=std::max(errmax,std::abs(yerr[i]/yScale[i]));
            errmax/=eps;
            if (errmax>1.0) {
                Real htemp1 = ADAPTIVERK_SAFETY*h*std::pow(errmax,ADAPTIVERK_PSHRINK);
                Real htemp2 = h / 10;
                // These would be std::min and std::max, of course,
                // but VC++14 had problems inlining them and caused
                // the wrong results to be calculated.  The problem
                // seems to be fixed in update 3, but let's keep this
                // implementation for compatibility.
                Real max_positive = htemp1 > htemp2 ? htemp1 : htemp2;
                Real max_negative = htemp1 < htemp2 ? htemp1 : htemp2;
                h = ((h >= 0.0) ? max_positive : max_negative);
                xnew=x+h;
                if (xnew==x)
                    QL_FAIL("Stepsize underflow (" << h << " at x = " << x
                            << ") in AdaptiveRungeKutta::rkqs");
                continue;
            } else {
                if (errmax>ADAPTIVERK_ERRCON)
                    hnext=ADAPTIVERK_SAFETY*h*std::pow(errmax,ADAPTIVERK_PGROW);
                else
                    hnext=5.0*h;
                x+=(hdid=h);
                for (Size i=0;i<n;i++)
                    y[i]=ytemp[i];
                break;
            }
        }
    }

    template <class T>
    void AdaptiveRungeKutta<T>::rkck(const std::vector<T>& y,
                                     const std::vector<T>& dydx,
                                     Real x,
                                     const Real h,
                                     std::vector<T>& yout,
                                     std::vector<T> &yerr,
                                     const OdeFct& derivs) {

        Size n=y.size();
        std::vector<T> ak2(n),ak3(n),ak4(n),ak5(n),ak6(n),ytemp(n);

        // first step
        for (Size i=0;i<n;i++)
            ytemp[i]=y[i]+b21*h*dydx[i];

        // second step
        ak2=derivs(x+a2*h,ytemp);
        for (Size i=0;i<n;i++)
            ytemp[i]=y[i]+h*(b31*dydx[i]+b32*ak2[i]);

        // third step
        ak3=derivs(x+a3*h,ytemp);
        for (Size i=0;i<n;i++)
            ytemp[i]=y[i]+h*(b41*dydx[i]+b42*ak2[i]+b43*ak3[i]);

        // fourth step
        ak4=derivs(x+a4*h,ytemp);
        for (Size i=0;i<n;i++)
            ytemp[i]=y[i]+h*(b51*dydx[i]+b52*ak2[i]+b53*ak3[i]+b54*ak4[i]);

        // fifth step
        ak5=derivs(x+a5*h,ytemp);
        for (Size i=0;i<n;i++)
            ytemp[i]=y[i]+h*(b61*dydx[i]+b62*ak2[i]+b63*ak3[i]+b64*ak4[i]+b65*ak5[i]);

        // sixth step
        ak6=derivs(x+a6*h,ytemp);
        for (Size i=0;i<n;i++) {
            yout[i]=y[i]+h*(c1*dydx[i]+c3*ak3[i]+c4*ak4[i]+c6*ak6[i]);
            yerr[i]=h*(dc1*dydx[i]+dc3*ak3[i]+dc4*ak4[i]+dc5*ak5[i]+dc6*ak6[i]);
        }
    }

}

#endif
