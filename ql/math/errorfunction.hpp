/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003 Ferdinando Ametrano

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

/*! \file errorfunction.hpp
    \brief Error function
*/

#ifndef quantlib_error_function_h
#define quantlib_error_function_h

#include <ql/types.hpp>
#include <functional>

namespace QuantLib {

    //! %Error function
    /*! formula here ...
        Used to calculate the cumulative normal distribution function
    */
    class ErrorFunction {
      public:
        /*! \deprecated Use `auto` or `decltype` instead.
                        Deprecated in version 1.29.
        */
        QL_DEPRECATED
        typedef Real argument_type;

        /*! \deprecated Use `auto` or `decltype` instead.
                        Deprecated in version 1.29.
        */
        QL_DEPRECATED
        typedef Real result_type;

        ErrorFunction() = default;
        // function
        Real operator()(Real x) const;
      private:
        static const Real tiny, one, erx, efx, efx8;
        static const Real pp0, pp1,pp2,pp3,pp4;
        static const Real qq1,qq2,qq3,qq4,qq5;
        static const Real pa0,pa1,pa2,pa3,pa4,pa5,pa6;
        static const Real qa1,qa2,qa3,qa4,qa5,qa6;
        static const Real ra0,ra1,ra2,ra3,ra4,ra5,ra6,ra7;
        static const Real sa1,sa2,sa3,sa4,sa5,sa6,sa7,sa8;
        static const Real rb0,rb1,rb2,rb3,rb4,rb5,rb6;
        static const Real sb1,sb2,sb3,sb4,sb5,sb6,sb7;
    };

}


#endif
