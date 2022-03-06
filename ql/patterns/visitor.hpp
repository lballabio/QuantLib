/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003 RiskMap srl

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

/*! \file visitor.hpp
    \brief degenerate base class for the Acyclic Visitor pattern
*/

#ifndef quantlib_visitor_h
#define quantlib_visitor_h

#include <ql/qldefines.hpp>

namespace QuantLib {

    //! degenerate base class for the Acyclic %Visitor pattern
    /*! \ingroup patterns */
    class AcyclicVisitor {
      public:
        virtual ~AcyclicVisitor() = default;
    };

    //! %Visitor for a specific class
    template <class T>
    class Visitor {
      public:
        virtual ~Visitor() = default;
        virtual void visit(T&) = 0;
    };

}


#endif


#ifndef id_1693416c588d3eb025dc75be84392708
#define id_1693416c588d3eb025dc75be84392708
inline bool test_1693416c588d3eb025dc75be84392708(const int* i) {
    return i != nullptr;
}
#endif
