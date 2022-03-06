/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2015 Andres Hernandez

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

/*! \file isotropicrandomwalk.hpp
    \brief Isotropic random walk
*/

#ifndef quantlib_isotropic_random_walk_hpp
#define quantlib_isotropic_random_walk_hpp

#include <ql/math/array.hpp>
#include <ql/math/randomnumbers/mt19937uniformrng.hpp>
#include <ql/mathconstants.hpp>
#include <boost/random/variate_generator.hpp>
#include <utility>

namespace QuantLib {

    //! Isotropic random walk
    /*! A variate is used to draw from a random element of a 
        probability distribution. The draw corresponds to the 
        radius of a d-dimensional sphere. The position on the
        surface of the d-dimensional sphere is randomly chosen
        with all points on the surface having the same probability,
        i.e. all directions are isotropic and the step is randomly
        drawn from the given variate.
    */
    template <class Distribution, class Engine>
    class IsotropicRandomWalk {
      public:
        typedef boost::variate_generator<Engine, Distribution> VariateGenerator;
        IsotropicRandomWalk(const Engine& eng,
                            Distribution dist,
                            Size dim,
                            Array weights = Array(),
                            unsigned long seed = 0)
        : variate_(eng, dist), rng_(seed), weights_(std::move(weights)), dim_(dim) {
            if (weights_.empty())
                weights_ = Array(dim, 1.0);
            else
                QL_REQUIRE(dim_ == weights_.size(), "Invalid weights");
        }
        template <class InputIterator>
        inline void nextReal(InputIterator first) const {
            Real radius = variate_();
            Array::const_iterator weight = weights_.begin();
            if (dim_ > 1) {
                //Isotropic random direction
                Real phi = M_PI*rng_.nextReal();
                for (Size i = 0; i < dim_ - 2; i++) {
                    *first++ = radius*cos(phi)*(*weight++);
                    radius *= sin(phi);
                    phi = M_PI*rng_.nextReal();
                }
                *first++ = radius*cos(2.0*phi)*(*weight++);
                *first = radius*sin(2.0*phi)*(*weight);
            }
            else {
                if (rng_.nextReal() < 0.5)
                    *first = -radius*(*weight);
                else
                    *first = radius*(*weight);
            }
        }
        inline void setDimension(Size dim) { 
            dim_ = dim;
            weights_ = Array(dim, 1.0);
        }
        inline void setDimension(Size dim, const Array& weights) {
            QL_REQUIRE(dim == weights.size(), "Invalid weights");
            dim_ = dim;
            weights_ = weights;
        }
        /*!
        The isotropic random walk will not adjust its draw to be within the lower and upper bounds,
        but if the limits are provided, they are used to rescale the sphere so as to make it to an
        ellipsoid, with different radius in different dimensions.
        */
        inline void setDimension(Size dim,
            const Array& lowerBound, const Array& upperBound) {
            QL_REQUIRE(dim == lowerBound.size(),
                "Incompatible dimension and lower bound");
            QL_REQUIRE(dim == upperBound.size(),
                "Incompatible dimension and upper bound");
            //Find largest bound
            Array bounds = upperBound - lowerBound;
            Real maxBound = bounds[0];
            for (Size j = 1; j < dim; j++) {
                if (bounds[j] > maxBound) maxBound = bounds[j];
            }
            //weights by dimension is the size of the bound
            //divided by the largest bound
            maxBound = 1.0 / maxBound;
            bounds *= maxBound;
            setDimension(dim, bounds);
        }
      protected:
        mutable VariateGenerator variate_;
        MersenneTwisterUniformRng rng_;
        Array weights_;
        Size dim_;
    };
}
#endif


#ifndef id_bce8772582bda668cc60662aa2f02e18
#define id_bce8772582bda668cc60662aa2f02e18
inline bool test_bce8772582bda668cc60662aa2f02e18(const int* i) {
    return i != nullptr;
}
#endif
