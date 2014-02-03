/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Joseph Wang

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

/*! \file domain.hpp
    \brief domain
*/

#ifndef quantlib_domain_hpp
#define quantlib_domain_hpp

#include <ql/types.hpp>
#include <functional>

namespace QuantLib {

    //! %domain abstract class
    /*! \deprecated */
    class QL_DEPRECATED Domain
        : public std::binary_function<Real, Real, bool>{
    public:
#if defined(BOOST_MSVC)
#pragma warning( push )
#pragma warning( disable : 4996 )
#endif
        virtual bool includes(Real x, Real y) const = 0;
        bool operator()(Real x, Real y) {return includes(x, y);}
        virtual ~Domain() {};
    };

    class QL_DEPRECATED UniversalDomain : public Domain {
    public:
        bool includes(Real, Real) const { return true; }
    };

    class QL_DEPRECATED NullDomain : public Domain {
    public:
        bool includes(Real, Real) const { return false; }
    };

    class QL_DEPRECATED BoundedDomain : public Domain {
    public:
        virtual Real xLowerBound() const = 0;
        virtual Real xUpperBound() const = 0;
        virtual Real yLowerBound() const = 0;
        virtual Real yUpperBound() const = 0;
    };

    class QL_DEPRECATED RectangularDomain : public BoundedDomain {
    public:
        RectangularDomain(Real x1, Real y1, Real x2, Real y2) :
            minX_(x1), minY_(y1), maxX_(x2), maxY_(y2) {};
        bool includes(Real x, Real y) const {
            return x >= minX_ && x <= maxX_ &&
                y >= minY_ && y <= maxY_;
        }
        Real xLowerBound() const { return minX_;}
        Real yLowerBound() const { return minY_;}
        Real xUpperBound() const { return maxX_;}
        Real yUpperBound() const { return maxY_;}
    private:
        Real minX_, minY_, maxX_, maxY_;
    };

#if defined(BOOST_MSVC)
#pragma warning( pop )
#endif

}


#endif
