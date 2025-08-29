/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005 Joseph Wang

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file parallelevolver.hpp
    \brief Parallel evolver for multiple arrays

    This class takes the evolver class and creates a new class which evolves
    each of the evolvers in parallel.  Part of what this does is to take the 
    types for each evolver class and then wrapper them so that they create
    new types which are sets of the old types.

    This class is intended to be run in situations where there are parallel
    differential equations such as with some convertible bond models.
*/

#ifndef quantlib_system_evolver_hpp
#define quantlib_system_evolver_hpp

// Deprecated in version 1.37
#pragma message("Warning: this file is empty and will disappear in a future release; do not include it.")


#endif
