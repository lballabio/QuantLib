/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Roland Lichters
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

#ifndef quantlib_latent_model_hpp
#define quantlib_latent_model_hpp

#include <ql/experimental/math/multidimquadrature.hpp>
#include <ql/experimental/math/multidimintegrator.hpp>
#include <ql/math/integrals/trapezoidintegral.hpp>
#include <ql/math/randomnumbers/randomsequencegenerator.hpp>
#include <ql/experimental/math/gaussiancopulapolicy.hpp>
#include <ql/experimental/math/tcopulapolicy.hpp>
#include <ql/math/randomnumbers/boxmullergaussianrng.hpp>
#include <ql/experimental/math/polarstudenttrng.hpp>
#include <ql/handle.hpp>
#include <ql/quote.hpp>
#include <vector>

/*! \file latentmodel.hpp
    \brief Generic multifactor latent variable model.
*/

namespace QuantLib {

    namespace detail {
        // havent figured out how to do this in-place
        struct multiplyV {
            std::vector<Real> operator()(Real d, std::vector<Real> v) 
            {
                std::transform(v.begin(), v.end(), v.begin(), 
                               [=](Real x) -> Real { return x * d; });
                return v;
            }
        };
    }

    //! \name Latent model direct integration facility.
    //@{
    /* Things trying to achieve here:
    1.- Unify the two branches of integrators in the library, they do not 
      hang from a common base class and here a common ptr for the 
      factory is needed.
    2.- Have a common signature for the integration call.
    3.- Factory construction so integrable latent models can choose the 
      integration algorithm separately.
    */
    class LMIntegration {
    public:
        // Interface with actual integrators:
        // integral of a scalar function
        virtual Real integrate(const std::function<Real (
            const std::vector<Real>& arg)>& f) const = 0;
        // integral of a vector function
        /* I had to use a different name, since the compiler does not
        recognise the overload; MSVC sees the argument as 
        std::function<Signature> in both cases....   
        I could do the as with the quadratures and have this as a template 
        function and spez for the vector case but I prefer to understand
        why the overload fails....
                    FIX ME
        */
        virtual std::vector<Real> integrateV(
            const std::function<std::vector<Real>  (
            const std::vector<Real>& arg)>& f) const {
            QL_FAIL("No vector integration provided");
        }
        virtual ~LMIntegration() = default;
    };

    //CRTP-ish for joining the integrations, class above to have the factory
    template <class I_T>
    class IntegrationBase : 
        public I_T, public LMIntegration {// diamond on 'integrate'
     // this class template always to be fully specialized:
     private:
       IntegrationBase() = default;
    };
    //@}
    
    // gcc reports value collision with heston engine (?!) thats why the name
    namespace LatentModelIntegrationType {
        typedef 
        enum LatentModelIntegrationType {
            #ifndef QL_PATCH_SOLARIS
            GaussianQuadrature,
            #endif
            Trapezoid
            // etc....
        } LatentModelIntegrationType;
    }

    #ifndef QL_PATCH_SOLARIS

    /* class template specializations. I havent use CRTP type cast directly
    because the signature of the integrators is different, grid integration
    needs the domain. */
    template<> class IntegrationBase<GaussianQuadMultidimIntegrator> : 
    public GaussianQuadMultidimIntegrator, public LMIntegration {
    public:
        IntegrationBase(Size dimension, Size order) 
        : GaussianQuadMultidimIntegrator(dimension, order) {}
        Real integrate(const std::function<Real(const std::vector<Real>& arg)>& f) const override {
            return GaussianQuadMultidimIntegrator::integrate<Real>(f);
        }
        std::vector<Real> integrateV(
            const std::function<std::vector<Real>(const std::vector<Real>& arg)>& f)
            const override {
            return GaussianQuadMultidimIntegrator::integrate<std::vector<Real>>(f);
        }
        ~IntegrationBase() override = default;
    };

    #endif

