/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Ferdinando Ametrano
 Copyright (C) 2007 Chris Kenyon
 Copyright (C) 2007 StatPro Italia srl

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

/*! \file bootstraperror.hpp
    \brief boostrap error.
*/

#ifndef quantlib_bootstrap_error_hpp
#define quantlib_bootstrap_error_hpp

#include <ql/shared_ptr.hpp>
#include <ql/types.hpp>
#include <utility>

namespace QuantLib {

    //! bootstrap error
    template <class Curve>
    class BootstrapError {
        typedef typename Curve::traits_type Traits;
      public:
        BootstrapError(const Curve* curve,
                       ext::shared_ptr<typename Traits::helper> instrument,
                       Size segment);
        Real operator()(Rate guess) const;
        const ext::shared_ptr<typename Traits::helper>& helper() {
            return helper_;
        }
      private:
        const Curve* curve_;
        const ext::shared_ptr<typename Traits::helper> helper_;
        const Size segment_;
    };


    // template definitions

    template <class Curve>
    BootstrapError<Curve>::BootstrapError(const Curve* curve,
                                          ext::shared_ptr<typename Traits::helper> helper,
                                          Size segment)
    : curve_(curve), helper_(std::move(helper)), segment_(segment) {}

#ifndef __DOXYGEN__
    template <class Curve>
    Real BootstrapError<Curve>::operator()(Real guess) const {
        Traits::updateGuess(curve_->data_, guess, segment_);
        curve_->interpolation_.update();
        return helper_->quoteError();
    }
    #endif

}

#endif
