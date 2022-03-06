/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*

 Copyright (C) 2007 Mark Joshi

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

/*! \file latticerules.hpp
  Cools, Kuo, Nuyens (SIAM J. Sci. Comp., 2006)
*/

#ifndef quantlib_lattice_rules_hpp
#define quantlib_lattice_rules_hpp


#include <ql/types.hpp>
#include <vector>

namespace QuantLib
{

class LatticeRule
{
public:

    enum type {A, B , C , D};

    static void getRule(type name, std::vector<Real>& Z, Integer N);


};
}

#endif


#ifndef id_4d825abade6c4795658cba829d762b01
#define id_4d825abade6c4795658cba829d762b01
inline bool test_4d825abade6c4795658cba829d762b01(int* i) { return i != 0; }
#endif
