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

#ifndef quantlib_alpha_finder_hpp
#define quantlib_alpha_finder_hpp

#include <ql/models/marketmodels/models/alphaform.hpp>
#include <ql/shared_ptr.hpp>
#include <vector>

namespace QuantLib {

    class AlphaFinder {
      public:
        AlphaFinder(ext::shared_ptr<AlphaForm> parametricform);

        bool solve(Real alpha0,
                   Integer stepindex,
                   const std::vector<Volatility>& rateonevols,
                   const std::vector<Volatility>& ratetwohomogeneousvols,
                   const std::vector<Real>& correlations,
                   Real w0,
                   Real w1,
                   Real targetVariance,
                   Real tolerance,
                   Real alphaMax,
                   Real alphaMin,
                   Integer steps,
                   Real& alpha,
                   Real& a,
                   Real& b,
                   std::vector<Volatility>& ratetwovols);

        bool solveWithMaxHomogeneity(
                        Real alpha0,
                        Integer stepindex,
                        const std::vector<Volatility>& rateonevols,
                        const std::vector<Volatility>& ratetwohomogeneousvols,
                        const std::vector<Real>& correlations,
                        Real w0,
                        Real w1,
                        Real targetVariance,
                        Real tolerance,
                        Real alphaMax,
                        Real alphaMin,
                        Integer steps,
                        Real& alpha,
                        Real& a,
                        Real& b,
                        std::vector<Volatility>& ratetwovols);
      private:
        Real computeLinearPart(Real alpha);
        Real computeQuadraticPart(Real alpha);
        Real valueAtTurningPoint(Real alpha);
        Real minusValueAtTurningPoint(Real alpha);
        bool testIfSolutionExists(Real alpha);
        bool finalPart(Real alphaFound,
                       Integer stepindex,
                       const std::vector<Volatility>& ratetwohomogeneousvols,
                       Real quadraticPart,
                       Real linearPart,
                       Real constantPart,
                       Real& alpha,
                       Real& a,
                       Real& b,
                       std::vector<Volatility>& ratetwovols);
        Real homogeneityfailure(Real alpha);
        ext::shared_ptr<AlphaForm> parametricform_;
        Integer stepindex_;
        std::vector<Volatility> rateonevols_, ratetwohomogeneousvols_;
        std::vector<Volatility> putativevols_;
        std::vector<Real> correlations_;
        Real w0_, w1_;
        Real constantPart_, linearPart_, quadraticPart_;
        Real totalVar_, targetVariance_;
    };

}

#endif


#ifndef id_63a7ce369d9eb4a56b3efc0192f49774
#define id_63a7ce369d9eb4a56b3efc0192f49774
inline bool test_63a7ce369d9eb4a56b3efc0192f49774(int* i) { return i != 0; }
#endif
