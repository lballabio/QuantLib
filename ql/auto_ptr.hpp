/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2018 StatPro Italia srl

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

/*! \file auto_ptr.hpp
    \brief Facilities to switch from auto_ptr to unique_ptr
*/

#ifndef quantlib_auto_ptr_hpp
#define quantlib_auto_ptr_hpp

#include <ql/qldefines.hpp>

#if defined(QL_USE_STD_UNIQUE_PTR)
#define QL_UNIQUE_OR_AUTO_PTR std::unique_ptr
#else
#define QL_UNIQUE_OR_AUTO_PTR std::auto_ptr
#endif


#endif

