#ifndef quantlib_math_multidimintegrator_hpp
#define quantlib_math_multidimintegrator_hpp

#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/lambda/bind.hpp>

#include <ql/types.hpp>
#include <ql/math/integrals/gaussianquadratures.hpp>

#include <ql/math/integrals/integral.hpp>

namespace QuantLib {

    /*!
        Integrates a separable vector function of vector domain. Separability 
        enables a template recursion along dimensions without calling depth 
        test or virtual functions.
    */

    /* 
    - Adapt this to a parametric basic integrator, do it so it 
        works at least with two basic ones: GaussHermiteIntegration and a 
        trapezoid.

    - I am copying the function to integrate in the function call, while it 
        should be a reference.

    */
    /*! Particular integration case with infinite domain using a quadrature

        \todo Allow other domains by leaving the quadrature type as a parameter
    */
    /*
        Test example:
        \code{.cpp}
        #include <iostream>
        #include <iomanip>
        #include <boost/function.hpp>
        #include <ql/experimental/math/multidimintegrator.hpp>

        using namespace QuantLib;
        using namespace std;
        
        // Correct value is: (e^{-.25} \sqrt{\pi})^{dim}
        struct integrand {
            Real operator()(const std::vector<Real>& arg) const {
                Real sum = 1.;
                for(Size i=0; i<arg.size(); i++) 
                    sum *= std::exp(-arg[i]*arg[i]) * std::cos(arg[i]);/////////////////////////accumulate
                return sum;
            }
        };

        int main() {
            boost::function<Real (const std::vector<Real>& arg)> f;
            f = integrand();
            Size dim = 8;
            GaussianQuadMultidimIntegrator intg(dim, 15);
            Real value = intg.integrate(f);//////////////////////////////////////consider operator()
            return 0;
        }
        \endcode

        \todo Put the recursive call mechanism into a base class and derive
        classes for quadratures, simpson type etc through thte inner vector
        integrator. Or set the 1D underlying integrator as a T arg.

        \todo Add coherence test between the integrand function dimensions (the
        vector size) and the declared dimension in the constructor.
    */
    class GaussianQuadMultidimIntegrator {
        // in the future rely on move-semantics/rvals
        typedef Disposable<std::vector<Real> > DispArray;
    private:
        // Vector integration. Quadrature to functions returning a vector of 
        // real numbers.
        class VectorIntegrator : public GaussHermiteIntegration {
        public:
            VectorIntegrator(Size n, Real mu = 0.0) 
            : GaussHermiteIntegration(n, mu) {}

