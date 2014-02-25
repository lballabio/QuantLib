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

#include <vector>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/construct.hpp>

#include <ql/experimental/math/multidimintegrator.hpp>

// for the Gaussian inverter spez:
//////#include <ql/experimental/credit/tinasmerstwisterurng.hpp>
//////#include <ql/math/randomnumbers/boxmullergaussianrng.hpp>
#include <ql/math/randomnumbers/randomsequencegenerator.hpp>

/* removed, see note in typedef.
#include <ql/experimental/math/gaussiancopulapolicy.hpp> // for typedef
#include <ql/experimental/math/tcopulapolicy.hpp> // for typedef
*/

/*! \file latentmodel.hpp
    \brief Generic multifactor latent variable model.
*/
namespace QuantLib {

    namespace {
        // havent figured out how to do this in-place
        struct multiplyV {
            typedef std::vector<QuantLib::Real>& result_type;
            std::vector<Real>& operator()(Real d,  std::vector<Real>& v) {
                std::transform(v.begin(), v.end(), v.begin(), 
                    boost::lambda::_1 * d);//1., 
            return v;
            }
        };
    }


        

    /*.........
        -A restriction of this implementation is that all the idiosyncratic variables/drivers have the same distribution.

        Part of the interface should be implemented statically. The reason
        for this is that these methods are heavily called within Monte Carlo
        simulations and dynamic polymorphism is too expensive.

        Derived classes must implement the following interface (preferentially
        inlined). These define the copula and can be analitically specified, tabulated or calibrated.
        \code
        public:
          Probability cumulativeY(Real val) const;
          Real inverseCumulativeZ(Probability p) const;
          Real inverseCumulativeDensity(Probability p) const;
        \endcode
        and may implement the following:
        \code
        public:


        \endcode

        \ingroup lattices
    */

        /*  Need the policy pattern to separate the copula function (the distributions) and the functionality (i.e. what the latent model represents: a default probability, a recovery, a bond value....) Otherwise we end up with a diamond.

*/













    // allow pointers to generic LatentModels:.....disallow to users.
    class LatentModelInterface {
    public:
        virtual ~LatentModelInterface(){}
    };

