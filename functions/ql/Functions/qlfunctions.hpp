
/*
 Copyright (C) 2004 Ferdinando Ametrano
 Copyright (C) 2003 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#ifndef quantlib_functions_all_hpp
#define quantlib_functions_all_hpp

#include <ql/qldefines.hpp>

#define QL_FUN_LIB_NAME "QuantLibFunctions-" QL_LIB_TOOLSET QL_LIB_THREAD_OPT QL_LIB_RT_OPT "-" QL_LIB_VERSION ".lib"

#if defined(_MSC_VER)
#pragma comment(lib, QL_FUN_LIB_NAME)
#ifdef BOOST_LIB_DIAGNOSTIC
#  pragma message("Linking to lib file: " QL_FUN_LIB_NAME)
#endif
#endif


#endif