    template<> class IntegrationBase<MultidimIntegral> : 
        public MultidimIntegral, public LMIntegration {
    public:
        IntegrationBase(
            const std::vector<ext::shared_ptr<Integrator> >& integrators, 
            Real a, Real b) 
        : MultidimIntegral(integrators), 
          a_(integrators.size(),a), b_(integrators.size(),b) {}
        Real integrate(const std::function<Real(const std::vector<Real>& arg)>& f) const override {
            return MultidimIntegral::operator()(f, a_, b_);
        }
        // vector version here....
        ~IntegrationBase() override = default;
        const std::vector<Real> a_, b_;
    };

    // Intended to replace OneFactorCopula

    /*!
    \brief Generic multifactor latent variable model.\par
        In this model set up one considers latent (random) variables 
        \f$ Y_i \f$ described by:
        \f[
        \begin{array}{ccccc}
        Y_1 & = & \sum_k M_k a_{1,k} & + \sqrt{1-\sum_k a_{1,k}^2} Z_1 & 
            \sim \Phi_{Y_1}\nonumber \\
        ... & = &      ... & ...   & \nonumber \\
        Y_i & = & \sum_k M_k a_{i,k} & + \sqrt{1-\sum_k a_{i,k}^2} Z_i & 
            \sim \Phi_{Y_i}\nonumber \\
        ... & = &      ... & ...   & \nonumber \\
        Y_N & = & \sum_k M_k a_{N,k} & + \sqrt{1-\sum_k a_{N,k}^2} Z_N & 
            \sim \Phi_{Y_N}
        \end{array}
        \f]
        where the systemic \f$ M_k \f$ and idiosyncratic \f$ Z_i \f$ (this last 
        one known as error term in some contexts) random variables have 
        independent zero-mean unit-variance distributions. A restriction of the 
        model implemented here is that the N idiosyncratic variables all follow 
        the same probability law \f$ \Phi_Z(z)\f$ (but they are still 
        independent random variables) Also the model is normalized 
        so that: \f$-1\leq a_{i,k} \leq 1\f$ (technically the \f$Y_i\f$ are 
        convex linear combinations). The correlation between \f$Y_i\f$ and 
        \f$Y_j\f$ is then \f$\sum_k a_{i,k} a_{j,k}\f$. 
        \f$\Phi_{Y_i}\f$ denotes the cumulative distribution function of 
        \f$Y_i\f$ which in general differs for each latent variable.\par
        In its single factor set up this model is usually employed in derivative
        pricing and it is best to use it through integration of the desired 
        statistical properties of the model; in its multifactorial version (with
        typically around a dozen factors) it is used in the context of portfolio
        risk metrics; because of the number of variables it is best to opt for a
        simulation to compute model properties/magnitudes. 
        For this reason this class template provides a random factor sample 
        interface and an integration interface that will be instantiated by 
        derived concrete models as needed. The class is neutral on the 
        integration and random generation algorithms\par
        The latent variables are typically treated as unobservable magnitudes 
        and they serve to model one or several magnitudes related to them 
        through some function
        \f[
        \begin{array}{ccc}
        F_i(Y_i) & = & 
            F_i(\sum_k M_k a_{i,k} + \sqrt{1-\sum_k a_{i,k}^2} Z_i )\nonumber \\
        & = & F_i(M_1,..., M_k, ..., M_K, Z_i)
        \end{array}
        \f]
        The transfer function can have a more generic form: 
        \f$F_i(Y_1,....,Y_N)\f$ but here the model is restricted to a one to 
        one relation between the latent variables and the modelled ones. Also 
        it is assumed that \f$F_i(y_i; \tau)\f$ is monotonic in \f$y_i\f$; it 
        can then be inverted and the relation of the cumulative probability of 
        \f$F_i\f$ and \f$Y_i\f$ is simple:
        \f[
        \int_{\infty}^b \phi_{F_i} df = 
            \int_{\infty}^{F_i^{-1}(b)} \phi_{Y_i} dy
        \f]
        If  \f$t\f$ is some value of the functional or modelled variable, 
        \f$y\f$ is mapped to \f$t\f$ such that percentiles match, i.e. 
        \f$F_Y(y)=Q_i(t)\f$ or \f$y=F_Y^{-1}(Q_i(t))\f$.
        The class provides an integration facility of arbitrary functions 
        dependent on the model states. It also provides random number generation
        interfaces for usage of the model in monte carlo simulations.\par
        Now let \f$\Phi_Z(z)\f$ be the cumulated distribution function of (all 
        equal as mentioned) \f$Z_i\f$. For a given realization of \f$M_k\f$, 
        this determines the distribution of \f$y\f$:
        \f[
        Prob \,(Y_i < y|M_k) = \Phi_Z \left( \frac{y-\sum_k a_{i,k}\,M_k}
            {\sqrt{1-\sum_k a_{i,k}^2}}\right)
        \qquad
        \mbox{or}
        \qquad
        Prob \,(t_i < t|M) = \Phi_Z \left( \frac
            {F_{Y_{i}}^{-1}(Q_i(t))-\sum_k a_{i,k}\,M_k}
            {\sqrt{1-\sum_k a_{i,k}^2}}
        \right)
        \f]
        The distribution functions of \f$ M_k, Z_i \f$ are specified in
        specific copula template classes. The distribution function 
        of \f$ Y_i \f$ is then given by the convolution
        \f[
        F_{Y_{i}}(y) = Prob\,(Y_i<y) = 
        \int_{-\infty}^\infty\,\cdots\,\int_{-\infty}^{\infty}\:
        D_Z(z)\,\prod_k D_{M_{k}}(m_k) \quad
        \Theta \left(y - \sum_k a_{i,k}m_k - 
            \sqrt{1-\sum_k a_{i,k}^2}\,z\right)\,d\bar{m}\,dz,
        \qquad
        \Theta (x) = \left\{
        \begin{array}{ll}
        1 & x \geq 0 \\
        0 & x < 0
        \end{array}\right.
        \f]
        where \f$ D_Z(z) \f$ and \f$ D_M(m) \f$ are the probability
        densities of \f$ Z\f$ and \f$ M, \f$ respectively.\par
        This convolution can also be written
        \f[
        F_{Y_{i}}(y) = Prob \,(Y_i < y) =
        \int_{-\infty}^\infty\,\cdots\,\int_{-\infty}^{\infty} 
            D_{M_{k}}(m_k)\,dm_k\:
        \int_{-\infty}^{g(y,\vec{a},\vec{m})} D_Z(z)\,dz, \qquad
        g(y,\vec{a},\vec{m}) = \frac{y - \sum_k a_{i,k}m_k}
            {\sqrt{1-\sum_k a_{i,k}^2}}, \qquad \sum_k a_{i,k}^2 < 1
        \f]
        In general, \f$ F_{Y_{i}}(y) \f$ needs to be computed numerically.\par
        The policy class template separates the copula function (the 
        distributions involved) and the functionality (i.e. what the latent  
        model represents: a default probability, a recovery...). Since the  
        copula methods for the 
        probabilities are to be called repeatedly from an integration or a MC 
        simulation, virtual tables are avoided and template parameter mechnics 
        is preferred.\par
        There is nothing at this level enforncing the requirement 
        on the factor distributions to be of zero mean and unit variance. Thats 
        the user responsibility and the model fails to behave correctly if it 
        is not the case.\par
        Derived classes should implement a modelled magnitude (default time, 
        etc) and will provide probability distributions and conditional values.
        They could also provide functionality for the parameter inversion 
        problem, the (e.g.) time at which the modeled variable first takes a 
        given value. This problem has solution/sense depending on the transfer 
        function \f$F_i(Y_i)\f$ characteristics.

        To make direct integration and simulation time efficient virtual 
        functions have been avoided in accessing methods in the copula policy 
        and in the sampling of the random factors
    */
    template <class copulaPolicyImpl>
    class LatentModel 
        : public virtual Observer , public virtual Observable 
    {//observer if factors as quotes
    public:
      void update() override;
      //! \name Copula interface.
      //@{
      typedef copulaPolicyImpl copulaType;
      /*! Cumulative probability of the \f$ Y_i \f$ modelled latent random
          variable to take a given value.
      */
      Probability cumulativeY(Real val, Size iVariable) const {
          return copula_.cumulativeY(val, iVariable);
        }
        //! Cumulative distribution of Z, the idiosyncratic/error factors.
        Probability cumulativeZ(Real z) const {
            return copula_.cumulativeZ(z);
        }
        //! Density function of M, the market/systemic factors.
        Probability density(const std::vector<Real>& m) const {
            #if defined(QL_EXTRA_SAFETY_CHECKS)
                QL_REQUIRE(m.size() == nFactors_, 
                    "Factor size must match that of model.");
            #endif
            return copula_.density(m);
        }
        //! Inverse cumulative distribution of the systemic factor iFactor.
        Real inverseCumulativeDensity(Probability p, Size iFactor) const {
            return copula_.inverseCumulativeDensity(p, iFactor);
        }
        /*! Inverse cumulative value of the i-th random latent variable with a 
         given probability. */
        Real inverseCumulativeY(Probability p, Size iVariable) const {
            return copula_.inverseCumulativeY(p, iVariable);
        }
        /*! Inverse cumulative value of the idiosyncratic variable with a given 
        probability. */
        Real inverseCumulativeZ(Probability p) const {
            return copula_.inverseCumulativeZ(p);
        }
        /*! All factor cumulative inversion. Used in integrations and sampling.
            Inverts all the cumulative random factors probabilities in the 
            model. These are all the systemic factors plus all the idiosyncratic
            ones, so the size of the inversion is the number of systemic factors
            plus the number of latent modelled variables*/
        std::vector<Real> allFactorCumulInverter(const std::vector<Real>& probs) const {
            return copula_.allFactorCumulInverter(probs);
        }
        //@}

        /*! The value of the latent variable Y_i conditional to
            (given) a set of values of the factors.

            The passed allFactors vector contains values for all the
            independent factors in the model (systemic and
            idiosyncratic, in that order). A full sample is required,
            i.e. all the idiosyncratic values are expected to be
            present even if only the relevant one is used.
        */
        Real latentVarValue(const std::vector<Real>& allFactors, 
                            Size iVar) const 
        {
            return std::inner_product(factorWeights_[iVar].begin(), 
                // systemic term:
                factorWeights_[iVar].end(), allFactors.begin(),
                // idiosyncratic term:
                Real(allFactors[numFactors()+iVar] * idiosyncFctrs_[iVar]));
        }
        // \to do write variants of the above, although is the most common case

        const copulaType& copula() const {
            return copula_;
        }


    //  protected:
        //! \name Latent model random factor number generator facility.
        //@{
        /*!  Allows generation or random samples of the latent variable. 

            Generates samples of all the factors in the latent model according 
            to the given copula as random sequence. The default implementation 
            given uses the inversion in the copula policy (which must be 
            present).
            USNG is expected to be a uniform sequence generator in the default 
            implementation. 
        */
        /*
            Several (very different) usages make the spez non trivial
            The final goal is to obtain a sequence generator of the factor 
            samples, several routes are possible depending on the algorithms:
            
            1.- URNG -> Sequence Gen -> CopulaInversion  
              e.g.: CopulaInversion(RandomSequenceGenerator<MersenneTwisterRNG>)
            2.- PseudoRSG ------------> CopulaInversion
              e.g.: CopulaInversion(SobolRSG)
            3.- URNG -> SpecificMapping -> Sequence Gen  (bypasses the copula 
                for performance)
              e.g.: RandomSequenceGenerator<BoxMullerGaussianRng<
                MersenneTwisterRNG> > 
            
            Notice that the order the three algorithms involved (uniform gen, 
            sequence construction, distribution mapping) is not always the same.
            (in fact there could be some other ways to generate but these are 
            the ones in the library now.)
            Difficulties arise when wanting to use situation 3.- whith a generic
            RNG, leaving it unspecified
            
            Derived classes might specialize (on the copula
            type) to another type of generator if a more efficient algorithm 
            that the distribution inversion is available; rewritig then the 
            nextSequence method for a particular copula implementation.
            Some combinations of generators might make no sense, while it 
            could be possible to block template classes corresponding to those
            cases its not done (yet?) (e.g. a BoxMuller under a TCopula.)
            Dimensionality coherence (between the generator and the copula) 
            should have been checked by the client code.
            In multithread usage the sequence generator is expect to be already
            in position.
            To sample the latent variable itself users should call 
            LatentModel::latentVarValue with these samples.
        */
        // Cant use InverseCumulativeRsg since the inverse there has to return a
        //   real number and here a vector is needed, the function inverted here
        //   is multivalued.
        template <class USNG, 
            // dummy template parameter to allow for 'full' specialization of 
            // inner class without specialization of the outer.
            bool = true>
        class FactorSampler {
        public:
            typedef Sample<std::vector<Real> > sample_type;
            explicit FactorSampler(const copulaType& copula, 
                BigNatural seed = 0) 
            : sequenceGen_(copula.numFactors(), seed), // base case construction
              x_(std::vector<Real>(copula.numFactors()), 1.0),
              copula_(copula) { }
            /*! Returns a sample of the factor set \f$ M_k\,Z_i\f$. 
            This method has the vocation of being specialized at particular 
            types of the copula with a more efficient inversion to generate the 
            random variables modelled (e.g. Box-Muller for a gaussian).
            Here a default implementation is provided based directly on the 
            inversion of the cumulative distribution from the copula.
            Care has to be taken in potential specializations that the generator
            algorithm is compatible with an eventual concurrence of the 
            simulations.
             */
            const sample_type& nextSequence() const {
                typename USNG::sample_type sample =
                    sequenceGen_.nextSequence();
                x_.value = copula_.allFactorCumulInverter(sample.value);
                return x_;
            }
        private:
            USNG sequenceGen_;// copy, we might be mutithreaded
            mutable sample_type x_;
            // no copies
            const copulaType& copula_;
        };
        //@}
    protected:
        /* \todo Move integrator traits like number of quadrature points, 
        integration domain dimensions, etc to the copula through a static 
        member function. Since they depend on the nature of the probability 
        density distribution thats where they belong.
        This is why theres one factory per copula policy template parameter 
        (even if this is not used...yet)
        */
        class IntegrationFactory {
        public:
            static ext::shared_ptr<LMIntegration> createLMIntegration(
                Size dimension, 
                LatentModelIntegrationType::LatentModelIntegrationType type = 
                    #ifndef QL_PATCH_SOLARIS
                    LatentModelIntegrationType::GaussianQuadrature)
                    #else
                    LatentModelIntegrationType::Trapezoid)
                    #endif
            {
                switch(type) {
                    #ifndef QL_PATCH_SOLARIS
                    case LatentModelIntegrationType::GaussianQuadrature:
                        return 
                            ext::make_shared<
                            IntegrationBase<GaussianQuadMultidimIntegrator> >(
                                dimension, 25);
                    #endif
                    case LatentModelIntegrationType::Trapezoid:
                        {
                        std::vector<ext::shared_ptr<Integrator> > integrals;
                        for(Size i=0; i<dimension; i++)
                            integrals.push_back(
                            ext::make_shared<TrapezoidIntegral<Default> >(
                                1.e-4, 20));
                        /* This integration domain is tailored for the T 
                        distribution; it is too wide for normals or Ts of high
                        order. 
                        \todo This needs to be solved by having the copula to 
                        provide the integration traits for any integration 
                        algorithm since it is the copula that knows the relevant
                        domain for its density distributions. Also to be able to
                        block integrations which will fail; like a quadrature  
                        here in some cases.
                        */
                        return 
                          ext::make_shared<IntegrationBase<MultidimIntegral> >
                               (integrals, -35., 35.);
                        }
                    default:
                        QL_FAIL("Unknown latent model integration type.");
                }
            }
        private:
          IntegrationFactory() = default;
        };
        //@}


