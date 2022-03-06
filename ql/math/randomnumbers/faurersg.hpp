/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2004 Ferdinando Ametrano
 Copyright (C) 2004 Gianni Piolanti

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

/*! \file faurersg.hpp
    \brief Faure low-discrepancy sequence generator
*/

#ifndef quantlib_faure_ld_rsg_h
#define quantlib_faure_ld_rsg_h

#include <ql/math/matrix.hpp>
#include <ql/methods/montecarlo/sample.hpp>
#include <vector>


namespace QuantLib {

    //! Faure low-discrepancy sequence generator
    /*! It is based on existing Fortran and C algorithms to calculate pascal
        matrix and gray transforms.
        -# E. Thiemard Economic generation of low-discrepancy sequences with
           a b-ary gray code.
        -# Algorithms 659, 647. http://www.netlib.org/toms/647,
           http://www.netlib.org/toms/659

        \test the correctness of the returned values is tested by
              reproducing known good values.
    */
    class FaureRsg {
      public:
        typedef Sample<std::vector<Real> > sample_type;
        FaureRsg(Size dimensionality);
        const std::vector<long int>& nextIntSequence() const {
            generateNextIntSequence();
            return integerSequence_;
        }
        const std::vector<long int>& lastIntSequence() const {
            return integerSequence_;
        }
        const sample_type& nextSequence() const {
            generateNextIntSequence();
            for (Size i=0; i<dimensionality_; i++)
                sequence_.value[i] = integerSequence_[i]/normalizationFactor_;
            return sequence_;
        }
        const sample_type& lastSequence() const { return sequence_; }
        Size dimension() const { return dimensionality_; }
      private:
        void generateNextIntSequence() const;
        Size dimensionality_;
        // mutable unsigned long sequenceCounter_;
        mutable sample_type sequence_;
        mutable std::vector<long int> integerSequence_;
        mutable std::vector<long int> bary_;
        mutable std::vector<std::vector<long int> > gray_;
        Size base_, mbit_;
        std::vector<std::vector<long int> > powBase_;
        std::vector<long int> addOne_;
        std::vector<std::vector<std::vector<long int> > > pascal3D;
        double normalizationFactor_;
    };

}

#endif




#ifndef id_06a111bdc6e579eb9924b14259ecd005
#define id_06a111bdc6e579eb9924b14259ecd005
inline bool test_06a111bdc6e579eb9924b14259ecd005(const int* i) {
    return i != nullptr;
}
#endif
