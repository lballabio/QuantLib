/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2011 Klaus Spanderen

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

/*! \file exponentialjump1dmesher.hpp
    \brief mesher for a exponential jump mesher with high 
           mean reversion rate and low jump intensity
*/

#ifndef quantlib_exponential_jump_1d_mesher_hpp
#define quantlib_exponential_jump_1d_mesher_hpp

#include <ql/methods/finitedifferences/meshers/fdm1dmesher.hpp>
#include <ql/methods/finitedifferences/operators/fdmlinearopiterator.hpp>

namespace QuantLib {

    /*! Mesher for a exponential jump process with high 
        mean reversion rate and low jump intensity
        \f[
        \begin{array}{rcl}
        dY_t  &=& -\beta Y_{t-}dt + J_tdN_t \\
        \omega(J)&=&\frac{1}{\eta_u}e^{-\frac{1}{\eta_u}J}
        \end{array}
        \f]
    */

    /*! References:
        B. Hambly, S. Howison, T. Kluge, Modelling spikes and pricing 
        swing options in electricity markets,
        http://people.maths.ox.ac.uk/hambly/PDF/Papers/elec.pdf
    */

    class ExponentialJump1dMesher : public Fdm1dMesher {
      public:
        ExponentialJump1dMesher(Size steps, Real beta, Real jumpIntensity, 
                                Real eta, Real eps = 1e-3);
        
        // approximation. see Hambly et.al.
        Real jumpSizeDensity(Real x) const; // t->\inf
        Real jumpSizeDensity(Real x, Time t) const;
        Real jumpSizeDistribution(Real x) const; // t->\inf
        Real jumpSizeDistribution(Real x, Time t) const;
        
      private:
        const Real beta_, jumpIntensity_, eta_;
    };
}

#endif