    // 9.- Adapt the coments in Rolands code to the methods here.
    // Rolands documentation included
    // Intended to replace OneFactorCopula
    // EXPLAIN THE TEMPLATE PARAMETER EXPECTED INTERFACE.....
    /*!
    \brief Generic multifactor latent variable model.\par
        In this model set up one considers latent (random) variables \f$ Y_i \f$ described by:
        \f[
        \begin{array}{ccccc}
        Y_1 & = & \sum_k M_k a_{1,k} & + \sqrt{1-\sum_k a_{1,k}^2} Z_1 & \sim \Phi_{Y_1}\nonumber \\
        ... & = &      ... & ...   & \nonumber \\
        Y_i & = & \sum_k M_k a_{i,k} & + \sqrt{1-\sum_k a_{i,k}^2} Z_i & \sim \Phi_{Y_i}\nonumber \\
        ... & = &      ... & ...   & \nonumber \\
        Y_N & = & \sum_k M_k a_{N,k} & + \sqrt{1-\sum_k a_{N,k}^2} Z_N & \sim \Phi_{Y_N}
        \end{array}
        \f]
        where the systemic \f$ M_k \f$ and idiosyncratic \f$ Z_i \f$ (this last one known as error term in some contexts) random variables have independent zero-mean unit-variance distributions. A restriction of the model implemented here is that the N idiosyncratic variables all follow the same probability law \f$ \Phi_Z(z)\f$. Also the model is normalized so that: \f$-1\leq a_{i,k} \leq 1\f$ (technically the \f$Y_i\f$ are convex linear combinations). The correlation between \f$Y_i\f$ and \f$Y_j\f$ is then \f$\sum_k a_{i,k} a_{j,k}\f$. 
        \f$\Phi_{Y_i}\f$ denotes the cumulative distribution function of \f$Y_i\f$ which in general differs for each latent variable.\par
        The latent variables are typically treated as unobservable magnitudes and they serve to model one or several magnitudes related to them through some function
        \f[
        \begin{array}{ccc}
        F_i(Y_i) & = & F_i(\sum_k M_k a_{i,k} + \sqrt{1-\sum_k a_{i,k}^2} Z_i ) \nonumber \\
        & = & F_i(M_1,..., M_k, ..., M_K, Z_i)
        \end{array}
        \f]
        The transfer function can have a more generic form: \f$F_i(Y_1,....,Y_N)\f$ but here the model is restricted to a one to one relation between the latent variables and the modelled ones. Also it is assumed that \f$F_i(y_i; \tau)\f$ is monotonic in \f$y_i\f$; it can then be inverted and the relation of the cumulative probability of \f$F_i\f$ and \f$Y_i\f$ is simple:
        \f[
        \int_{\infty}^b \phi_{F_i} df = \int_{\infty}^{F_i^{-1}(b)} \phi_{Y_i} dy
        \f]
If  \f$t\f$ is some value of the functional or modelled variable, \f$y\f$ is mapped to \f$t\f$ such that percentiles match, i.e. \f$F_Y(y)=Q_i(t)\f$ or \f$y=F_Y^{-1}(Q_i(t))\f$.
The class provides an integration facility of arbitrary functions dependent on the model states. It also provides random number generation interfaces for usage of the model in monte carlo simulations.

        Now let \f$\Phi_Z(z)\f$ be the cumulated distribution function of (all equal as mentioned)
        \f$Z_i\f$. For given realization of \f$M_k\f$, this determines
        the distribution of \f$y\f$:
        \f[
        Prob \,(Y_i < y|M_k) = \Phi_Z \left( \frac{y-\sum_k a_{i,k}\,M_k}{\sqrt{1-\sum_k a_{i,k}^2}}\right)
        \qquad
        \mbox{or}
        \qquad
        Prob \,(t_i < t|M) = F_Z \left( \frac{F_Y^{-1}(Q_i(t))-a_i\,M}
        {\sqrt{1-a_i^2}}
        \right)
        \f]

        The distribution functions of \f$ M, Z_i \f$ are specified in
        derived classes. The distribution function of \f$ Y \f$ is
        then given by the convolution
        \f[
        F_Y(y) = Prob\,(Y<y) = \int_{-\infty}^\infty\,\int_{-\infty}^{\infty}\:
        D_Z(z)\,D_M(m) \quad
        \Theta \left(y - a\,m - \sqrt{1-a^2}\,z\right)\,dm\,dz,
        \qquad
        \Theta (x) = \left\{
        \begin{array}{ll}
        1 & x \geq 0 \\
        0 & x < 0
        \end{array}\right.
        \f]
        where \f$ D_Z(z) \f$ and \f$ D_M(m) \f$ are the probability
        densities of \f$ Z\f$ and \f$ M, \f$ respectively.

        This convolution can also be written
        \f[
        F(y) = Prob \,(Y < y) =
        \int_{-\infty}^\infty D_M(m)\,dm\:
        \int_{-\infty}^{g(y,a,m)} D_Z(z)\,dz, \qquad
        g(y,a,m) = \frac{y - a\cdot m}{\sqrt{1-a^2}}, \qquad a < 1
        \f]

        or

        \f[
        F(y) = Prob \,(Y < y) =
        \int_{-\infty}^\infty D_Z(z)\,dz\:
        \int_{-\infty}^{h(y,a,z)} D_M(m)\,dm, \qquad
        h(y,a,z) = \frac{y - \sqrt{1 - a^2}\cdot z}{a}, \qquad a > 0.
        \f]

        In general, \f$ F_Y(y) \f$ needs to be computed numerically.



        // document policy template parameter , interface etc.................
    */
        /* Subproblems or problem components:
          At this base class level:
         -> The value of the latent variable Y_i conditional to (given) a set of values of the factors. This is independent of the copula since the values are given
         -> The probability of a given value of the latent variable Y_i. This depends on the copula (convolution)Problem is transferred to the copula.
         -> Probability of the independent systemic factors: Problem is transferred to the copula.
         -> Probability of the independent idiosyncratic: Problem is transferred to the copula.

          In derived classes (i.e. modeling a concrete magnitude from the Latent variable):
         -> The value of the modeled variable given a value of the latent variable Y_i (or the whole set of Y_i)
         -> and indirectly from the above: The value of the modeled variable given a set of values of the factors.
         Using one or the above depends on wehter we want a joint or conditional integration.....
         -> Conidtional prob of a value of the modelled variable given a set value of the systemic factors. This leaves the systemic outside of the problem and creates a distribution.
 
         -> parameter inversion problem, the time at which the modeled variable first takes a given value. This problem has solution depending on the transfer function F_i(Y_i) characteristics.
There are no checks that the variables modelled through the copula parameter have the required properties, namely zero mean and unit variance.
        */
    template <class copulaPolicyImpl>
    class LatentModel : public LatentModelInterface {// private copulaPolicyImpl  <- non-static policies
    public:
        //! \name Copula interface.
        //@{
        typedef typename copulaPolicyImpl copulaType;
        /* 
         Allows calling of the variables probabilities directly. It enforces 
        the interface into the template argument this can be an issue since not 
        all aplications might need all of them, if so implement an empty method 
        with a QL_FAIL. 
        */
        /*  These to be activated if finally I opt for non-static policies and this class derives from them.
        using copulaPolicyImpl::cumulativeY;
        using copulaPolicyImpl::cumulativeZ;
        using copulaPolicyImpl::density;
        using copulaPolicyImpl::inverseCumulativeDensity;
        using copulaPolicyImpl::inverseCumulativeY;
        using copulaPolicyImpl::inverseCumulativeZ;
        */
        /*! Cumulative probability of the \f$ Y_i \f$ modelled random variable 
            to take a given value.
        */
        Probability cumulativeY(Real val, Size iVariable) const {
          /////////////////////  return copulaPolicyImpl::cumulativeY(val, iVariable);
            return copula_.cumulativeY(val, iVariable);
        }
        //! Cumulative distribution of Z, the idiosyncratic/error factors.
        Probability cumulativeZ(Real z) const {
          /////////////////////  return copulaPolicyImpl::cumulativeZ(z);
            return copula_.cumulativeZ(z);
        }
        //! Density function of M, the market/systemic factors.
        Probability density(const std::vector<Real>& m) const {
            #if defined(QL_EXTRA_SAFETY_CHECKS)
                QL_REQUIRE(m.size() == nFactors_, 
                    "Factor size must match that of model.");
            #endif
          /////////////////////  return copulaPolicyImpl::density(m);
            return copula_.density(m);
        }
        //! Inverse of the cumulative distribution of the idiosyncratic factor iFactor. Theres no index since it follows the same probability law for all variables.
        Real inverseCumulativeDensity(Probability p) const {//<<<<<<<<REDUNDANT, DUPLICATED...IT IS == inverseCumulativeZ
          /////////////////////  return copulaPolicyImpl::inverseCumulativeDensity(p);
            return copula_.inverseCumulativeDensity(p);
        }
        /*! Inverse cumulative value of the i-th random variable with a given 
        probability. */
        Real inverseCumulativeY(Probability p, Size iVariable) const {
          /////////////////////  return copulaPolicyImpl::inverseCumulativeY(p);
            return copula_.inverseCumulativeY(p, iVariable);
        }
        /*! Inverse cumulative value of the idiosyncratic variable with a given 
        probability. */
        Real inverseCumulativeZ(Probability p) const {
          /////////////////////  return copulaPolicyImpl::inverseCumulativeZ(p);
            return copula_.inverseCumulativeZ(p);
        }
        /// --- ADD NOW allFactorCumulInverter(const std::vector<Real>& uniformSeqSampleProbs) or use it differently in the random generator..
        //@}


