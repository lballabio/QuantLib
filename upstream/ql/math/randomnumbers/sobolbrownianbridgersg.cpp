/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2012 Klaus Spanderen

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file brownianbridgersg.hpp
    \brief interface class to map the functionality of SobolBrownianGenerator
           to the "conventional" sequence generator interface
*/

#include <ql/math/randomnumbers/sobolbrownianbridgersg.hpp>

namespace QuantLib {

    namespace {
        void setNextSequence(SobolBrownianGeneratorBase& gen, std::vector<Real>& seq) {
            gen.nextPath();
            std::vector<Real> output(gen.numberOfFactors());
            for (Size i = 0; i < gen.numberOfSteps(); ++i) {
                gen.nextStep(output);
                std::copy(output.begin(), output.end(), seq.begin() + i * gen.numberOfFactors());
            }
        }
    }

    SobolBrownianBridgeRsg::SobolBrownianBridgeRsg(Size factors,
                                                   Size steps,
                                                   SobolBrownianGenerator::Ordering ordering,
                                                   unsigned long seed,
                                                   SobolRsg::DirectionIntegers directionIntegers)
    : seq_(sample_type::value_type(factors * steps), 1.0),
      gen_(factors, steps, ordering, seed, directionIntegers) {}

    const SobolBrownianBridgeRsg::sample_type&
    SobolBrownianBridgeRsg::nextSequence() const {
        setNextSequence(gen_, seq_.value);
        return seq_;

    }

    const SobolBrownianBridgeRsg::sample_type&
    SobolBrownianBridgeRsg::lastSequence() const {
        return seq_;
    }

    Size SobolBrownianBridgeRsg::dimension() const {
        return gen_.numberOfFactors() * gen_.numberOfSteps();
    }

    Burley2020SobolBrownianBridgeRsg::Burley2020SobolBrownianBridgeRsg(
        Size factors,
        Size steps,
        SobolBrownianGenerator::Ordering ordering,
        unsigned long seed,
        SobolRsg::DirectionIntegers directionIntegers,
        unsigned long scrambleSeed)
    : seq_(sample_type::value_type(factors * steps), 1.0),
      gen_(factors, steps, ordering, seed, directionIntegers, scrambleSeed) {}

    const Burley2020SobolBrownianBridgeRsg::sample_type&
    Burley2020SobolBrownianBridgeRsg::nextSequence() const {
        setNextSequence(gen_, seq_.value);
        return seq_;
    }

    const Burley2020SobolBrownianBridgeRsg::sample_type&
    Burley2020SobolBrownianBridgeRsg::lastSequence() const {
        return seq_;
    }

    Size Burley2020SobolBrownianBridgeRsg::dimension() const {
        return gen_.numberOfFactors() * gen_.numberOfSteps();
    }

}
