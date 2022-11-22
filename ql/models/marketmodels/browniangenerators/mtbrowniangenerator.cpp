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

#include <ql/models/marketmodels/browniangenerators/mtbrowniangenerator.hpp>
#include <algorithm>

namespace QuantLib {

    MTBrownianGenerator::MTBrownianGenerator(Size factors, Size steps, unsigned long seed)
    : factors_(factors), steps_(steps),
      generator_(factors * steps, MersenneTwisterUniformRng(seed)) {}

    Real MTBrownianGenerator::nextStep(std::vector<Real>& output) {
        #if defined(QL_EXTRA_SAFETY_CHECKS)
        QL_REQUIRE(output.size() == factors_, "size mismatch");
        QL_REQUIRE(lastStep_<steps_, "uniform sequence exhausted");
        #endif
        // no copying, just fetching a reference
        const std::vector<Real>& currentSequence = generator_.lastSequence().value;
        Size start = lastStep_*factors_, end = (lastStep_+1)*factors_;
        std::transform(currentSequence.begin()+start,
                       currentSequence.begin()+end,
                       output.begin(),
                       inverseCumulative_);
        ++lastStep_;
        return 1.0;
    }

    Real MTBrownianGenerator::nextPath() {
        typedef RandomSequenceGenerator<MersenneTwisterUniformRng>::sample_type
            sample_type;

        const sample_type& sample = generator_.nextSequence();
        lastStep_ = 0;
        return sample.weight;
    }

    Size MTBrownianGenerator::numberOfFactors() const { return factors_; }

    Size MTBrownianGenerator::numberOfSteps() const { return steps_; }


    MTBrownianGeneratorFactory::MTBrownianGeneratorFactory(unsigned long seed)
    : seed_(seed) {}

    ext::shared_ptr<BrownianGenerator>
    MTBrownianGeneratorFactory::create(Size factors, Size steps) const {
        return ext::shared_ptr<BrownianGenerator>(
                              new MTBrownianGenerator(factors, steps, seed_));
    }

}

