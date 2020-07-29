/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2014 Jose Aparicio

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

#ifndef quantlib_math_multidimintegrator_hpp
#define quantlib_math_multidimintegrator_hpp

#include <ql/types.hpp>
#include <ql/errors.hpp>
#include <ql/math/integrals/integral.hpp>
#include <ql/functional.hpp>
#include <vector>

namespace QuantLib {

    /*! \brief Integrates a vector or scalar function of vector domain. 
        
        Uses a collection of arbitrary 1D integrators along each of the 
        dimensions. A template recursion along dimensions avoids calling depth 
        test or virtual functions.\par
        This class generalizes to an arbitrary number of dimensions the 
        functionality in class TwoDimensionalIntegral  
    */
    class MultidimIntegral {
    public:
        explicit MultidimIntegral(
            const std::vector<ext::shared_ptr<Integrator> >& integrators);

        // scalar variant
        /*! f is the integrand function; a and b are the lower and
            upper integration limit domain for each dimension.
        */
        Real operator()(
            const ext::function<Real (const std::vector<Real>&)>& f,
            const std::vector<Real>& a,
            const std::vector<Real>& b) const 
        {
            QL_REQUIRE((a.size()==b.size())&&(b.size()==integrators_.size()), 
                "Incompatible integration problem dimensions");
            return integrationLevelEntries_[integrators_.size()-1](f, a, b);
        }
        // to do: write std::vector<Real> operator()(...) version

    private:
        static const Size maxDimensions_ = 15;

        /* Here is the tradeoff; this is avoiding the dimension limits checks 
        during integration at the price of these asignments during construction.
        Explicit template instantiation is of no use, an object is needed 
        (notice 'this' is needed for the asignment.)
        If not all the dimensions up the maximum number are used the waste goes
        into storage of the functions (in fact only one is used)
        */
        template<Size depth>
        void spawnFcts() const;
        // Splits the integration in cross-sections per dimension.
        template<int T_N> 
        Real vectorBinder (
            const ext::function<Real (const std::vector<Real>&)>& f,
            Real z,
            const std::vector<Real>& a,
            const std::vector<Real>& b) const ;
        // actual integration of dimension nT
        template<int nT>
        Real integrate(
            const ext::function<Real (const std::vector<Real>&)>& f,
            const std::vector<Real>& a,
            const std::vector<Real>& b) const;

        const std::vector<ext::shared_ptr<Integrator> > integrators_;

        /* typedef (const ext::function<Real 
            (const std::vector<Real>&arg1)>&arg2) integrableFunctType;
        */

        /* vector of, functions returning reals And taking as argument: 
        1.- a const ref to a function taking vectors 
        2.- a vector, 3. another vector. typedefs eventually...
         at first sight this might look like mimicking a virtual table, it isnt 
         that. The reason is to be able to select the correct integration 
         dimension at run time, this can not be done before because of the 
         template argument restriction to be constant known at compilation.
        */
        mutable std::vector<ext::function<Real (//<- members: integrate<N>
            // integrable function:
            const ext::function<Real (const std::vector<Real>&)>&, 
            const std::vector<Real>&, //<- a
            const std::vector<Real>&) //<- b
            > > 
            integrationLevelEntries_;

        /* One can avoid the passing around of the ct refs to a and b but the 
        price is to keep a copy of them (they are unknown at construction time)
         On the other hand the vector integration variable has to be created.*/
        mutable std::vector<Real> varBuffer_;

    };

    // spez last call/dimension
    template<>
    Real inline MultidimIntegral::vectorBinder<0> (
        const ext::function<Real (const std::vector<Real>&)>& f, 
        Real z,
        const std::vector<Real>& a,
        const std::vector<Real>& b) const
    {
        varBuffer_[0] = z;
        return f(varBuffer_);
    }

    template<>
    void inline MultidimIntegral::spawnFcts<1>() const {
        integrationLevelEntries_[0] = 
            ext::bind(&MultidimIntegral::integrate<0>, this,
                      ext::placeholders::_1, ext::placeholders::_2, ext::placeholders::_3);
    }

    template<int nT>
    inline Real MultidimIntegral::integrate(
        const ext::function<Real (const std::vector<Real>&)>& f,
        const std::vector<Real>& a,
        const std::vector<Real>& b) const 
    {
        return 
            (*integrators_[nT])(
                ext::bind(&MultidimIntegral::vectorBinder<nT>, this, f, 
                    ext::placeholders::_1, ext::cref(a), ext::cref(b)), a[nT], b[nT]);
    }

    template<int T_N> 
    inline Real MultidimIntegral::vectorBinder (
        const ext::function<Real (const std::vector<Real>&)>& f,
        Real z,
        const std::vector<Real>& a,
        const std::vector<Real>& b) const 
    {
        varBuffer_[T_N] = z;
        return integrate<T_N-1>(f, a, b);
    }

    template<Size depth>
    void MultidimIntegral::spawnFcts() const {
        integrationLevelEntries_[depth-1] =
          ext::bind(&MultidimIntegral::integrate<depth-1>, this, 
            ext::placeholders::_1, ext::placeholders::_2, ext::placeholders::_3);
        spawnFcts<depth-1>();
    }

}

#endif