    public:
        // model size, number of latent variables modelled
        Size size() const {return nVariables_;}
        //! Number of systemic factors.
        Size numFactors() const {return nFactors_;}
        //! Number of total free random factors; systemic and idiosyncratic.
        Size numTotalFactors() const { return nVariables_ + nFactors_; }

        /*! Constructs a LM with an arbitrary number of latent variables
          and factors given by the dimensions of the passed matrix.
            @param factorsWeights Ordering is factorWeights_[iVar][iFactor]
            @param ini Initialization variables. Trait type from the copula 
              policy to allow for static policies (this solution needs to be 
              revised, possibly drop the static policy and create a policy 
              member in LatentModel)
        */
        explicit LatentModel(
            const std::vector<std::vector<Real> >& factorsWeights, 
            const typename copulaType::initTraits& ini = 
                typename copulaType::initTraits());
        /*! Constructs a LM with an arbitrary number of latent variables 
          depending only on one random factor but contributing to each latent
          variable through different weights.
            @param factorsWeight Ordering is factorWeights_[iVariable]
            @param ini Initialization variables. Trait type from the copula 
              policy to allow for static policies (this solution needs to be 
              revised, possibly drop the static policy and create a policy 
              member in LatentModel)
        */
        explicit LatentModel(const std::vector<Real>& factorsWeight,
            const typename copulaType::initTraits& ini = 
                typename copulaType::initTraits());
        /*! Constructs a LM with an arbitrary number of latent variables 
          depending only on one random factor with the same weight for all
          latent variables.

            correlSqr is the weight, same for all.

            ini is a trait type from the copula policy, to allow for
            static policies (this solution needs to be revised,
            possibly drop the static policy and create a policy member
            in LatentModel)
        */
        explicit LatentModel(Real correlSqr,
                             Size nVariables,
                             const typename copulaType::initTraits& ini = typename copulaType::initTraits());
        /*! Constructs a LM with an arbitrary number of latent variables 
          depending only on one random factor with the same weight for all
          latent variables. The weight is observed and this constructor is
          intended to be used when the model relates to a market value.

            singleFactorCorrel is the weight/mkt-factor, same for all.

            ini is a trait type from the copula policy, to allow for
            static policies (this solution needs to be revised,
            possibly drop the static policy and create a policy member
            in LatentModel)
        */
        explicit LatentModel(const Handle<Quote>& singleFactorCorrel,
            Size nVariables,
            const typename copulaType::initTraits& ini = 
                typename copulaType::initTraits());

