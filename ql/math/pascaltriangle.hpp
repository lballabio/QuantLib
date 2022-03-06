/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2015 Ferdinando Ametrano
 Copyright (C) 2015 Paolo Mazzocchi

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

/*! \file pascaltriangle.hpp
    \brief Pascal triangle coefficients calculator
*/

#ifndef quantlib_pascal_triangle_hpp
#define quantlib_pascal_triangle_hpp

#include <ql/types.hpp>
#include <vector>

namespace QuantLib {

    //! Pascal triangle coefficients calculator
    class PascalTriangle {
      public:
        //! Get and store one vector of coefficients after another.
        static const std::vector<BigNatural>& get(Size order);
      private:
        PascalTriangle() = default;
        static void nextOrder();
        static std::vector<std::vector<BigNatural> > coefficients_;
    };

}

#endif


#ifndef id_32564c876288e3e1a4820020aafcc54f
#define id_32564c876288e3e1a4820020aafcc54f
inline bool test_32564c876288e3e1a4820020aafcc54f(const int* i) {
    return i != nullptr;
}
#endif
