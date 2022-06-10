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

/*! \file transformedgrid.hpp
    \brief encapuslates a grid
*/

#ifndef quantlib_transformed_grid_hpp
#define quantlib_transformed_grid_hpp

#include <ql/math/array.hpp>
#include <functional>
#include <numeric>

namespace QuantLib {

    //! transformed grid
    /*! This package encapuslates an array of grid points.  It is used primarily
     in PDE calculations.
    */
    class TransformedGrid {
    public:
        TransformedGrid (const Array &grid) :
            grid_(grid), transformedGrid_(grid),
            dxm_(grid.size()), dxp_(grid.size()),
            dx_(grid.size()){
            for (Size i=1; i < transformedGrid_.size() -1 ; i++) {
                dxm_[i] = transformedGrid_[i] - transformedGrid_[i-1];
                dxp_[i] = transformedGrid_[i+1] - transformedGrid_[i];
                dx_[i] = dxm_[i] + dxp_[i];
            }
        }

#if defined(__GNUC__) && (__GNUC__ >= 7)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnoexcept-type"
#endif

        template <class T>
        TransformedGrid (const Array &grid, T func) :
            grid_(grid), transformedGrid_(grid.size()),
            dxm_(grid.size()), dxp_(grid.size()),
            dx_(grid.size()){
            std::transform(grid_.begin(),
                           grid_.end(),
                           transformedGrid_.begin(),
                           func);
            for (Size i=1; i < transformedGrid_.size() -1 ; i++) {
                dxm_[i] = transformedGrid_[i] - transformedGrid_[i-1];
                dxp_[i] = transformedGrid_[i+1] - transformedGrid_[i];
                dx_[i] = dxm_[i] + dxp_[i];
            }
        }

#if defined(__GNUC__) && (__GNUC__ >= 7)
#pragma GCC diagnostic pop
#endif

        const Array &gridArray() const { return grid_;}
        const Array &transformedGridArray() const { return transformedGrid_;}
        const Array &dxmArray() const { return dxm_;}
        const Array &dxpArray() const { return dxp_;}
        const Array &dxArray() const { return dx_;}

        Real grid(Size i) const { return grid_[i];}
        Real transformedGrid(Size i) const { return transformedGrid_[i];}
        Real dxm(Size i) const { return dxm_[i];}
        Real dxp(Size i) const { return dxp_[i];}
        Real dx(Size i) const { return dx_[i];}
        Size size() const {return grid_.size();}

    protected:
        Array grid_;
        Array transformedGrid_;
        Array dxm_;
        Array dxp_;
        Array dx_;
    };

    class LogGrid : public TransformedGrid {
    public:
        LogGrid(const Array &grid) :
            TransformedGrid(grid, [](Real x) { return std::log(x); }){};
        const Array & logGridArray() const { return transformedGridArray();}
        Real logGrid(Size i) const { return transformedGrid(i);}
    };

}


#endif
