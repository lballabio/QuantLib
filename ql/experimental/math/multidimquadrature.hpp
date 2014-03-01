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

#ifndef quantlib_math_multidimquadrature_hpp
#define quantlib_math_multidimquadrature_hpp

#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/lambda/bind.hpp>

#include <ql/types.hpp>
#include <ql/math/integrals/gaussianquadratures.hpp>

namespace QuantLib {

    /*! \brief Integrates a vector or scalar function of vector domain. 
        
        A template recursion along dimensions avoids calling depth 
        test or virtual functions.

        Test example:
        \code{.cpp}
        #include <boost/function.hpp>
        #include <ql/experimental/math/multidiminquadrature.hpp>

        using namespace QuantLib;
        using namespace std;
        
        // Correct value is: (e^{-.25} \sqrt{\pi})^{dim}
        struct integrand {
            Real operator()(const std::vector<Real>& arg) const {
                Real sum = 1.;
                for(Size i=0; i<arg.size(); i++) 
                    sum *= std::exp(-arg[i]*arg[i]) * std::cos(arg[i]);
                return sum;
            }
        };

        int main() {
            boost::function<Real (const std::vector<Real>& arg)> f;
            f = integrand();
            Size dim = 8;
            GaussianQuadMultidimIntegrator intg(dim, 15);
            Real value = intg(f);
            return 0;
        }
        \endcode

        \todo Add coherence test between the integrand function dimensions (the
        vector size) and the declared dimension in the constructor.

        \todo Split into integrator for functions returning scalar and vector?
    */
    class GaussianQuadMultidimIntegrator {
    private:
        typedef Disposable<std::vector<Real> > DispArray;
        // Vector integration. Quadrature to functions returning a vector of 
        // real numbers, turns 1D quadratures into ND
        class VectorIntegrator : public GaussHermiteIntegration {
        public:
            VectorIntegrator(Size n, Real mu = 0.0) 
            : GaussHermiteIntegration(n, mu) {}

            template <class F> // todo: fix copies.
                DispArray operator()(const F& f) const {
                //first one, we do not know the size of the vector returned by f
                Integer i = order()-1;
                std::vector<Real> term = f(x_[i]);// potential copy! @#$%^!!!
                std::for_each(term.begin(), term.end(), 
                    std::bind1st(std::multiplies<Real>(), w_[i]));
                std::vector<Real> sum = term;
           
                for (i--; i >= 0; --i) {
                    term = f(x_[i]);// potential copy! @#$%^!!!
                    // sum[j] += term[j] * w_[i];
                    std::transform(term.begin(), term.end(), sum.begin(), 
                        sum.begin(), 
                        boost::bind(std::plus<Real>(), _2,
                            boost::bind(std::multiplies<Real>(), w_[i], _1)));
                }
                return sum;
            }
        };

    public:
        /*!
            @param dimension The number of dimensions of the argument of the 
            function we want to integrate.
            @param dimension Integration variable dimension.
            @param mu Parameter in the Gauss Hermite weight (i.e. points load).
        */
        GaussianQuadMultidimIntegrator(Size dimension, Size quadOrder, 
            Real mu = 0.);
        //! Integration quadrature order.
        Size order() const {return integralV_.order();}

        //! Integrates function f over \f[ R^{dim} \f]
        /* This function is just syntax since the only thing it does is calling 
        to integrate<RetType> which has to exist for the type returned by the 
        function. So theres one redundant call but there should not be any extra 
        cost... up to the compiler. It can not be templated all the way since
        the integration entries functions can not be templates.
        Most times integrands will return a scalar or vector but could be a 
        matrix too. Also vectors might be returned as vector or Disposable 
        wrapped (which is preferred and I have removed the plain vector
        version).
         */
        template<class RetType_T>
        RetType_T operator()(const boost::function<RetType_T (
            const std::vector<Real>& arg)>& f) const 
        {
            return integrate<RetType_T>(f);
        }
    private:
        // Declare, spezializations follow.
        template<class RetType_T>
        RetType_T integrate(const boost::function<RetType_T (
            const std::vector<Real>& v1)>& f) const;

