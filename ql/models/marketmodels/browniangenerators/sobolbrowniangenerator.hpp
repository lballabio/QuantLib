/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 StatPro Italia srl

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


#ifndef quantlib_sobol_brownian_generator_hpp
#define quantlib_sobol_brownian_generator_hpp

#include <ql/models/marketmodels/browniangenerator.hpp>
#include <ql/math/randomnumbers/inversecumulativersg.hpp>
#include <ql/math/randomnumbers/sobolrsg.hpp>
#include <ql/methods/montecarlo/brownianbridge.hpp>
#include <ql/math/distributions/normaldistribution.hpp>
#include <vector>

namespace QuantLib {

    //! Sobol Brownian generator for market-model simulations
    /*! Incremental Brownian generator using a Sobol generator,
        inverse-cumulative Gaussian method, and Brownian bridging.
    */
    class SobolBrownianGenerator : public BrownianGenerator {
      public:
        enum Ordering {
            Factors,  /*!< The variates with the best quality will be
                           used for the evolution of the first factor. */
            Steps,    /*!< The variates with the best quality will be
                           used for the largest steps of all factors. */
            Diagonal  /*!< A diagonal schema will be used to assign
                           the variates with the best quality to the
                           most important factors and the largest
                           steps. */
        };
        SobolBrownianGenerator(
                           Size factors,
                           Size steps,
                           Ordering ordering,
                           unsigned long seed = 0,
                           SobolRsg::DirectionIntegers directionIntegers
                                                        = SobolRsg::Jaeckel);

        Real nextPath() override;
        Real nextStep(std::vector<Real>&) override;

        Size numberOfFactors() const override;
        Size numberOfSteps() const override;

        // test interface
        const std::vector<std::vector<Size> >& orderedIndices() const;
        std::vector<std::vector<Real> > transform(
                              const std::vector<std::vector<Real> >& variates);

      private:
        Size factors_, steps_;
        Ordering ordering_;
        InverseCumulativeRsg<SobolRsg,InverseCumulativeNormal> generator_;
        BrownianBridge bridge_;
        // work variables
        Size lastStep_;
        std::vector<std::vector<Size> > orderedIndices_;
        std::vector<std::vector<Real> > bridgedVariates_;
    };

    class SobolBrownianGeneratorFactory : public BrownianGeneratorFactory {
      public:
        SobolBrownianGeneratorFactory(
                           SobolBrownianGenerator::Ordering ordering,
                           unsigned long seed = 0,
                           SobolRsg::DirectionIntegers directionIntegers
                                                         = SobolRsg::Jaeckel);
        ext::shared_ptr<BrownianGenerator> create(Size factors, Size steps) const override;

      private:
        SobolBrownianGenerator::Ordering ordering_;
        unsigned long seed_;
        SobolRsg::DirectionIntegers integers_;
    };

}


#endif


#ifndef id_090f03733b60536b68a497005920ac6a
#define id_090f03733b60536b68a497005920ac6a
inline bool test_090f03733b60536b68a497005920ac6a(const int* i) {
    return i != nullptr;
}
#endif
