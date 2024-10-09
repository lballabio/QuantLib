/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2024 Klaus Spanderen

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

/*! \file householder.hpp
    \brief Householder transformation and Householder projection
*/

#ifndef quantlib_householder_hpp
#define quantlib_householder_hpp

#include <ql/math/matrix.hpp>

namespace QuantLib {

    /*! References:
     	 https://en.wikipedia.org/wiki/Householder_transformation
    */

    class HouseholderTransformation  {
      public:
    	HouseholderTransformation(const Array v);

        Matrix getMatrix() const;
    	Array operator()(const Array& x) const;

      private:
    	const Array v_;
    };


    class HouseholderReflection {
      public:
    	HouseholderReflection(const Array e);

        Array operator()(const Array& a) const;
    	Array reflectionVector(const Array& a) const;

      private:
    	const Array e_;
    };
}

#endif
