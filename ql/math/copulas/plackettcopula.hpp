/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2010 Hachemi Benyahia
 Copyright (C) 2010 DeriveXperts SAS

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

/*! \file plackettcopula.hpp
    \brief Plackett copula
*/

#ifndef quantlib_math_plackett_copula_hpp
#define quantlib_math_plackett_copula_hpp

#include <ql/types.hpp>
#include <functional>

namespace QuantLib {

    //! Plackett copula
    class PlackettCopula {
      public:
        typedef Real first_argument_type;
        typedef Real second_argument_type;
        typedef Real result_type;

        PlackettCopula(Real theta);
        Real operator()(Real x, Real y) const;
      private:
        Real theta_;
    };

}

#endif
