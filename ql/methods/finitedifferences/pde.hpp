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

/*! \file pde.hpp
    \brief General class for one dimensional PDE's
*/

#ifndef quantlib_pde_hpp
#define quantlib_pde_hpp

#include <ql/math/array.hpp>
#include <ql/methods/finitedifferences/tridiagonaloperator.hpp>
#include <ql/math/transformedgrid.hpp>

namespace QuantLib {
    class PdeSecondOrderParabolic {
    public:
      virtual ~PdeSecondOrderParabolic() = default;
      virtual Real diffusion(Time t, Real x) const = 0;
      virtual Real drift(Time t, Real x) const = 0;
      virtual Real discount(Time t, Real x) const = 0;
      virtual void
      generateOperator(Time t, const TransformedGrid& tg, TridiagonalOperator& L) const {
          for (Size i = 1; i < tg.size() - 1; i++) {
              Real sigma = diffusion(t, tg.grid(i));
              Real nu = drift(t, tg.grid(i));
              Real r = discount(t, tg.grid(i));
              Real sigma2 = sigma * sigma;

              Real pd = -(sigma2 / tg.dxm(i) - nu) / tg.dx(i);
              Real pu = -(sigma2 / tg.dxp(i) + nu) / tg.dx(i);
              Real pm = sigma2 / (tg.dxm(i) * tg.dxp(i)) + r;
              L.setMidRow(i, pd, pm, pu);
          }
        }
    };

    template <class PdeClass>
    class PdeConstantCoeff : public PdeSecondOrderParabolic  {
    public:
        PdeConstantCoeff(const typename PdeClass::argument_type &process,
                         Time t, Real x) {
            PdeClass pde(process);
            diffusion_ = pde.diffusion(t, x);
            drift_ = pde.drift(t, x);
            discount_ = pde.discount(t, x);
        }
        Real diffusion(Time, Real) const override { return diffusion_; }
        Real drift(Time, Real) const override { return drift_; }
        Real discount(Time, Real) const override { return discount_; }

      private:
        Real diffusion_;
        Real drift_;
        Real discount_;
    };

    template <class PdeClass>
    class GenericTimeSetter:public TridiagonalOperator::TimeSetter {
    public:
        template <class T>
        GenericTimeSetter(const Array &grid, T process) :
            grid_(grid), pde_(process) {}
        void setTime(Time t, TridiagonalOperator& L) const override {
            pde_.generateOperator(t, grid_, L);
        }

    private:
        typename PdeClass::grid_type grid_;
        PdeClass pde_;
    };

    template <class PdeClass>
    class PdeOperator:public TridiagonalOperator {
    public:
        template <class T>
        PdeOperator(const Array& grid,
                    T process,
                    Time residualTime = 0.0) :
            TridiagonalOperator(grid.size()) {
            timeSetter_ =
                ext::shared_ptr<GenericTimeSetter<PdeClass> >(
                     new GenericTimeSetter<PdeClass>(grid, process));
            setTime(residualTime);
        }
    };
}


#endif


#ifndef id_c3098599f1ab6e69fa9aa064cf9151b7
#define id_c3098599f1ab6e69fa9aa064cf9151b7
inline bool test_c3098599f1ab6e69fa9aa064cf9151b7(const int* i) {
    return i != nullptr;
}
#endif
