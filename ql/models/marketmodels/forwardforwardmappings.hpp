/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
Copyright (C) 2007 Mark Joshi

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

/*! \file forwardforwardmappings.hpp
\brief Utility functions for mapping between forward rates of varying tenor
*/

#ifndef quantlib_forward_forward_mappings_hpp
#define quantlib_forward_forward_mappings_hpp

#include <ql/math/matrix.hpp>

namespace QuantLib {

    class CurveState;
    class LMMCurveState;

    namespace ForwardForwardMappings 
        {
            /*! Returns the dg[i]/df[j] jacobian between
            forward rates with tenor multipler and forward rates with tenor 1*/
            Matrix ForwardForwardJacobian(const CurveState& cs, 
                                          Size multiplier,
                                          Size offset);

            /*! Returns the Y matrix to switch base          
            forward rates with tenor multipler and forward rates with tenor 1*/

            Matrix YMatrix(const CurveState& cs,
                           const std::vector<Spread>& shortDisplacements,
                           const std::vector<Spread>& longDisplacements,
                           Size Multiplier,
                           Size offset);

            /*! 
            replaces curve state with curve state based on periodic subset of times

            */
            LMMCurveState RestrictCurveState(const CurveState& cs,
                                             Size multiplier,
                                             Size offSet);

        }

    }

#endif
