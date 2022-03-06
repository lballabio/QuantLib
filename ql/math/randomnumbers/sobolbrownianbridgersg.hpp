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

/*! \file sobolbrownianbridgersg.hpp
    \brief interface class to map the functionality of SobolBrownianGenerator
           to the "conventional" sequence generator interface
*/

#ifndef quantlib_sobol_brownian_bridge_rsg_hpp
#define quantlib_sobol_brownian_bridge_rsg_hpp

#include <ql/models/marketmodels/browniangenerators/sobolbrowniangenerator.hpp>

namespace QuantLib {

    class SobolBrownianBridgeRsg {
      public:
        typedef Sample<std::vector<Real> > sample_type;

        SobolBrownianBridgeRsg(Size factors, Size steps,
                               SobolBrownianGenerator::Ordering ordering
                                   = SobolBrownianGenerator::Diagonal,
                               unsigned long seed = 0,
                               SobolRsg::DirectionIntegers directionIntegers
                                   = SobolRsg::JoeKuoD7);

        const sample_type& nextSequence() const;
        const sample_type& lastSequence() const;
        Size dimension() const;

      private:
        const Size factors_, steps_, dim_;
        mutable sample_type seq_;
        mutable SobolBrownianGenerator gen_;
    };
}

#endif


#ifndef id_984016e84069b834d7ba57347d2c5db8
#define id_984016e84069b834d7ba57347d2c5db8
inline bool test_984016e84069b834d7ba57347d2c5db8(const int* i) {
    return i != nullptr;
}
#endif