        //! Provides values of the factors \f$ a_{i,k} \f$ 
        const std::vector<std::vector<Real> >& factorWeights() const {
            return factorWeights_;
        }
        //! Provides values of the normalized idiosyncratic factors \f$ Z_i \f$
        const std::vector<Real>& idiosyncFctrs() const {return idiosyncFctrs_;}

        //! Latent variable correlations:
        Real latentVariableCorrel(Size iVar1, Size iVar2) const {
            // true for any normalized combination
            Real init = (iVar1 == iVar2 ? 
                idiosyncFctrs_[iVar1] * idiosyncFctrs_[iVar1] : Real(0.));
            return std::inner_product(factorWeights_[iVar1].begin(), 
                factorWeights_[iVar1].end(), factorWeights_[iVar2].begin(), 
                    init);
        }
        //! \name Integration facility interface
        //@{
        /*! Integrates an arbitrary scalar function over the density domain(i.e.
         computes its expected value).
        */
        Real integratedExpectedValue(
            const std::function<Real(const std::vector<Real>& v1)>& f) const {
            // function composition: composes the integrand with the density 
            //   through a product.
            return integration()->integrate(
                [&](const std::vector<Real>& x){ return copula_.density(x) * f(x); });
        }
        /*! Integrates an arbitrary vector function over the density domain(i.e.
         computes its expected value).
        */
        std::vector<Real> integratedExpectedValueV(
            // const std::function<std::vector<Real>(
            const std::function<std::vector<Real>(
                const std::vector<Real>& v1)>& f ) const {
            detail::multiplyV M;
            return integration()->integrateV(//see note in LMIntegrators base class
                [&](const std::vector<Real>& x){ return M(copula_.density(x), f(x)); });
        }
    protected:
        // Integrable models must provide their integrator.
        // Arguable, not having the integration in the LM class saves that 
        //   memory but have an entry in the VT... 
        virtual const ext::shared_ptr<LMIntegration>& integration() const {
            QL_FAIL("Integration non implemented in Latent model.");
        }
        //@}

