
/*
 Copyright (C) 2002, 2003 Roman Gitlin
 Copyright (C) 2003 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file trapezoidintegral.hpp
    \brief integral of a one-dimensional function
*/

#ifndef quantlib_trapezoid_integral_hpp
#define quantlib_trapezoid_integral_hpp

#include <ql/null.hpp>

namespace QuantLib {

    //! Integral of a one-dimensional function
    /*! Given a target accuracy \f$ \epsilon \f$, the integral of
        a function \f$ f \f$ between \f$ a \f$ and \f$ b \f$ is 
        calculated by means of the trapezoid formula
        \f[
        \int_{a}^{b} f \mathrm{d}x = 
        \frac{1}{2} f(x_{0}) + f(x_{1}) + f(x_{2}) + \dots 
        + f(x_{N-1}) + \frac{1}{2} f(x_{N})
        \f]
        where \f$ x_0 = a \f$, \f$ x_N = b \f$, and 
        \f$ x_i = a+i \Delta x \f$ with 
        \f$ \Delta x = (b-a)/N \f$. The number \f$ N \f$ of intervals
        is repeatedly increased until the target accuracy is reached.
    */
    class TrapezoidIntegral {
      public:
        enum Method { Default, MidPoint };
        TrapezoidIntegral(double accuracy, 
                          Method method = Default,
                          Size maxIterations = Null<Size>())
        : accuracy_(accuracy), method_(method), 
          maxIterations_(maxIterations) {}
        template <class F>
        double operator()(const F& f, double a, double b) const {

            if (a == b)
                return 0.0;
            if (a > b)
                return -(*this)(f,b,a);

            // start from the coarsest trapezoid...
            Size N = 1;
            double I = (f(a)+f(b))*(b-a)/2.0, newI;
            // ...and refine it
            Size i = 1;
            do {
                switch (method_) {
                  case MidPoint:
                    newI = midPointIteration(f,a,b,I,N);
                    N *= 3;
                    break;
                  default:
                    newI = defaultIteration(f,a,b,I,N);
                    N *= 2;
                    break;
                }
                // good enough? Also, don't run away immediately
                if (QL_FABS(I-newI) <= accuracy_ && i > 5)
                    // ok, exit
                    return newI;
                // oh well. Another step.
                I = newI;
                i++;
            } while (i < maxIterations_);
            QL_FAIL("max number of iterations reached");
        }
        // calculation parameters
        double accuracy() const { return accuracy_; }
        double& accuracy() { return accuracy_; }
        Method method() const { return method_; }
        Method& method() { return method_; }
        Size maxIterations() const { return maxIterations_; }
        Size& maxIterations() { return maxIterations_; }
      protected:
        double accuracy_;
        Method method_;
        Size maxIterations_;
        template <class F>
        double defaultIteration(const F& f, double a, double b,
                                double I, Size N) const {
            double sum = 0.0;
            double dx = (b-a)/N;
            double x = a + dx/2.0;
            for (Size i=0; i<N; x += dx, ++i) 
                sum += f(x);
            return (I + dx*sum)/2.0;
        }
        template <class F>
        double midPointIteration(const F& f, double a, double b,
                                 double I, Size N) const {
            double sum = 0.0;
            double dx = (b-a)/N;
            double x = a + dx/6.0;
            double D = 2.0*dx/3.0;
            for (Size i=0; i<N; x += dx, ++i) 
                sum += f(x) + f(x+D);
            return (I + dx*sum)/3.0;
        }
    };

}


#endif
