/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Allen Kuo
 Copyright (C) 2010 Alessandro Roveda

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

/*! \file nonlinearfittingmethods.hpp
    \brief nonlinear methods to fit a bond discount function
*/

#ifndef quantlib_nonlinear_fitting_methods_hpp
#define quantlib_nonlinear_fitting_methods_hpp

#include <ql/termstructures/yield/fittedbonddiscountcurve.hpp>
#include <ql/math/bspline.hpp>

namespace QuantLib {

    //! Exponential-splines fitting method
    /*! Fits a discount function to the exponential form
        \f[
        d(t) = \sum_{i=1}^9 c_i \exp^{-kappa i t}
        \f]
        where the constants \f$ c_i \f$ and \f$ \kappa \f$ are to be
        determined.  See:Li, B., E. DeWetering, G. Lucas, R. Brenner
        and A. Shapiro (2001): "Merrill Lynch Exponential Spline
        Model." Merrill Lynch Working Paper

        \warning convergence may be slow
    */
    class ExponentialSplinesFitting
        : public FittedBondDiscountCurve::FittingMethod {
      public:
        ExponentialSplinesFitting(bool constrainAtZero = true);
        std::auto_ptr<FittedBondDiscountCurve::FittingMethod> clone() const;
      private:
        Size size() const;
        DiscountFactor discountFunction(const Array& x, Time t) const;
    };


    //! Nelson-Siegel fitting method
    /*! Fits a discount function to the form
        \f$ d(t) = \exp^{-r t}, \f$ where the zero rate \f$r\f$ is defined as
        \f[
        r \equiv c_0 + (c_0 + c_1)*(1 - exp^{-\kappa*t}/(\kappa t) -
        c_2 exp^{ - \kappa t}.
        \f]
        See: Nelson, C. and A. Siegel (1985): "Parsimonious modeling of yield
        curves for US Treasury bills." NBER Working Paper Series, no 1594.
    */
    class NelsonSiegelFitting
        : public FittedBondDiscountCurve::FittingMethod {
      public:
        NelsonSiegelFitting();
        std::auto_ptr<FittedBondDiscountCurve::FittingMethod> clone() const;
      private:
        Size size() const;
        DiscountFactor discountFunction(const Array& x, Time t) const;
    };


    //! Svensson Fitting method
    /*! Fits a discount function to the form
        \f$ d(t) = \exp^{-r t}, \f$ where the zero rate \f$r\f$ is defined as
        \f[
        r \equiv c_0 + (c_0 + c_1)(\frac {1 - exp^{-\kappa t}}{\kappa t})
        - c_2exp^{ - \kappa t}
        + c_3{(\frac{1 - exp^{-\kappa_1 t}}{\kappa_1 t} -exp^{-\kappa_1 t})}.
        \f]
        See: Svensson, L. (1994). Estimating and interpreting forward
        interest rates: Sweden 1992-4.
        Discussion paper, Centre for Economic Policy Research(1051).
    */
    class SvenssonFitting
        : public FittedBondDiscountCurve::FittingMethod {
      public:
        SvenssonFitting();
        std::auto_ptr<FittedBondDiscountCurve::FittingMethod> clone() const;
      private:
        Size size() const;
        DiscountFactor discountFunction(const Array& x, Time t) const;
    };


    //! CubicSpline B-splines fitting method
    /*! Fits a discount function to a set of cubic B-splines
        \f$ N_{i,3}(t) \f$, i.e.,
        \f[
        d(t) = \sum_{i=0}^{n}  c_i * N_{i,3}(t)
        \f]

        See: McCulloch, J. 1971, "Measuring the Term Structure of
        Interest Rates." Journal of Business, 44: 19-31

        McCulloch, J. 1975, "The tax adjusted yield curve."
        Journal of Finance, XXX811-30

        \warning "The results are extremely sensitive to the number
                  and location of the knot points, and there is no
                  optimal way of selecting them." James, J. and
                  N. Webber, "Interest Rate Modelling" John Wiley,
                  2000, pp. 440.
    */
    class CubicBSplinesFitting
        : public FittedBondDiscountCurve::FittingMethod {
      public:
        CubicBSplinesFitting(const std::vector<Time>& knotVector,
                             bool constrainAtZero = true);
        //! cubic B-spline basis functions
        Real basisFunction(Integer i, Time t) const;
        std::auto_ptr<FittedBondDiscountCurve::FittingMethod> clone() const;
      private:
        Size size() const;
        DiscountFactor discountFunction(const Array& x, Time t) const;
        BSpline splines_;
        Size size_;
        //! N_th basis function coefficient to solve for when d(0)=1
        Natural N_;
    };


    //! Simple polynomial fitting method
    /*  Fits a discount function to the simple polynomial form:
        \f[
        d(t) = \sum_{i=0}^{degree}  c_i * t^{i}
        \f]
        where the constants \f$ c_i \f$ are to be determined.

        This is a simple/crude, but fast and robust, means of fitting
        a yield curve.
    */
    class SimplePolynomialFitting
        : public FittedBondDiscountCurve::FittingMethod {
      public:
        SimplePolynomialFitting(Natural degree,
                                bool constrainAtZero = true);
        std::auto_ptr<FittedBondDiscountCurve::FittingMethod> clone() const;
      private:
        Size size() const;
        DiscountFactor discountFunction(const Array& x, Time t) const;
        Size size_;
    };

}


#endif