        // Ordering is: factorWeights_[iVariable][iFactor]
        mutable std::vector<std::vector<Real> > factorWeights_;
        /* This is a duplicated value from the data above chosen for memory 
        reasons.
        I have opted for this one value redundant memory rather than have the 
        memory load of the observable in all factors. Typically Latent models 
        are used in two very different ways: with many factors and not linked 
        to a market observable (typical matrix size above is of tens of 
        thousands entries) or with just one observable value and the matrix is 
        just a scalar. Otherwise, to remove the redundancy, the matrix 
        factorWeights_ should be one of Quotes Handles.
        Yet it is not entirely true that quotes might be used only in pricing, 
        think sensitivity analysis....
        \todo Reconsider this, see how expensive truly is.
        */
        mutable Handle<Quote> cachedMktFactor_;

        // updated only by correlation observability and constructors.
        // \sqrt{1-\sum_k \beta_{i,k}^2} the addition being along the factors. 
        // It has therefore the size of the basket. Cached for perfomance
        mutable std::vector<Real> idiosyncFctrs_;
        //! Number of systemic factors.
        mutable Size nFactors_;//matches idiosyncFctrs_[0].size();i=0 or any
        //! Number of latent model variables, idiosyncratic terms or model dim
        mutable Size nVariables_;// matches idiosyncFctrs_.size() 

