/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Roland Lichters

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

/*! \file studenttdistribution.hpp
    \brief Student's t-distribution
*/

#ifndef quantlib_student_t_distribution_hpp
#define quantlib_student_t_distribution_hpp

#include <ql/errors.hpp>
#include <ql/types.hpp>
#include <functional>

namespace QuantLib {

    //! Student t-distribution
    /*! Probability density function for \f$ n \f$ degrees of freedom
        (see mathworld.wolfram.com or wikipedia.org):
        \f[
        f(x) = \frac {\Gamma\left(\frac{n+1}{2}\right)} {\sqrt{n\pi}
        \, \Gamma\left(\frac{n}{2}\right)}\:
        \frac {1} {\left(1+\frac{x^2}{n}\right)^{(n+1)/2}}
        \f]
    */
    class StudentDistribution {
      public:
        typedef Real argument_type;
        typedef Real result_type;

        StudentDistribution(Integer n) : n_(n) {
            QL_REQUIRE(n > 0, "invalid parameter for t-distribution");
        }
        Real operator()(Real x) const;
      private:
        Integer n_;
    };

    //! Cumulative Student t-distribution
    /*! Cumulative distribution function for \f$ n \f$ degrees of freedom
        (see mathworld.wolfram.com):
        \f[
        F(x) = \int_{-\infty}^x\,f(y)\,dy
        = \frac{1}{2}\,
        +\,\frac{1}{2}\,sgn(x)\,
        \left[ I\left(1,\frac{n}{2},\frac{1}{2}\right)
        - I\left(\frac{n}{n+y^2}, \frac{n}{2},\frac{1}{2}\right)\right]
        \f]
        where \f$ I(z; a, b) \f$ is the regularized incomplete beta function.
    */
    class CumulativeStudentDistribution {
      public:
        typedef Real argument_type;
        typedef Real result_type;

        CumulativeStudentDistribution(Integer n) : n_(n) {
            QL_REQUIRE(n > 0, "invalid parameter for t-distribution");
        }
        Real operator()(Real x) const;
      private:
        Integer n_;
    };

    //! Inverse cumulative Student t-distribution
    /*! \todo Find/implement an efficient algorithm for evaluating the
              cumulative Student t-distribution, replacing the Newton
              iteration
    */
    class InverseCumulativeStudent {
      public:
        typedef Real argument_type;
        typedef Real result_type;

        InverseCumulativeStudent(Integer n,
                                 Real accuracy = 1e-6,
                                 Size maxIterations = 50)
        : d_(n), f_(n), accuracy_(accuracy),
          maxIterations_(maxIterations) {}
        Real operator()(Real x) const;
      private:
        StudentDistribution d_;
        CumulativeStudentDistribution f_;
        Real accuracy_;
        Size maxIterations_;
    };

}

#endif


#ifndef id_8e73392a9d0a8cd800527ee300cc171d
#define id_8e73392a9d0a8cd800527ee300cc171d
inline bool test_8e73392a9d0a8cd800527ee300cc171d(const int* i) {
    return i != nullptr;
}
#endif
