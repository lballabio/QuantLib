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
            Disposable<Matrix>
                ForwardForwardJacobian(const CurveState& cs, 
                                       Size multiplier,
                                       Size offset);

            /*! Returns the Y matrix to switch base          
            forward rates with tenor multipler and forward rates with tenor 1*/

            Disposable<Matrix>
                YMatrix(const CurveState& cs,
                const std::vector<Spread>& shortDisplacements,
                const std::vector<Spread>& longDisplacements,
                Size Multiplier,
                Size offset
                );

            /*! 
            replaces curve state with curve state based on periodic subset of times

            */
            LMMCurveState
                RestrictCurveState(const CurveState& cs,
                                 Size multiplier,
                                 Size offSet
                                );

        }

    }

#endif


#ifndef id_04482d2c7c96027dc5e370b6b36a2b74
#define id_04482d2c7c96027dc5e370b6b36a2b74
inline bool test_04482d2c7c96027dc5e370b6b36a2b74(int* i) { return i != 0; }
#endif