        mutable copulaType copula_;
    };




    // Defines ----------------------------------------------------------------

#ifndef __DOXYGEN__

    template <class Impl>
    LatentModel<Impl>::LatentModel(
        const std::vector<std::vector<Real> >& factorWeights,
        const typename Impl::initTraits& ini)
    : factorWeights_(factorWeights),
      nFactors_(factorWeights[0].size()), 
      nVariables_(factorWeights.size()), copula_(factorWeights, ini)
    {
        for(Size i=0; i<factorWeights.size(); i++) {
            idiosyncFctrs_.push_back(std::sqrt(1.-
                    std::inner_product(factorWeights[i].begin(), 
                factorWeights[i].end(), 
                factorWeights[i].begin(), Real(0.))));
            // while at it, check sizes are coherent:
            QL_REQUIRE(factorWeights[i].size() == nFactors_, 
                "Name " << i << " provides a different number of factors");
        }
    }

    template <class Impl>
    LatentModel<Impl>::LatentModel(
        const std::vector<Real>& factorWeights,
        const typename Impl::initTraits& ini)
    : nFactors_(1),
      nVariables_(factorWeights.size())
    {
        for (Real factorWeight : factorWeights)
            factorWeights_.emplace_back(1, factorWeight);
        for (Real factorWeight : factorWeights)
            idiosyncFctrs_.push_back(std::sqrt(1. - factorWeight * factorWeight));
        //convert row to column vector....
        copula_ = copulaType(factorWeights_, ini);
    }

    template <class Impl>
    LatentModel<Impl>::LatentModel(
        const Real correlSqr,
        Size nVariables,
        const typename Impl::initTraits& ini)
    : factorWeights_(nVariables, std::vector<Real>(1, correlSqr)),
      idiosyncFctrs_(nVariables, 
        std::sqrt(1.-correlSqr*correlSqr)),
      nFactors_(1), 
      nVariables_(nVariables),
      copula_(factorWeights_, ini)
    { }

    template <class Impl>
    LatentModel<Impl>::LatentModel(
        const Handle<Quote>& singleFactorCorrel,
        Size nVariables,
        const typename Impl::initTraits& ini)
    : factorWeights_(nVariables, std::vector<Real>(1, 
        std::sqrt(singleFactorCorrel->value()))),
      cachedMktFactor_(singleFactorCorrel),
      idiosyncFctrs_(nVariables, 
        std::sqrt(1.-singleFactorCorrel->value())),
      nFactors_(1), 
      nVariables_(nVariables),
      copula_(factorWeights_, ini)
    {
        registerWith(cachedMktFactor_);
    }

