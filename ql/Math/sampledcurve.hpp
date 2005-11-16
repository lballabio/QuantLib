/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
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

/*! \file sampledcurve.hpp
    \brief a class that contains a sampled curve
*/

#ifndef quantlib_sampled_curve_hpp
#define quantlib_sampled_curve_hpp

#include <ql/Math/array.hpp>
#include <ql/grid.hpp>
#include <ql/Math/cubicspline.hpp>

namespace QuantLib {

    //! This class contains a sampled curve.
    /*! Initially the class will contain one indexed curve */
    class SampledCurve {
      public:
        SampledCurve(Size gridSize = 0);
        SampledCurve(const Array &grid);
        SampledCurve& operator=(const SampledCurve&);

        //! \name inspectors
        //@{
        const Array& grid() const;
        Array& grid();
        const Array& values() const;
        Array& values();
        Real gridValue(Size i) const;
        Real& gridValue(Size i);
        Real value(Size i) const;
        Real& value(Size i);
        Size size() const;
        bool empty() const;
        //@}

        //! \name modifiers
        //@{
        void setGrid(const Array&);
        void setValues(const Array&);
        template <class F>
        void sample(const F& f) {
            Array::iterator i, j;
            for(i=grid_.begin(), j = values_.begin(); 
                i != grid_.end(); i++, j++)
                *j = f(*i);
        }
        //@}

        //! \name calculations
        //@{
        /*! \todo replace or complement with a more general function
                  valueAt(spot)
        */
        Real valueAtCenter() const;
        /*! \todo replace or complement with a more general function
                  firstDerivativeAt(spot)
        */
        Real firstDerivativeAtCenter() const;
        /*! \todo replace or complement with a more general function
                  secondDerivativeAt(spot)
        */
        Real secondDerivativeAtCenter() const;
        //@}

        //! \name utilities
        //@{
        void swap(SampledCurve&);
        void setLogGrid(Real min, Real max) {
            setGrid(BoundedLogGrid(min, max, size()-1));
        }
        void regridLogGrid(Real min, Real max) {
            regrid(BoundedLogGrid(min, max, size()-1),
                   std::ptr_fun<Real,Real>(std::log));
        }
        void shiftGrid(Real s) {
            grid_ += s;
        }
        void regrid(const Array &new_grid);
        template <class T>
        void regrid(const Array &new_grid,
                    T func) {
            Array transformed_grid(grid_.size());
            
            std::transform(grid_.begin(), grid_.end(),
                           transformed_grid.begin(), func);
            NaturalCubicSpline priceSpline(transformed_grid.begin(), 
                                           transformed_grid.end(),
                                           values_.begin());

            values_ = new_grid;
            values_.transform(func);            
            for (Array::iterator j = values_.begin(); 
                 j != values_.end(); j++) {
                *j = priceSpline(*j, true);
            } 
            grid_ = new_grid;
        }

        template <class T>
        inline const SampledCurve& transform(T x) {
            values_.transform(x);
            return *this;
        }

        template <class T>
        inline const SampledCurve& transformGrid(T x) {
            grid_.transform(x);
            return *this;
        }
        //@}
      private:
        Array grid_;
        Array values_;
    };

    /* \relates SampledCurve */
    void swap(SampledCurve&, SampledCurve&);

    typedef SampledCurve SampledCurveSet;


    // inline definitions

    inline SampledCurve::SampledCurve(Size gridSize)
    : grid_(gridSize), values_(gridSize) {}

    inline SampledCurve::SampledCurve(const Array& grid) 
        : grid_(grid), values_(grid.size()) {}

    inline SampledCurve& SampledCurve::operator=(const SampledCurve& from) {
        SampledCurve temp(from);
        swap(temp);
        return *this;
    }

    inline Array& SampledCurve::grid() {
        return grid_;
    }

    inline const Array& SampledCurve::grid() const {
        return grid_;
    }

    inline const Array& SampledCurve::values() const {
        return values_;
    }

    inline Array& SampledCurve::values() {
        return values_;
    }

    inline Real SampledCurve::gridValue(Size i) const {
        return grid_[i];
    }

    inline Real& SampledCurve::gridValue(Size i) {
        return grid_[i];
    }

    inline Real SampledCurve::value(Size i) const {
        return values_[i];
    }

    inline Real& SampledCurve::value(Size i) {
        return values_[i];
    }

    inline Size SampledCurve::size() const {
        return grid_.size();
    }

    inline bool SampledCurve::empty() const {
        return grid_.empty();
    }

    inline void SampledCurve::setGrid(const Array &g) {
        grid_ = g;
    }

    inline void SampledCurve::setValues(const Array &g) {
        values_ = g;
    }

    inline void SampledCurve::swap(SampledCurve& from) {
        using std::swap;
        grid_.swap(from.grid_);
        values_.swap(from.values_);
    }

    inline void swap(SampledCurve& c1, SampledCurve& c2) {
        c1.swap(c2);
    }

    inline std::ostream& operator<<(std::ostream& out, 
                                    const SampledCurve& a) {
        out << "[ " << a.grid() << "; " 
            << a.values() << " ]";
        return out;
    }


}


#endif