        friend class InverseLatentVariableRsg;
        //! \name Random number generator facility.
        //@{
        /*!  Allows generation or random samples of the latent variable. 

            RSG will be a uniform sequence generator in the default 
            implementation. Some derived classes might specialize (on the copula
            type) to another type of generator if a more efficient algorithm 
            that the distribution inversion is available rewritig then the 
            nextSequence method for a particular copula implementation.
        */
        template <class RSG>         
        class LatentVariableModelRsg {// Was: class InverseLatentVariableRsg {
            // derive RandomSequenceGenerator? <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
        public:
            typedef Sample<std::vector<Real> > sample_type;
            LatentVariableModelRsg(const RSG& rsg)
            : sequenceGenerator_(rsg),
              // dimensions should be coherent (check?)
              x_(std::vector<Real>(rsg.dimension()), 1.0) { }

            /*! Returns a sample of the modelled variables \f$ Y_i \f$. 
            This method has the vocation of being specialized at particular 
            types of the copula with a more efficient inversion to generate the 
            random variables modelled (e.g. Box-Muller for a gaussian).
            Here a default implementation is provided based directly on the 
            inversion of the cumulative distribution from the copula.
             */
            const sample_type& nextSequence() const {
                typename RSG::sample_type sample =
                    sequenceGenerator_.nextSequence();
                // sample.value.size() is equal to the num of factors in the
                // model plus one, this extra one corresponds to the systemic 
                // variable.

                /* Delegate the inversion to the copula policy.
                  Notice theres no ptr to the LM here and thus we are ignorant 
                  of the problem dimensions (how many variables and number of 
                  isiosync factors), we only know the total. And one needs to 
                  split since some factors need to be inverted through the 
                  systemic and some through the idiosyncratic inverse 
                  cumulatives. This is trivial in the Gaussian case where even 
                  the dimensions can be ignored. In the general case this means 
                  the copula has to be aware of the problems dimensions; this 
                  is one argument against the decission to make copula policies 
                  static as it is now.
                */
                // use inversecumulativersg.hpp? only is one dimensional there
                x_.value = 
                    ////////////////   copulaPolicyImpl::allFactorCumulInverter(sample.value);
                    copula_.allFactorCumulInverter(sample.value);////<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
                // weight stays 1.
                return x_;
            }
            const sample_type& lastSequence() const { return x_; }
            Size dimension() const { return usg.dimension(); }
        private:
            RSG sequenceGenerator_;
            mutable sample_type x_;
        };
        //@}

