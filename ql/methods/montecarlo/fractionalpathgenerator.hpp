/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2022 Chester Wong
 Copyright (C) 2022 Wojciech Czernous

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

/*! \file fractionalpathgenerator.hpp
    \brief Generates random paths using a sequence generator
*/

#ifndef quantlib_montecarlo_fractional_path_generator_hpp
#define quantlib_montecarlo_fractional_path_generator_hpp

#include "path.hpp"
#include <ql/math/randomnumbers/fractionalnoisersg.hpp>

using std::pow;

namespace QuantLib {
    //! Generates random rough paths using a sequence generator
    /*! Generates random rough paths with Hurst parameter H
        using a gaussian sequence generator

        \ingroup mcarlo

        \test see the test for the underlying FractionalGaussianNoiseRsg
    */
    template <class GSG>
    // It could be a subtype of FractionalPathGenerator<GSG>, 
    // if not for the underlying Itô process;
    // unfortunately, the fBm is not an Itô process.
    class FractionalPathGenerator {
      public:
        typedef Sample<Path> sample_type;
        // constructors
        FractionalPathGenerator(Real HurstParameter,
                                Time length,
                                Size timeSteps,
                                GSG generator);
        //! \name inspectors
        //@{
        const sample_type& next() const;
        const sample_type& antithetic() const;
        Size size() const { return dimension_; }
        const TimeGrid& timeGrid() const { return timeGrid_; }
        Real covariance(Size i, Size j) const { return 0.5 * ( pow(timeGrid_[i],2*H_) + pow(timeGrid_[j],2*H_) - pow(fabs(timeGrid_[i] - timeGrid_[j]),2*H_) ); }
        //@}
      private:
        const sample_type& next(bool antithetic) const;
        Real H_;
        FractionalGaussianNoiseRsg<GSG> generator_;
        Size dimension_;
        TimeGrid timeGrid_;
        mutable sample_type next_;
    };


    // template definitions

    template <class GSG>
    FractionalPathGenerator<GSG>::FractionalPathGenerator(Real HurstParameter,
                                                          Time length,
                                                          Size timeSteps,
                                                          GSG generator)
    : H_(HurstParameter), generator_(H_, std::move(generator)),
      dimension_(generator_.dimension()), timeGrid_(length, timeSteps),
      next_(Path(timeGrid_), 1.0) {
        QL_REQUIRE(dimension_==timeSteps,
                   "sequence generator dimensionality (" << dimension_
                   << ") != timeSteps (" << timeSteps << ")");        
    }

    template <class GSG>
    const typename FractionalPathGenerator<GSG>::sample_type&
    FractionalPathGenerator<GSG>::next() const {
        return next(false);
    }

    template <class GSG>
    const typename FractionalPathGenerator<GSG>::sample_type&
    FractionalPathGenerator<GSG>::antithetic() const {
        return next(true);
    }

    template <class GSG>
    const typename FractionalPathGenerator<GSG>::sample_type&
    FractionalPathGenerator<GSG>::next(bool antithetic) const {

        typedef typename FractionalGaussianNoiseRsg<GSG>::sample_type sequence_type;
        const sequence_type& sequence_ =
            antithetic ? generator_.lastSequence()
                       : generator_.nextSequence();

        next_.weight = sequence_.weight;

        Path& path = next_.value;
        Real wH_i = 0.0;
        path.front() = wH_i; 

        Time dt = timeGrid_.dt(0);
        Real scale = std::pow(dt, H_);

        if (antithetic)
            scale *= -1;

        for (Size i=1; i<path.length(); i++) {
            wH_i += sequence_.value[i];
            path[i] = scale * wH_i;  // W^H_t  ~  (t/i)^H * W^H_i = scale * W^H_i
        }

        return next_;
    }

}


#endif
