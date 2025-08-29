/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2003, 2004, 2005, 2006 StatPro Italia srl

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

#include <ql/methods/finitedifferences/bsmoperator.hpp>
#include <ql/math/transformedgrid.hpp>
#include <ql/methods/finitedifferences/pdebsm.hpp>

namespace QuantLib {

    QL_DEPRECATED_DISABLE_WARNING

    BSMOperator::BSMOperator(Size size, Real dx, Rate r,
                             Rate q, Volatility sigma)
    : TridiagonalOperator(size) {
        Real sigma2 = sigma*sigma;
        Real nu = r-q-sigma2/2;
        Real pd = -(sigma2/dx-nu)/(2*dx);
        Real pu = -(sigma2/dx+nu)/(2*dx);
        Real pm = sigma2/(dx*dx)+r;
        setMidRows(pd,pm,pu);
    }

    BSMOperator::BSMOperator(const Array& grid,
                             Rate r, Rate q, Volatility sigma)
    : TridiagonalOperator(grid.size()) {
        PdeBSM::grid_type logGrid(grid);
        Real sigma2 = sigma*sigma;
        Real nu = r-q-sigma2/2;
        for (Size i=1; i<logGrid.size()-1; ++i) {
            Real pd = -(sigma2/logGrid.dxm(i)-nu)/logGrid.dx(i);
            Real pu = -(sigma2/logGrid.dxp(i)+nu)/logGrid.dx(i);
            Real pm = sigma2/(logGrid.dxm(i)*logGrid.dxp(i)) + r;
            setMidRow(i,pd,pm,pu);
        }
    }

    QL_DEPRECATED_ENABLE_WARNING

}
