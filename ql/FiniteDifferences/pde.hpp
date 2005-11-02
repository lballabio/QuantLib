/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 StatPro Italia srl
 Copyright (C) 2005 Joseph Wang

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file valueatcenter.hpp
    \brief compute value, first, and second derivatives at grid center
*/

#ifndef quantlib_pde_hpp
#define quantlib_pde_hpp

#include <ql/Math/array.hpp>
#include <ql/Math/transformedgrid.hpp>

namespace QuantLib {
    class PdeSecondOrderParabolic {
    public:
        virtual ~PdeSecondOrderParabolic() {};
        virtual Real diffusion(Time t, Real x) const = 0;
        virtual Real drift(Time t, Real x) const = 0;
        virtual Real discount(Time t, Real x) const = 0;
        virtual void generateOperator(Time t,
                                      const TransformedGrid &tg,
                                      TridiagonalOperator &L) const {
            for (Size i=1; i < tg.size() - 1; i++) {
                Real sigma = diffusion(t, tg.grid(i));
                Real nu = drift(t, tg.grid(i));
                Real r = discount(t, tg.grid(i));
                Real sigma2 = sigma * sigma;
                
                Real pd = -(sigma2/tg.dxm(i)-nu)/ tg.dx(i);
                Real pu = -(sigma2/tg.dxp(i)+nu)/ tg.dx(i);
                Real pm = sigma2/(tg.dxm(i) * tg.dxp(i))+r;
                L.setMidRow(i, pd,pm,pu);
            }
        }
    };

    class PdeConstantCoeff : PdeSecondOrderParabolic  {
    public:
        PdeConstantCoeff(const PdeSecondOrderParabolic & pde,
                         Time t, Real x) {
          diffusion_ = pde.diffusion(t, x);
          drift_ = pde.drift(t, x);
          discount_ = pde.discount(t, x);
      }
      virtual Real diffusion(Time t, Real x) const {
          return diffusion_;
      }
      virtual Real drift(Time t, Real x) const {
          return drift_;
      }
      virtual Real discount(Time t, Real x) const {
          return discount_;
      }
    private:
        Real diffusion_; 
        Real drift_;
        Real discount_;
    };
}


#endif