        // model size, number of latent variables modelled
        Size size() const {return nVariables_;}
        // 
        Size numFactors() const {return nFactors_;}

        /*! Constructs a LM with an arbitrary number of latent variables
          and factors given by the dimensions of the passed matrix.
            @param factorsWeights Ordering is factorWeights_[iName][iFactor]
            @param quadOrder Quadrature load.
            @param ini Initialization variables. Trait type from the copula 
              policy to allow for static policies (this solution needs to be 
              revised, possibly drop the static policy and create a policy 
              member in LatentModel)
        */
        LatentModel(const std::vector<std::vector<Real> >& factorsWeights, 
            Size quadOrder,
            const typename copulaType::initTraits& ini = 
                copulaType::initTraits());
        /*! Constructs a LM with an arbitrary number of latent variables 
          depending only on one random factor but contributing to each latent
          variable through different weights.
            @param factorsWeights Ordering is factorWeights_[iVariable]
            @param quadOrder Quadrature load.
            @param ini Initialization variables. Trait type from the copula 
              policy to allow for static policies (this solution needs to be 
              revised, possibly drop the static policy and create a policy 
              member in LatentModel)
        */
        LatentModel(const std::vector<Real>& factorsWeight, Size quadOrder,
            const typename copulaType::initTraits& ini = 
                copulaType::initTraits());
        /*! Constructs a LM with an arbitrary number of latent variables 
          depending only on one random factor with the same weight for all
          latent variables.
            @param factorsWeight The weight, same for all.
            @param quadOrder Quadrature load.
            @param ini Initialization variables. Trait type from the copula 
              policy to allow for static policies (this solution needs to be 
              revised, possibly drop the static policy and create a policy 
              member in LatentModel)
        */
        LatentModel(const Real beta, Size nVariables, Size quadOrder,
            const typename copulaType::initTraits& ini = 
                copulaType::initTraits());

