/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 Copyright (C) 2005 Joseph Wang

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

/*! \file operatortraits.hpp
    \brief Differential operator traits
*/

#ifndef quantlib_operator_traits_hpp
#define quantlib_operator_traits_hpp

#include <ql/methods/finitedifferences/boundarycondition.hpp>
#include <ql/methods/finitedifferences/stepcondition.hpp>
#include <vector>

namespace QuantLib {

    template <class Operator>
    class OperatorTraits {
      public:
        typedef Operator operator_type;
        typedef typename Operator::array_type array_type;
        typedef BoundaryCondition<operator_type> bc_type;
        typedef std::vector<boost::shared_ptr<bc_type> > bc_set;
        typedef StepCondition<array_type> condition_type;
    };

}


#endif

