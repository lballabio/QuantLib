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

    /*! \deprecated Part of the old FD framework; copy this function
                    in your codebase if needed.
                    Deprecated in version 1.37.
    */
    class [[deprecated("Part of the old FD framework; copy this function in your codebase if needed")]] PdeSecondOrderParabolic {
    public:
      virtual ~PdeSecondOrderParabolic() = default;
      virtual Real diffusion(Time t, Real x) const = 0;
      virtual Real drift(Time t, Real x) const = 0;
      virtual Real discount(Time t, Real x) const = 0;
      QL_DEPRECATED_DISABLE_WARNING
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

    /*! \deprecated Part of the old FD framework; copy this function
                    in your codebase if needed.
                    Deprecated in version 1.37.
    */
    template <class PdeClass>
    class [[deprecated("Part of the old FD framework; copy this function in your codebase if needed")]] PdeConstantCoeff : public PdeSecondOrderParabolic  {
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

    QL_DEPRECATED_ENABLE_WARNING

    /*! \deprecated Part of the old FD framework; copy this function
                    in your codebase if needed.
                    Deprecated in version 1.37.
    */
    template <class PdeClass>
    class [[deprecated("Part of the old FD framework; copy this function in your codebase if needed")]] GenericTimeSetter:public TridiagonalOperator::TimeSetter {
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

    /*! \deprecated Part of the old FD framework; copy this function
                    in your codebase if needed.
                    Deprecated in version 1.37.
    */
    template <class PdeClass>
    class [[deprecated("Part of the old FD framework; copy this function in your codebase if needed")]] PdeOperator:public TridiagonalOperator {
    public:
        template <class T>
        PdeOperator(const Array& grid,
                    T process,
                    Time residualTime = 0.0) :
            TridiagonalOperator(grid.size()) {
            QL_DEPRECATED_DISABLE_WARNING
            timeSetter_ =
                ext::shared_ptr<GenericTimeSetter<PdeClass> >(
                     new GenericTimeSetter<PdeClass>(grid, process));
            QL_DEPRECATED_ENABLE_WARNING
            setTime(residualTime);
        }
    };
}


#endif