            template <class F>
                Disposable<std::vector<Real> > operator()(const F& f) const {
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
        template<class RetType_T>
        RetType_T operator()(boost::function<RetType_T (
            const std::vector<Real>& arg)> f) 
        {
            return integrate<RetType_T>(f);
        }
        // template template now? (of op() )

        //! Declare, spezializations follow.
        /*
        template<class RetType_T>
        RetType_T integrate(boost::function<RetType_T (
            const std::vector<Real>& v1)> f) const;
        */
        template<class RetType_T>
        RetType_T integrate(const boost::function<RetType_T (
            const std::vector<Real>& v1)>& f) const;

        //////template<class RetType_T>
        //////RetType_T integrate(const F& f) const;
        //\todo: write pointer version

        // Scalar integrand version (merge with vector case?)
        template<>
        Real integrate<Real>(const boost::function<Real (
            const std::vector<Real>& v1)>& f) const 
        {//QL_REQUIRE(f && !f.empty());
            // integration variables
            std::fill(integrandFctrs.begin(), integrandFctrs.end(), 0.);
            // call vector quadrature integration with the function and start 
            // values, kicks in recursion over the dimensions of the integration
            // variable.
            return integral_(boost::bind(
                       // integration entry level is selected now
                       integrationEntries_[dimension_-1],
                       f,
                       boost::ref(integrandFctrs),
                       _1)
                       );
        }
        /*
        // Vector integrand version
        template<>
        std::vector<Real> integrate<std::vector<Real> >(
              boost::function<std::vector<Real> (
                const std::vector<Real>& v1)> f) const 
        {
            std::fill(integrandFctrs.begin(), integrandFctrs.end(), 0.);
            return integralV_(boost::bind(
                       // integration entry level is selected now
                       integrationEntriesV_[dimension_-1],
                       f,
                       boost::ref(integrandFctrs),
                       _1)
                       );
        } 
*/
        template<>
        DispArray integrate<DispArray>(
            const boost::function<DispArray (const std::vector<Real>& v1)>& f) const ////////////////////////////////////////////
        {
            std::fill(integrandFctrs.begin(), integrandFctrs.end(), 0.);

            return integralV_(boost::bind(
                       boost::cref(integrationEntriesVR_[dimension_-1]),/// cref with no effect
                       boost::cref(f),///////////////////////////////////////////////////////////////////////////
                       boost::ref(integrandFctrs),
                       _1)
                       );
        } 
/*
        template<class F>
        DispArray integrateF(
            const F& f) const ////////////////////////////////////////////
        {
            std::fill(integrandFctrs.begin(), integrandFctrs.end(), 0.);

            return integralV_(boost::bind(
                       integrationEntriesVR_[dimension_-1],
                       boost::cref(f),///////////////////////////////////////////////////////////////////////////
                       boost::ref(integrandFctrs),
                       _1)
                       );
        } 
*/
    private:
        template <int integrandDepth>
        Real scalarIntegrator(
            boost::function<Real (const std::vector<Real>& arg1)> f, 
            std::vector<Real>& mFactrs, 
            const Real mFctr) const 
        {
            mFactrs[dimension_-integrandDepth] = mFctr;
            return integral_(boost::bind(
                &GaussianQuadMultidimIntegrator::scalarIntegrator<integrandDepth-1>,
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
/*
        template <int integrandDepth>
        std::vector<Real> vectorIntegrator(
            boost::function<std::vector<Real>(const std::vector<Real>& arg1)> f, 
            std::vector<Real>& mFactrs, 
            const Real mFctr) const 
        {
            mFactrs[dimension_-integrandDepth] = mFctr;
            return 
              integralV_(boost::bind(
                &GaussianQuadMultidimIntegrator::vectorIntegrator<integrandDepth-1>,
                this,
                f,
                boost::ref(mFactrs),
                _1)
            );
        }

        //! Terminal integrand; vector function version
        template<>
        std::vector<Real>
            vectorIntegrator<1>(
            boost::function<std::vector<Real> (const std::vector<Real>& arg1)> f,
            std::vector<Real>& mFactrs, 
            const Real mFctr) const 
        {
            mFactrs.back() = mFctr;
            return f(mFactrs);
        }
*/
        template <int integrandDepth>
        DispArray vectorIntegratorVR(
            const boost::function<DispArray(const std::vector<Real>& arg1)>& f,///////////////////////////////////////////////
            std::vector<Real>& mFactrs, 
            const Real mFctr) const 
        {
            mFactrs[dimension_-integrandDepth] = mFctr;
            return 
              integralV_(boost::bind(
                &GaussianQuadMultidimIntegrator::vectorIntegratorVR<integrandDepth-1>,
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
        // The maximum number of factors (or size of the correlation matrix) 
        //  this model supports <-in the context of the latent variable model.
        //! The maximum number of dimensions of the integration variable domain
        // \to do Consider moving it to a library configuration variable.
        static const Size maxNumFactors_;
        //! Buffer to allow acces to integrations. We do not know at which 
        //    level/dimension we are going to start integration
        // \todo Declare typedefs for traits
        mutable std::vector<
        boost::function<Real (boost::function<Real (
            const std::vector<Real>& varg2)> f1, 
            std::vector<Real>& v2, const Real r3)> > integrationEntries_;
/*
        mutable std::vector<
        boost::function<std::vector<Real> (boost::function<std::vector<Real> (
            const std::vector<Real>& vvarg2)> vf1, 
            std::vector<Real>& vv2, const Real vr3)> > integrationEntriesV_;
*/
        mutable std::vector<
        boost::function<DispArray (const boost::function<DispArray(
            const std::vector<Real>& vvarg2)>& vf1, ///////////////---------------const ref no effect
            std::vector<Real>& vv2, const Real vr3)> > integrationEntriesVR_;


        //! Recursive template methods to statically generate (at this 
        //    class construction time) handles to the integration entry points
        template<Size levelSpawn>
        void spawnFcts() const {
            integrationEntries_[levelSpawn-1] = 
                boost::bind(
                &GaussianQuadMultidimIntegrator::scalarIntegrator<levelSpawn>, 
                    this, _1, _2, _3);
            /*
            integrationEntriesV_[levelSpawn-1] = 
                boost::bind(
                &GaussianQuadMultidimIntegrator::vectorIntegrator<levelSpawn>, 
                    this, _1, _2, _3);
                    */
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
/*
            integrationEntriesV_[0] = 
              boost::bind(&GaussianQuadMultidimIntegrator::vectorIntegrator<1>, 
              this, _1, _2, _3);
            */
            integrationEntriesVR_[0] = 
              boost::bind(&GaussianQuadMultidimIntegrator::vectorIntegratorVR<1>, 
              this, _1, _2, _3);

        }
        //@}
        Size dimension_;
        // integration veriable buffer
        mutable std::vector<Real> integrandFctrs;
    };










    class MultidimIntegral {
    public:
        MultidimIntegral(
            const std::vector<boost::shared_ptr<Integrator> >& integrators);

        // scalar variant:
        Real operator()(
            const boost::function<Real (const std::vector<Real>&)>& f,
            const std::vector<Real>& a,
            const std::vector<Real>& b) const 
        {
            QL_REQUIRE((a.size()==b.size())&&(b.size()==integrators_.size()), 
                "Incompatible integration problem dimensions");
            varBuffer_.resize(a.size());
            return integrationLevelEntries_[integrators_.size()-1](f, a, b);
        }
        // to do: write std::vector<Real> operator()(...) version

    private:
        static const Size maxDimensions_ = 30;

        /* Here is the tradeoff; this is avoiding the dimension limits checks 
        during integration at the price of these asignments during construction.
        Explicit template instantiation is of no use, an object is needed 
        (notice 'this' is needed for the asignment.)
        If not all the dimensions up the maximum number are used the waste goes
        into storage of the functions (in fact only one is used)
        */
        template<Size depth>
        void spawnFcts() const {
            integrationLevelEntries_[depth] =
             boost::bind(&MultidimIntegral::integrate<depth>, this, _1, _2, _3);
            spawnFcts<depth-1>();
        }
        template<>
        void spawnFcts<0>() const {
            integrationLevelEntries_[0] = 
                boost::bind(&MultidimIntegral::integrate<0>, this, _1, _2, _3);
        }

        // Splits the integration in cross-sections per dimension.
        template<int T_N> 
        Real vectorBinder (
            const boost::function<Real (const std::vector<Real>&)>& f,
            Real z,
            const std::vector<Real>& a,
            const std::vector<Real>& b) const 
        {
            varBuffer_[T_N] = z;
            return integrate<T_N-1>(f, a, b);
        }

        // spez last call/dimension
        template<>
        Real vectorBinder<0> (
            const boost::function<Real (const std::vector<Real>&)>& f, 
            Real z,
            const std::vector<Real>& a,
            const std::vector<Real>& b) const
        {
            varBuffer_[0] = z;
            return f(varBuffer_);
        }
        
        // actual integration of dimension nT
        template<int nT>
        Real integrate(
            const boost::function<Real (const std::vector<Real>&)>& f,
            const std::vector<Real>& a,
            const std::vector<Real>& b) const 
        {
            return 
                (*integrators_[nT])(
                    boost::bind(&MultidimIntegral::vectorBinder<nT>, this, f, 
                        _1, boost::cref(a), boost::cref(b)), a[nT], b[nT]);
        }
    private:
        const std::vector<boost::shared_ptr<Integrator> > integrators_;

        /* One can avoid the passing around of the ct refs to a and b but the 
        price is to keep a copy of them (they are unknown at construction time)
         On the other hand the vector integration variable has to be created.*/
        mutable std::vector<Real> varBuffer_;

        /* typedef (const boost::function<Real 
            (const std::vector<Real>&arg1)>&arg2) integrableFunctType;
        */

        /* vector of, functions returning reals And taking as argument: 
        1.- a const ref to a function taking vectors 
        2.- a vector, 3. another vector. typedefs eventually...
         at first sight this might look like mimicking a virtual table, it isnt 
         that. The reason is to be able to select the correct integration 
         dimension at run time, this can be done before because of the template 
         argument restriction to be constant known at compilation.
        */
        mutable std::vector<boost::function<Real (//<- members: integrate<N>
            // integrable function:
            const boost::function<Real (const std::vector<Real>&)>&, 
            const std::vector<Real>&, //<- a
            const std::vector<Real>&) //<- b
            > > 
            integrationLevelEntries_;
        //Pointers to functions?
    };

}

#endif