        const std::vector<std::vector<Real> >& factorWeights() const {
            return factorWeights_;
        }
        const std::vector<Real>& idiosyncFctrs() const {return idiosyncFctrs_;}


        
        

        //! \name Integration facility interface
        //@{
        /*! Integrates an arbitrary scalar function over the density domain(i.e.
         computes its expected value).
        */
        Real integrate(
            const boost::function<Real(const std::vector<Real>& v1)>& f) const {
            // function composition: composes the integrand with the density 
            //   through a product.

            return 
                integrator_.integrate<Real>(boost::bind(std::multiplies<Real>(), 
                boost::bind(&copulaPolicyImpl::density, copula_, _1),//notice, static<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
                boost::bind(boost::cref(f), _1)));   
                /*--------------------------------------------------------------------------------------------

            return integrator2_(boost::bind(std::multiplies<Real>(),
                    boost::bind(&copulaPolicyImpl::density, copula_, _1),//notice, static<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
                    boost::bind(boost::cref(f), _1)), std::vector<Real>(1, -19.), std::vector<Real>(1, 19.));
            */


        }
        /*! Integrates an arbitrary vector function over the density domain(i.e.
         computes its expected value).
        */
        Disposable<std::vector<Real> > integrate(
            const boost::function<std::vector<Real>(const std::vector<Real>& v1)>& f ) const {

            return integrator_.integrate<std::vector<Real> >(
                boost::bind(multiplyV(),
                    boost::bind(&copulaPolicyImpl::density, copula_, _1),//notice, static<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
                    boost::bind(boost::cref(f), _1)));

        }
        //@}

    protected:
        // Ordering is: factorWeights_[iVariable][iFactor]
        mutable std::vector<std::vector<Real> > factorWeights_;
        // updated only by correlation observability and constructors.
        // \sqrt{1-\sum_k \beta_{i,k}^2} the addition being along the factors. 
        // It has therefore the size of the basket. Cached for perfomance
        mutable std::vector<Real> idiosyncFctrs_;
        //! Number of systemic factors.
        mutable Size nFactors_;//matches idiosyncFctrs_[0].size();i=0 or any
        //! Number of latent model variables, idiosyncratic terms or model dim
        const Size nVariables_;// matches idiosyncFctrs_.size()

        // \todo Dont tie it to a Quadrature make the integrator algo generic:
        GaussianQuadMultidimIntegrator integrator_;
        MultidimIntegral integrator2_;///<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
        copulaType copula_;///   const ???   ///////////////////////////////////////////////
    };


    // Defines -----------------------------------------------------------------

