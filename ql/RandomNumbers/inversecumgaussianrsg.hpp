
/*
 Copyright (C) 2003 Ferdinando Ametrano
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file inversecumgaussianrsg.hpp
    \brief Inverse cumulative Gaussian random sequence generator
*/

#ifndef quantlib_inversecumulative_gaussian_rsg_h
#define quantlib_inversecumulative_gaussian_rsg_h

#include <ql/Math/array.hpp>
#include <ql/MonteCarlo/sample.hpp>

namespace QuantLib {

    //! Inverse cumulative Gaussian random sequence generator
    /*! It uses a sequence of uniform deviate in (0, 1) as the
        source of cumulative normal distribution values.
        Then an inverse cumulative normal distribution is used as it is
        approximately a Gaussian deviate with average 0.0 and standard
        deviation 1.0.

        The uniform deviate sequence is supplied by USG.

        Class USG must implement the following interface:
        \code
            USG::sample_type USG::nextSequence() const;
            Size USG::dimension() const;
        \endcode

        The inverse cumulative normal distribution is supplied by I.

        Class I must implement the following interface:
        \code
            I::I();
            Real I::operator() const;
        \endcode
    */
    template <class USG, class I>
    class ICGaussianRsg {
      public:
        typedef Sample<Array> sample_type;
        explicit ICGaussianRsg(const USG& uniformSequenceGenerator);
        //! returns next sample from the Gaussian distribution
        const sample_type& nextSequence() const;
        const sample_type& lastSequence() const {
            return x;
        }
        Size dimension() const {return dimension_;}
      private:
        USG uniformSequenceGenerator_;
        Size dimension_;
        mutable sample_type x;
        I ICND_;
    };

    template <class USG, class I>
    ICGaussianRsg<USG, I>::ICGaussianRsg(const USG& uniformSequenceGenerator)
    : uniformSequenceGenerator_(uniformSequenceGenerator),
      dimension_(uniformSequenceGenerator_.dimension()),
      x(Array(dimension_), 1.0) {}

    template <class USG, class I>
    inline const typename ICGaussianRsg<USG, I>::sample_type&
    ICGaussianRsg<USG, I>::nextSequence() const {
        typename USG::sample_type sample =
            uniformSequenceGenerator_.nextSequence();
        x.weight = sample.weight;
        for (Size i = 0; i < dimension_; i++) {
            x.value[i] = ICND_(sample.value[i]);
        }
        return x;
    }

}


#endif