#endif

    template <class Impl>
    void LatentModel<Impl>::update() {
        /* only registration with the single market correl quote. If we get 
        register with something else remember that the quote stores correlation
        and the model need factor values; which for one factor models are the
        square root of the correlation.
        */
        factorWeights_ = std::vector<std::vector<Real> >(nVariables_, 
            std::vector<Real>(1, std::sqrt(cachedMktFactor_->value())));
        idiosyncFctrs_ = std::vector<Real>(nVariables_, 
            std::sqrt(1.-cachedMktFactor_->value()));
        copula_ = copulaType(factorWeights_, copula_.getInitTraits());
        notifyObservers();
    }

#ifndef __DOXYGEN__

    //----Template partial specializations of the random FactorSampler--------
    /*
    Notice that while the default template needs a sequence generator the 
    specializations need a number generator. This is forced at the time the 
    concrete policy class is used in the template parameter, if it has been 
    specialized it needs the sample type typedef to match at compilation. 
    
    Notice here the outer class template is specialized only, leaving the inner
    generator still a class template. Apparently old versions of gcc (3.x) bug 
    on this one not recognizing the specialization.
    */
    /*! \brief  Specialization for direct Gaussian Box-Muller generation.\par
    The implementation of Box-Muller in the library is the rejection variant so
    do not use it within a multithreaded simulation.
    */
    template<class TC> template<class URNG, bool dummy>
    class LatentModel<TC>
        ::FactorSampler <RandomSequenceGenerator<BoxMullerGaussianRng<URNG> > ,
            dummy> {
        typedef URNG urng_type;
    public:
        //Size below must be == to the numb of factors idiosy + systemi
        typedef Sample<std::vector<Real> > sample_type;
        explicit FactorSampler(const GaussianCopulaPolicy& copula,
                               BigNatural seed = 0) 
        : boxMullRng_(copula.numFactors(), 
            BoxMullerGaussianRng<urng_type>(urng_type(seed))){ }
        const sample_type& nextSequence() const {
                return boxMullRng_.nextSequence();
        }
    private:
        RandomSequenceGenerator<BoxMullerGaussianRng<urng_type> > boxMullRng_;
    };

    /*! \brief Specialization for direct T samples generation.\par
    The PolarT is a rejection algorithm so do not use it within a 
    multithreaded simulation.
    The RandomSequenceGenerator class does not admit heterogeneous 
    distribution samples so theres a trick here since the template parameter is 
    not what it is used internally.
    */
    template<class TC> template<class URNG, bool dummy>//uniform number expected
    class LatentModel<TC>
        ::FactorSampler<RandomSequenceGenerator<PolarStudentTRng<URNG> > , 
            dummy> {
        typedef URNG urng_type;
    public:
        typedef Sample<std::vector<Real> > sample_type;
        explicit FactorSampler(const TCopulaPolicy& copula, BigNatural seed = 0)
        : sequence_(std::vector<Real> (copula.numFactors()), 1.0),
          urng_(seed) {
            // 1 == urng.dimension() is enforced by the sample type
            const std::vector<Real>& varF = copula.varianceFactors();
            for (Real i : varF) // ...use back inserter lambda
                trng_.push_back(PolarStudentTRng<urng_type>(2. / (1. - i * i), urng_));
        }
        const sample_type& nextSequence() const {
            Size i=0;
            for(; i<trng_.size(); i++)//systemic samples plus one idiosyncratic
                sequence_.value[i] = trng_[i].next().value;
            for(; i<sequence_.value.size(); i++)//rest of idiosyncratic samples
                sequence_.value[i] = trng_.back().next().value;
            return sequence_;
        }
    private:
        mutable sample_type sequence_;
        urng_type urng_;
        mutable std::vector<PolarStudentTRng<urng_type> > trng_;
    };

#endif

}                    


#endif
