/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Mark Joshi

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

/*! \file spherecylinder.hpp
    \brief Find closest point of the intersection of a sphere and cylinder to a
           given point
*/

#ifndef quantlib_optimization_sphere_cylinder_hpp
#define quantlib_optimization_sphere_cylinder_hpp

#include <ql/types.hpp>
#include <vector>

namespace QuantLib {

    /*! - we are in r^3 sphere centred at O radius r
        - vertical cylinder centred at (alpha,0) radius s
        - Z some point in R3
        - find point on intersection that is closest to Z

        the intersection may be empty!
    */
    class SphereCylinderOptimizer {
      public:
        SphereCylinderOptimizer(Real r,
                                Real s,
                                Real alpha,
                                Real z1,
                                Real z2,
                                Real z3,
                                Real zweight =1.0);
        bool isIntersectionNonEmpty() const;
        void findClosest(Size maxIterations,
                         Real tolerance,
                         Real& y1,
                         Real& y2,
                         Real& y3) const;
        bool findByProjection(
                         Real& y1,
                         Real& y2,
                         Real& y3) const;
      private:
        Real r_, s_, alpha_, z1_, z2_, z3_;
        Real objectiveFunction(Real x2) const;
        Real topValue_;
        Real bottomValue_;
        bool nonEmpty_;
        Real zweight_;
    };

    std::vector<Real> sphereCylinderOptimizerClosest(Real r,
                                                     Real s,
                                                     Real alpha,
                                                     Real z1,
                                                     Real z2,
                                                     Real z3,
                                                     Natural maxIterations,
                                                     Real tolerance,
                                                     Real finalWeight = 1.0);

}

#endif
