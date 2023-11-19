/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2023 Peter Caspers

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

/*! \file burley2020sobolrsg.hpp
    \brief scrambled Sobol sequence following Burley, 2020
*/

#ifndef quantlib_burley2020_scrambled_sobolrsg_hpp
#define quantlib_burley2020_scrambled_sobolrsg_hpp

#include <ql/math/randomnumbers/sobolrsg.hpp>
#include <ql/shared_ptr.hpp>

namespace QuantLib {

    //! Scrambled sobol sequence according to Burley, 2020
    /*! Reference: Brent Burley: Practical Hash-based Owen Scrambling,
        Journal of Computer Graphics Techniques, Vol. 9, No. 4, 2020 */
    class Burley2020SobolRsg {
      public:
        typedef Sample<std::vector<Real>> sample_type;
        explicit Burley2020SobolRsg(
            Size dimensionality,
            unsigned long seed = 42,
            SobolRsg::DirectionIntegers directionIntegers = SobolRsg::Jaeckel,
            unsigned long scrambleSeed = 43);
        const std::vector<std::uint32_t>& skipTo(std::uint32_t n) const;
        const std::vector<std::uint32_t>& nextInt32Sequence() const;
        const SobolRsg::sample_type& nextSequence() const;
        const sample_type& lastSequence() const { return sequence_; }
        Size dimension() const { return dimensionality_; }

      private:
        void reset() const;
        Size dimensionality_;
        unsigned long seed_;
        SobolRsg::DirectionIntegers directionIntegers_;
        mutable ext::shared_ptr<SobolRsg> sobolRsg_;
        mutable std::vector<std::uint32_t> integerSequence_;
        mutable sample_type sequence_;
        mutable std::uint32_t nextSequenceCounter_;
        mutable std::vector<std::uint32_t> group4Seeds_;
    };

}


#endif
