/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003, 2004 Ferdinando Ametrano
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

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

/*! \file inversecumulativersg.hpp
    \brief Inverse cumulative random sequence generator
*/

#ifndef quantlib_inversecumulative_rsg_h
#define quantlib_inversecumulative_rsg_h

#include <ql/methods/montecarlo/sample.hpp>
#include <utility>
#include <vector>

namespace QuantLib {

    //! Inverse cumulative random sequence generator
    /*! It uses a sequence of uniform deviate in (0, 1) as the
        source of cumulative distribution values.
        Then an inverse cumulative distribution is used to calculate
        the distribution deviate.

        The uniform deviate sequence is supplied by USG.

        Class USG must implement the following interface:
        \code
            USG::sample_type USG::nextSequence() const;
            Size USG::dimension() const;
        \endcode

        The inverse cumulative distribution is supplied by IC.

        Class IC must implement the following interface:
        \code
            IC::IC();
            Real IC::operator() const;
        \endcode
    */
    template <class USG, class IC>
    class InverseCumulativeRsg {
      public:
        typedef Sample<std::vector<Real> > sample_type;
        explicit InverseCumulativeRsg(USG uniformSequenceGenerator);
        InverseCumulativeRsg(USG uniformSequenceGenerator, const IC& inverseCumulative);
        //! returns next sample from the inverse cumulative distribution
        const sample_type& nextSequence() const;
        const sample_type& lastSequence() const { return x_; }
        Size dimension() const { return dimension_; }
      private:
        USG uniformSequenceGenerator_;
        Size dimension_;
        mutable sample_type x_;
        IC ICD_;
    };

    template <class USG, class IC>
    InverseCumulativeRsg<USG, IC>::InverseCumulativeRsg(USG usg)
    : uniformSequenceGenerator_(std::move(usg)), dimension_(uniformSequenceGenerator_.dimension()),
      x_(std::vector<Real>(dimension_), 1.0) {}

    template <class USG, class IC>
    InverseCumulativeRsg<USG, IC>::InverseCumulativeRsg(USG usg, const IC& inverseCum)
    : uniformSequenceGenerator_(std::move(usg)), dimension_(uniformSequenceGenerator_.dimension()),
      x_(std::vector<Real>(dimension_), 1.0), ICD_(inverseCum) {}

    template <class USG, class IC>
    inline const typename InverseCumulativeRsg<USG, IC>::sample_type&
    InverseCumulativeRsg<USG, IC>::nextSequence() const {
        typename USG::sample_type sample =
            uniformSequenceGenerator_.nextSequence();
        x_.weight = sample.weight;
        for (Size i = 0; i < dimension_; i++) {
            x_.value[i] = ICD_(sample.value[i]);
        }
        return x_;
    }

}


#endif


#ifndef id_a61f91e3c114de0dcd9c2bdeaad1f8e3
#define id_a61f91e3c114de0dcd9c2bdeaad1f8e3
inline bool test_a61f91e3c114de0dcd9c2bdeaad1f8e3(const int* i) {
    return i != nullptr;
}
#endif
