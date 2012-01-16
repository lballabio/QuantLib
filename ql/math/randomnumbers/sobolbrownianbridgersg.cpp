/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2012 Klaus Spanderen

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

/*! \file brownianbridgersg.hpp
    \brief interface class to map the functionality of SobolBrownianGenerator
           to the "conventional" sequence generator interface
*/

#include <ql/math/randomnumbers/sobolbrownianbridgersg.hpp>

namespace QuantLib {
    SobolBrownianBridgeRsg::SobolBrownianBridgeRsg(
        Size factors, Size steps,
        SobolBrownianGenerator::Ordering ordering,
        unsigned long seed,
        SobolRsg::DirectionIntegers directionIntegers)
    : factors_(factors), steps_(steps), dim_(factors*steps),
      seq_(sample_type::value_type(factors*steps), 1.0),
      gen_(factors, steps, ordering, seed, directionIntegers) {
    }

    const SobolBrownianBridgeRsg::sample_type&
    SobolBrownianBridgeRsg::nextSequence() const {
        gen_.nextPath();
        std::vector<Real> output(factors_);
        for (Size i=0; i < steps_; ++i) {
            gen_.nextStep(output);
            std::copy(output.begin(), output.end(),
                      seq_.value.begin()+i*factors_);
        }

        return seq_;
    }

    const SobolBrownianBridgeRsg::sample_type&
    SobolBrownianBridgeRsg::lastSequence() const {
        return seq_;
    }

    Size SobolBrownianBridgeRsg::dimension() const {
        return dim_;
    }
}
