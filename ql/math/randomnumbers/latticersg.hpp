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

/*! \file latticersg.hpp
    \brief lattice rule code for low discrepancy numbers
*/

#ifndef quantlib_lattice_rsg_hpp
#define quantlib_lattice_rsg_hpp

#include <ql/methods/montecarlo/sample.hpp>
#include <vector>

namespace QuantLib {

   
    class LatticeRsg 
    {
      public:
        typedef Sample<std::vector<Real> > sample_type;
        LatticeRsg(Size dimensionality, std::vector<Real> z, Size N);
        /*! skip to the n-th sample in the low-discrepancy sequence */
        void skipTo(unsigned long n);
        const LatticeRsg::sample_type& nextSequence();     
        Size dimension() const { return dimensionality_; }
        const sample_type& lastSequence() const { return sequence_; }

      private:
        Size dimensionality_;
        Size N_;
        Size i_;
        std::vector<Real> z_;
        
        sample_type sequence_;
    };

}

#endif


#ifndef id_1bc9d10fb5967c63e1b660cbbab4eaf7
#define id_1bc9d10fb5967c63e1b660cbbab4eaf7
inline bool test_1bc9d10fb5967c63e1b660cbbab4eaf7(const int* i) {
    return i != nullptr;
}
#endif