    template <class Impl>
    LatentModel<Impl>::LatentModel(
        const std::vector<std::vector<Real> >& idiosyncFctrsWeights,
        Size quadOrder,
        const typename Impl::initTraits& ini)
    : factorWeights_(idiosyncFctrsWeights),
      nFactors_(idiosyncFctrsWeights[0].size()), 
      integrator_(idiosyncFctrsWeights[0].size(), quadOrder),
      /* ----------------------------------------------------------- */ integrator2_(std::vector<boost::shared_ptr<Integrator> >(1, boost::make_shared<TrapezoidIntegral<Default> >(1.e-6, 500))),
      nVariables_(idiosyncFctrsWeights.size()), copula_(ini)
    {
        // \todo test idiosyncFctrsWeight areproperly normalized.

        /// REQUIRE SUM OF FACTORS SQUEARED IS EQ TO 1
        // ALSO THE INDIVIDUAL densities in the copula must have expected value of zero and variance of 1
        /* This is important since the model uses the fact that central moments of the orders of the expected value(n=1) and the variace(n=2) satisfy w.r.t 
        1.- the product of a constant: M_n(a X) = a^n M_n(X)
        2.- the addition: M_n(X+Y) = M_n(X) + M_n(Y)
        
        (2.-) guarantees that the expected value of Y is zero if all the expected vals of the individual distrib are.
        (1.-) guarantees that the variance is 1; and that independently of the individual distributions additioned.

        This moment conservation is not guaranteed for higher orders, for instante, in the gaussian case 4th order of the standard gausiians is 1, while Y has a moment-4 of value : \sum_i \beta^4 + (1-\sum_i\beta^2)^2 , no longer 1

        Since the variance of the T-\nu is \frac{\nu}{\nu-2} the individual distributions we shall convolve are the T-normalized : \sqrt{\frac{\nu-2}{\nu}} \Phi_T_{\nu}

        The normalization conditions on the betas allows us to determine the distribution of Y without knowing the betas. Still we might need any other parametrizations; think of the case of the gaussian wich is then determined by this normalization but it is not the case of families of distributions like the T.
        */

        // init the copula policy:
 /////////////////////////////>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>       Impl::init(ini);

        // idiosyncFctrsWeights[iVariable][iFactor]
        for(Size i=0; i<idiosyncFctrsWeights.size(); i++) {
            idiosyncFctrs_.push_back(std::sqrt(1.-
                    std::inner_product(idiosyncFctrsWeights[i].begin(), 
                idiosyncFctrsWeights[i].end(), 
                idiosyncFctrsWeights[i].begin(), 0.)));
            // while at it, check sizes are coherent:
            QL_REQUIRE(idiosyncFctrsWeights[i].size() == nFactors_, 
                "Name " << i << " provides a different number of factors");
        }
    }

    template <class Impl>
    LatentModel<Impl>::LatentModel(
        const std::vector<Real>& idiosyncFctrsWeights,
        Size quadOrder,
        const typename Impl::initTraits& ini)
    : nFactors_(1),
      nVariables_(idiosyncFctrsWeights.size()),
      integrator_(1, quadOrder),
       integrator2_(std::vector<boost::shared_ptr<Integrator> >(1, boost::make_shared<TrapezoidIntegral<Default> >(1.e-6, 500))),
      copula_(ini) 
    {
        // \todo test idiosyncFctrsWeight areproperly normalized.
        // init the copula policy:
//////////////////////////////////>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>        Impl::init(ini);

        // wont recognize the right constructor...?
        //std::transform(idiosyncFctrsWeights.begin(), 
        // idiosyncFctrsWeights.end(), std::back_inserter(factorWeights_), 
        // boost::lambda::bind(boost::lambda::constructor<std::vector<Real> >(),
        //   1, boost::cref(_1)));
        for(Size iName=0; iName < idiosyncFctrsWeights.size(); iName++)
            factorWeights_.push_back(std::vector<Real>(1, 
                idiosyncFctrsWeights[iName]));
        for(Size iName=0; iName < idiosyncFctrsWeights.size(); iName++)
            idiosyncFctrs_.push_back(std::sqrt(1. - 
                idiosyncFctrsWeights[iName]*idiosyncFctrsWeights[iName]));
    }

    template <class Impl>
    LatentModel<Impl>::LatentModel(
        const Real idiosyncFctrsWeight,
        Size nVariables,
        Size quadOrder,
        const typename Impl::initTraits& ini)
    : factorWeights_(nVariables, std::vector<Real>(1, idiosyncFctrsWeight)),
      nFactors_(1), 
      nVariables_(nVariables),
      idiosyncFctrs_(nVariables, std::sqrt(1.-idiosyncFctrsWeight*idiosyncFctrsWeight)),
      integrator_(1, quadOrder),
       integrator2_(std::vector<boost::shared_ptr<Integrator> >(1, boost::make_shared<TrapezoidIntegral<Default> >(1.e-6, 500))),
      copula_(ini) 
    {
        // \todo test idiosyncFctrsWeight areproperly normalized.
        // init the copula policy:
 /////////////////////////////////>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>       Impl::init(ini);
    }
  
/* not really needed, they are 'abstract' they do not model anything concrete yet, aside form the latent variable itself.
    typedef LatentModel<GaussianCopulaPolicy> GaussianLatentModel; 
    typedef LatentModel<TCopulaPolicy> TLatentModel;
*/

}                    


#endif