        // Scalar integrand version (merge with vector case?)
        template<>
        Real integrate<Real>(const boost::function<Real (
            const std::vector<Real>& v1)>& f) const 
        {
            // integration variables
            std::fill(integrandFctrs.begin(), integrandFctrs.end(), 0.);
            // call vector quadrature integration with the function and start 
            // values, kicks in recursion over the dimensions of the integration
            // variable.
            return integral_(boost::bind(
                       // integration entry level is selected now
                       integrationEntries_[dimension_-1],
                       boost::cref(f),
                       boost::ref(integrandFctrs),
                       _1)
                       );
        }
        // Vector integrand version
        template<>
        DispArray integrate<DispArray>(
            const boost::function<DispArray (
                const std::vector<Real>& v1)>& f) const 
        {
            std::fill(integrandFctrs.begin(), integrandFctrs.end(), 0.);

            return integralV_(boost::bind(
                       boost::cref(integrationEntriesVR_[dimension_-1]),
                       boost::cref(f),
                       boost::ref(integrandFctrs),
                       _1)
                       );
        } 

        template <int intgDepth>
        Real scalarIntegrator(
            boost::function<Real (const std::vector<Real>& arg1)> f, 
            std::vector<Real>& mFactrs, 
            const Real mFctr) const 
        {
            mFactrs[dimension_-intgDepth] = mFctr;
            return integral_(boost::bind(
                &GaussianQuadMultidimIntegrator::scalarIntegrator<intgDepth-1>,
                this,
                f,
                boost::ref(mFactrs),
                _1)
            );
        }
        //! Terminal integrand; scalar function version
        template<> 
        Real scalarIntegrator<1>(
            boost::function<Real (const std::vector<Real>& arg1)> f,
            std::vector<Real>& mFactrs, 
            const Real mFctr) const
        {
            mFactrs.back() = mFctr;
            return f(mFactrs);
        }

        template <int intgDepth>
        DispArray vectorIntegratorVR(
            const boost::function<DispArray(const std::vector<Real>& arg1)>& f,
            std::vector<Real>& mFactrs, 
            const Real mFctr) const 
        {
            mFactrs[dimension_-intgDepth] = mFctr;
            return 
              integralV_(boost::bind(
               &GaussianQuadMultidimIntegrator::vectorIntegratorVR<intgDepth-1>,
               this,
               f,
               boost::ref(mFactrs),
               _1)
            );
        }
        //! Terminal integrand; disposable vector function version
        // Notice it has to match with the type in the boost function.
        template<>
        DispArray
            vectorIntegratorVR<1>(
            const boost::function<DispArray (const std::vector<Real>& arg1)>& f,
            std::vector<Real>& mFactrs, 
            const Real mFctr) const 
        {
            mFactrs.back() = mFctr;
            return f(mFactrs);
        }

        // non static by design 
        // Same object for all dimensions poses problems when using the 
        //   parallelized integrals version.
        //! The actual integrators.
        GaussHermiteIntegration integral_;
        VectorIntegrator integralV_;

        //! \name Integration entry points generation
        //@{
        /* The maximum number of dimensions of the integration variable domain
            A higher than this number of dimension would presumably be 
           impractical and another integration algorithm (MC) should be 
           considered.
           \to do Consider moving it to a library configuration variable.
        */
        static const Size maxNumFactors_ = 30;

        //! Buffer to allow acces to integrations. We do not know at which 
        //    level/dimension we are going to start integration
        // \todo Declare typedefs for traits
        mutable std::vector<
        boost::function<Real (boost::function<Real (
            const std::vector<Real>& varg2)> f1, 
            std::vector<Real>& v2, const Real r3)> > integrationEntries_;
        mutable std::vector<
        boost::function<DispArray (const boost::function<DispArray(
            const std::vector<Real>& vvarg2)>& vf1, 
            std::vector<Real>& vv2, const Real vr3)> > integrationEntriesVR_;

        //! Recursive template methods to statically generate (at this 
        //    class construction time) handles to the integration entry points
        template<Size levelSpawn>
        void spawnFcts() const {
            integrationEntries_[levelSpawn-1] = 
                boost::bind(
                &GaussianQuadMultidimIntegrator::scalarIntegrator<levelSpawn>, 
                    this, _1, _2, _3);
            integrationEntriesVR_[levelSpawn-1] = 
                boost::bind(
                &GaussianQuadMultidimIntegrator::vectorIntegratorVR<levelSpawn>, 
                    this, _1, _2, _3);
            spawnFcts<levelSpawn-1>();
        }
        // terminal level:
        template<>
        void spawnFcts<1>() const {
            integrationEntries_[0] = 
              boost::bind(&GaussianQuadMultidimIntegrator::scalarIntegrator<1>, 
              this, _1, _2, _3);
            integrationEntriesVR_[0] = 
             boost::bind(&GaussianQuadMultidimIntegrator::vectorIntegratorVR<1>, 
             this, _1, _2, _3);
        }
        //@}
        Size dimension_;
        // integration veriable buffer
        mutable std::vector<Real> integrandFctrs;
    };

}

#endif
