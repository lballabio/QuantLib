/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2004 StatPro Italia srl

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

/*! \file extrapolation.hpp
    \brief class-wide extrapolation settings
*/

#ifndef quantlib_extrapolation_hpp
#define quantlib_extrapolation_hpp

#include <ql/qldefines.hpp>

namespace QuantLib {

    //! base class for classes possibly allowing extrapolation
    class Extrapolator {
      public:
        Extrapolator() = default;
        virtual ~Extrapolator() = default;
        //! \name modifiers
        //@{
        //! enable extrapolation in subsequent calls
        void enableExtrapolation(bool b = true) { extrapolate_ = b; }
        //! disable extrapolation in subsequent calls
        void disableExtrapolation(bool b = true) { extrapolate_ = !b; }
        //@}
        //! \name inspectors
        //@{
        //! tells whether extrapolation is enabled
        bool allowsExtrapolation() const { return extrapolate_; }
        //@}
      private:
        bool extrapolate_ = false;
    };

}


#endif
