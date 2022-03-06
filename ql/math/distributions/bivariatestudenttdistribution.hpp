/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2014 Michal Kaut

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

/*! \file bivariatestudenttdistribution.hpp
    \brief Bivariate Student t-distribution
*/

#ifndef quantlib_bivariate_student_t_distribution_hpp
#define quantlib_bivariate_student_t_distribution_hpp

#include <ql/errors.hpp>
#include <ql/types.hpp>
#include <functional>

namespace QuantLib {

    //! Cumulative Student t-distribution
    /*! Implemented following the formulas from Dunnett, C.W. and
        Sobel, M. (1954). A bivariate generalization of Student
        t-distribution with tables for certain special
        cases. Biometrika 41, 153–169.
    */
    class BivariateCumulativeStudentDistribution {
      public:
        /*! \param n    degrees of freedom
            \param rho  correlation
        */
        BivariateCumulativeStudentDistribution(Natural n,
                                               Real rho);
        Real operator()(Real x, Real y) const;
      private:
        Natural n_;
        Real rho_;
    };

}


#endif


#ifndef id_76b26b1bf467c1654250a4697afcb4e1
#define id_76b26b1bf467c1654250a4697afcb4e1
inline bool test_76b26b1bf467c1654250a4697afcb4e1(int* i) { return i != 0; }
#endif
