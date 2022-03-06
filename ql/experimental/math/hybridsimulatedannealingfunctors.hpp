/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
Copyright (C) 2015 Andres Hernandez

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

/*! \file hybridsimulatedannealingfunctors.hpp
\brief Functors for use on HybridSimulatedAnnealing
*/

#ifndef HYBRIDSIMULATEDANNEALINGFUNCTORS_H
#define HYBRIDSIMULATEDANNEALINGFUNCTORS_H

#include <ql/math/array.hpp>
#include <ql/math/randomnumbers/seedgenerator.hpp>
#include <ql/math/optimization/problem.hpp>

#include <boost/random/mersenne_twister.hpp>
typedef boost::mt19937 base_generator_type;

#include <boost/random/normal_distribution.hpp>
#include <boost/random/lognormal_distribution.hpp>
#include <boost/random/cauchy_distribution.hpp>
#include <boost/random/uniform_real_distribution.hpp>
typedef boost::random::uniform_real_distribution<double> uniform;
typedef boost::random::normal_distribution<> normal_random;
typedef boost::random::lognormal_distribution<> lognormal_random;
typedef boost::random::cauchy_distribution<> cauchy_random;

#include <boost/random/variate_generator.hpp>
typedef boost::variate_generator<base_generator_type&, uniform > uniform_variate;
typedef boost::variate_generator<base_generator_type, normal_random > normal_variate;
typedef boost::variate_generator<base_generator_type&, lognormal_random > lognormal_variate;
typedef boost::variate_generator<base_generator_type&, cauchy_random > cauchy_variate;

#include <algorithm> //for std::max
#include <cmath>     //for log
#include <utility>
#include <vector>

namespace QuantLib
{
    //! Lognormal Sampler
    /*!    Sample from lognormal distribution. This means that the parameter space
    must have support on the positve side of the real line only.
    */
    class SamplerLogNormal
    {
    public:
        explicit SamplerLogNormal(unsigned long seed = SeedGenerator::instance().get()) :
            generator_(seed),
            distribution_(0.0, 1.0), gaussian_(generator_, distribution_) {};
        SamplerLogNormal(const SamplerLogNormal& sampler) : generator_(sampler.gaussian_.engine()),
            distribution_(sampler.gaussian_.distribution()),
            gaussian_(generator_, distribution_) {};
        SamplerLogNormal& operator=(const SamplerLogNormal& sampler) {
            generator_ = sampler.gaussian_.engine();
            distribution_ = sampler.gaussian_.distribution();
            gaussian_ = normal_variate(generator_, distribution_);
            return *this;
        }

        inline void operator()(Array &newPoint, const Array &currentPoint, const Array &temp) const {
            QL_REQUIRE(newPoint.size() == currentPoint.size(), "Incompatible input");
            QL_REQUIRE(newPoint.size() == temp.size(), "Incompatible input");
            for (Size i = 0; i < currentPoint.size(); i++)
                newPoint[i] = currentPoint[i] * exp(sqrt(temp[i])*gaussian_());
        };
    private:
        base_generator_type generator_;
        normal_random distribution_;
        mutable normal_variate gaussian_;
    };

    //! Gaussian Sampler
    /*!    Sample from normal distribution. This means that the parameter space
    must have support on the whole real line.
    */
    class SamplerGaussian
    {
    public:
        explicit SamplerGaussian(unsigned long seed = SeedGenerator::instance().get()) :
            generator_(seed),
            distribution_(0.0, 1.0), gaussian_(generator_, distribution_) {};
        SamplerGaussian(const SamplerGaussian& sampler) : generator_(sampler.gaussian_.engine()),
            distribution_(sampler.gaussian_.distribution()),
            gaussian_(generator_, distribution_) {};
        SamplerGaussian& operator=(const SamplerGaussian& sampler) {
            generator_ = sampler.gaussian_.engine();
            distribution_ = sampler.gaussian_.distribution();
            gaussian_ = normal_variate(generator_, distribution_);
            return *this;
        }

        inline void operator()(Array &newPoint, const Array &currentPoint, const Array &temp) const {
            QL_REQUIRE(newPoint.size() == currentPoint.size(), "Incompatible input");
            QL_REQUIRE(newPoint.size() == temp.size(), "Incompatible input");
            for (Size i = 0; i < currentPoint.size(); i++)
                newPoint[i] = currentPoint[i] + std::sqrt(temp[i])*gaussian_();
        };
    private:
        base_generator_type generator_;
        normal_random distribution_;
        mutable normal_variate gaussian_;
    };
    
    //! Gaussian Ring Sampler
    /*! Sample from normal distribution, but constrained to lie within
     * .boundaries. If the value ends up beyond the boundary, the value
     * is circled back from the other side.
    */
    class SamplerRingGaussian
    {
    public:
      SamplerRingGaussian(Array lower,
                          Array upper,
                          unsigned long seed = SeedGenerator::instance().get())
      : generator_(seed), distribution_(0.0, 1.0), gaussian_(generator_, distribution_),
        lower_(std::move(lower)), upper_(std::move(upper)){};
      SamplerRingGaussian(const SamplerRingGaussian& sampler)
      : generator_(sampler.gaussian_.engine()), distribution_(sampler.gaussian_.distribution()),
        gaussian_(generator_, distribution_), lower_(sampler.lower_), upper_(sampler.upper_){};
      SamplerRingGaussian& operator=(const SamplerRingGaussian& sampler) {
          generator_ = sampler.gaussian_.engine();
          distribution_ = sampler.gaussian_.distribution();
          gaussian_ = normal_variate(generator_, distribution_);
          lower_ = sampler.lower_;
          upper_ = sampler.upper_;
          return *this;
        }

        inline void operator()(Array &newPoint, const Array &currentPoint, const Array &temp) const {
            QL_REQUIRE(newPoint.size() == currentPoint.size(), "Incompatible input");
            QL_REQUIRE(newPoint.size() == temp.size(), "Incompatible input");
            for (Size i = 0; i < currentPoint.size(); i++){
                newPoint[i] = currentPoint[i] + std::sqrt(temp[i])*gaussian_();
                while(newPoint[i] < lower_[i] || newPoint[i] > upper_[i]){
					if(newPoint[i] < lower_[i]){
						newPoint[i] = upper_[i] + newPoint[i] - lower_[i];
					} else {
						newPoint[i] = lower_[i] + newPoint[i] - upper_[i];
					}
				} 
            }
        };
    private:
        base_generator_type generator_;
        normal_random distribution_;
        mutable normal_variate gaussian_;
        Array lower_, upper_;
    };
    
    //! Gaussian Mirror Sampler
    /*! Sample from normal distribution, but constrained to lie within
     * .boundaries. If the value ends up beyond the boundary, the value
     * is reflected back.
    */
    class SamplerMirrorGaussian
    {
    public:
      SamplerMirrorGaussian(Array lower,
                            Array upper,
                            unsigned long seed = SeedGenerator::instance().get())
      : generator_(seed), distribution_(0.0, 1.0), gaussian_(generator_, distribution_),
        lower_(std::move(lower)), upper_(std::move(upper)){};
      SamplerMirrorGaussian(const SamplerMirrorGaussian& sampler)
      : generator_(sampler.gaussian_.engine()), distribution_(sampler.gaussian_.distribution()),
        gaussian_(generator_, distribution_), lower_(sampler.lower_), upper_(sampler.upper_){};
      SamplerMirrorGaussian& operator=(const SamplerMirrorGaussian& sampler) {
          generator_ = sampler.gaussian_.engine();
          distribution_ = sampler.gaussian_.distribution();
          gaussian_ = normal_variate(generator_, distribution_);
          lower_ = sampler.lower_;
          upper_ = sampler.upper_;
          return *this;
        }

        inline void operator()(Array &newPoint, const Array &currentPoint, const Array &temp) const {
            QL_REQUIRE(newPoint.size() == currentPoint.size(), "Incompatible input");
            QL_REQUIRE(newPoint.size() == temp.size(), "Incompatible input");
            for (Size i = 0; i < currentPoint.size(); i++){
                newPoint[i] = currentPoint[i] + std::sqrt(temp[i])*gaussian_();
                while(newPoint[i] < lower_[i] || newPoint[i] > upper_[i]){
					if(newPoint[i] < lower_[i]){
						newPoint[i] = lower_[i] + lower_[i] - newPoint[i];
					} else {
						newPoint[i] = upper_[i] + upper_[i] - newPoint[i];
					}
				}
            }
        };
    private:
        base_generator_type generator_;
        normal_random distribution_;
        mutable normal_variate gaussian_;
        Array lower_, upper_;
    };

    //! Cauchy Sampler
    /*!    Sample from cauchy distribution. This means that the parameter space
    must have support on the positive whole real line. For lower dimensions
    it could be faster than the Gaussian sampler, specially when combined
    with the Cauchy temperature.
    */
    class SamplerCauchy
    {
    public:
        explicit SamplerCauchy(unsigned long seed = SeedGenerator::instance().get()) :
            generator_(seed),
            distribution_(0.0, 1.0), cauchy_(generator_, distribution_) {};
        SamplerCauchy(const SamplerCauchy& sampler) : generator_(sampler.cauchy_.engine()),
            distribution_(sampler.cauchy_.distribution()),
            cauchy_(generator_, distribution_) {};

        inline void operator()(Array &newPoint, const Array &currentPoint, const Array &temp) const {
            QL_REQUIRE(newPoint.size() == currentPoint.size(), "Incompatible input");
            QL_REQUIRE(newPoint.size() == temp.size(), "Incompatible input");
            for (Size i = 0; i < currentPoint.size(); i++)
                newPoint[i] = currentPoint[i] + temp[i] * cauchy_();
        };
    protected:
        base_generator_type generator_;
        cauchy_random distribution_;
        mutable cauchy_variate cauchy_;
    };

    //! Very Fast Annealing Sampler
    /*!    For consistency should be used with TemperatureVeryFastAnnealing.
    Requires that the parameter space be bounded above and below.
    */
    class SamplerVeryFastAnnealing
    {
    public:
      SamplerVeryFastAnnealing(Array lower,
                               Array upper,
                               unsigned long seed = SeedGenerator::instance().get())
      : lower_(std::move(lower)), upper_(std::move(upper)), generator_(seed),
        uniform_(generator_, distribution_) {
          QL_REQUIRE(lower_.size() == upper_.size(), "Incompatible input");
      };
        SamplerVeryFastAnnealing(const SamplerVeryFastAnnealing& sampler) :
            lower_(sampler.lower_), upper_(sampler.upper_),
            generator_(sampler.uniform_.engine()), distribution_(sampler.uniform_.distribution()),
            uniform_(generator_, distribution_) {
            QL_REQUIRE(lower_.size() == upper_.size(), "Incompatible input");
        };

        inline void operator()(Array &newPoint, const Array &currentPoint, const Array &temp) const {
            QL_REQUIRE(newPoint.size() == currentPoint.size(), "Incompatible input");
            QL_REQUIRE(newPoint.size() == lower_.size(), "Incompatible input");
            QL_REQUIRE(newPoint.size() == temp.size(), "Incompatible input");
            for (Size i = 0; i < currentPoint.size(); i++) {
                newPoint[i] = lower_[i] - 1.0;
                while (newPoint[i] < lower_[i] || newPoint[i] > upper_[i]) {
                    Real draw = uniform_();
                    Real sign = static_cast<int>(0.5 < draw) - static_cast<int>(draw < 0.5);
                    Real y = sign*temp[i] * (std::pow(1.0 + 1.0 / temp[i],
                                                      std::abs(2 * draw - 1.0)) - 1.0);
                    newPoint[i] = currentPoint[i] + y*(upper_[i] - lower_[i]);
                }
            }
        };
    private:
        Array lower_, upper_;
        base_generator_type generator_;
        uniform distribution_;
        mutable uniform_variate uniform_;
    };

    //! Always Downhill Probability
    /*!    Only points that improve on the current solution are accepted.
    Depending on the problem, this makes it very unlikely that the
    optimizer will be able to escape a local optimum.
    */
    struct ProbabilityAlwaysDownhill {
        inline bool operator()(Real currentValue, Real newValue, const Array &temp) const {
            return currentValue > newValue; //return true if new value is lower than old value
        }
    };

    //! Boltzmann Probability
    /*!    The probability of accepting a new point is sampled from a Boltzmann distribution.
    A point is accepted if \f$ \frac{1}{1+exp(-(current-new)/T)} > u \f$
    where \f$ u \f$ is drawn from a uniform distribution.
    */
    class ProbabilityBoltzmann {
    public:
        explicit ProbabilityBoltzmann(unsigned long seed = SeedGenerator::instance().get()) :
            generator_(seed),
            uniform_(generator_, distribution_) {};
        ProbabilityBoltzmann(const ProbabilityBoltzmann &probability) :
            generator_(probability.uniform_.engine()), distribution_(probability.uniform_.distribution()),
            uniform_(generator_, distribution_) {};
        inline bool operator()(Real currentValue, Real newValue, const Array &temp) const {
            Real temperature = *std::max_element(temp.begin(), temp.end());
            return (1.0 / (1.0 + exp((newValue - currentValue) / temperature))) > uniform_();
        }
    private:
        base_generator_type generator_;
        uniform distribution_;
        mutable uniform_variate uniform_;
    };
    //! Boltzmann Downhill Probability
    /*!    Similarly to the Boltzmann Probability, but if new < current, then the point is
    always accepted.
    */
    class ProbabilityBoltzmannDownhill
    {
    public:
        explicit ProbabilityBoltzmannDownhill(unsigned long seed = SeedGenerator::instance().get()) :
            generator_(seed),
            uniform_(generator_, distribution_) {};
        ProbabilityBoltzmannDownhill(const ProbabilityBoltzmannDownhill& probability) :
            generator_(probability.uniform_.engine()), distribution_(probability.uniform_.distribution()),
            uniform_(generator_, distribution_) {};
        inline bool operator()(Real currentValue, Real newValue, const Array &temp) const {
            if (newValue < currentValue)
                return true;
            double mTemperature = *std::max_element(temp.begin(), temp.end());
            return (1.0 / (1.0 + exp((newValue - currentValue) / mTemperature))) > uniform_();
        }
    private:
        base_generator_type generator_;
        uniform distribution_;
        mutable uniform_variate uniform_;
    };
    //! Temperature Boltzmann
    /*!    For use with the Gaussian sampler
    */
    class TemperatureBoltzmann {
    public:
        TemperatureBoltzmann(Real initialTemp, Size dimension)
            : initialTemp_(dimension, initialTemp) {}
        inline void operator()(Array &newTemp, const Array &currTemp, const Array &steps) const {
            QL_REQUIRE(currTemp.size() == initialTemp_.size(), "Incompatible input");
            QL_REQUIRE(currTemp.size() == newTemp.size(), "Incompatible input");
            for (Size i = 0; i < initialTemp_.size(); i++)
                newTemp[i] = initialTemp_[i] / log(steps[i]);
        }
    private:
        Array initialTemp_;
    };
    //! Temperature Cauchy
    /*!    For use with the Cauchy sampler
    */
    class TemperatureCauchy {
    public:
        TemperatureCauchy(Real initialTemp, Size dimension)
            : initialTemp_(dimension, initialTemp) {}
        inline void operator()(Array &newTemp, const Array &currTemp, const Array &steps) const {
            QL_REQUIRE(currTemp.size() == initialTemp_.size(), "Incompatible input");
            QL_REQUIRE(currTemp.size() == newTemp.size(), "Incompatible input");
            for (Size i = 0; i < initialTemp_.size(); i++)
                newTemp[i] = initialTemp_[i] / steps[i];
        }
    private:
        Array initialTemp_;
    };

    class TemperatureCauchy1D {
    public:
        TemperatureCauchy1D(Real initialTemp, Size dimension) :
            inverseN_(1.0 / dimension),
            initialTemp_(dimension, initialTemp) {}
        inline void operator()(Array &newTemp, const Array &currTemp, const Array &steps) const {
            QL_REQUIRE(currTemp.size() == initialTemp_.size(), "Incompatible input");
            QL_REQUIRE(currTemp.size() == newTemp.size(), "Incompatible input");
            for (Size i = 0; i < initialTemp_.size(); i++)
                newTemp[i] = initialTemp_[i] / std::pow(steps[i], inverseN_);
        }
    private:
        Real inverseN_;
        Array initialTemp_;
    };

    class TemperatureExponential {
    public:
        TemperatureExponential(Real initialTemp, Size dimension, Real power = 0.95)
            : initialTemp_(dimension, initialTemp), power_(power) {}
        inline void operator()(Array &newTemp, const Array &currTemp, const Array &steps) const {
            QL_REQUIRE(currTemp.size() == initialTemp_.size(), "Incompatible input");
            QL_REQUIRE(currTemp.size() == newTemp.size(), "Incompatible input");
            for (Size i = 0; i < initialTemp_.size(); i++)
                newTemp[i] = initialTemp_[i] * std::pow(power_, steps[i]);
        }
    private:
        Array initialTemp_;
        Real power_;
    };
    //! Temperature Very Fast Annealing
    /*!    For use with the Very Fast Annealing sampler
    */
    class TemperatureVeryFastAnnealing {
    public:
        TemperatureVeryFastAnnealing(Real initialTemp, Real finalTemp, Real maxSteps, Size dimension)
            :inverseN_(1.0 / dimension), initialTemp_(dimension, initialTemp),
            finalTemp_(dimension, finalTemp), exponent_(dimension, 0.0) {
            Real coeff = std::pow(maxSteps, -inverseN_);
            for (Size i = 0; i < initialTemp_.size(); i++)
                exponent_[i] = -log(finalTemp_[i] / initialTemp_[i])*coeff;
        }
        inline void operator()(Array &newTemp, const Array &currTemp, const Array &steps) const {
            QL_REQUIRE(currTemp.size() == initialTemp_.size(), "Incompatible input");
            QL_REQUIRE(currTemp.size() == newTemp.size(), "Incompatible input");
            for (Size i = 0; i < initialTemp_.size(); i++)
                newTemp[i] = initialTemp_[i] * exp(-exponent_[i] * std::pow(steps[i], inverseN_));
        }
    private:
        Real inverseN_;
        Array initialTemp_, finalTemp_, exponent_;
    };
    //! Reannealing Trivial
    /*!    No reannealing is performed
    */
    struct ReannealingTrivial {
        ReannealingTrivial() = default;
        ;
        inline void setProblem(Problem &P) {};
        inline void operator()(Array & steps, const Array &currentPoint,
            Real aCurrentValue, const Array & currTemp) const {};
    };
    //! Reannealing Finite Difference
    /*!    In multidimensional problems, different dimensions might have different
    sensitivities, and might have dimensions on which the solution is rather
    insensitive. If possible, the search should concentrate more on the more
    sensitive dimensions, therefore a reannealing schedule might raise the
    temperature seen by those more fruitful dimensions so as to allow for more
    movement along the dimensions of interest
    */
    class ReannealingFiniteDifferences {
    public:
        ReannealingFiniteDifferences(Real initialTemp,
            Size dimension,
            const Array & lower = Array(),
            const Array & upper = Array(),
            Real stepSize = 1e-7,
            Real minSize = 1e-10,
            Real functionTol = 1e-10)
            : stepSize_(stepSize), minSize_(minSize),
            functionTol_(functionTol), N_(dimension), bound_(false),
            lower_(lower), upper_(upper), initialTemp_(dimension, initialTemp),
            bounded_(dimension, 1.0) {
            if (!lower.empty() && !upper.empty()) {
                QL_REQUIRE(lower.size() == N_, "Incompatible input");
                QL_REQUIRE(upper.size() == N_, "Incompatible input");
                bound_ = true;
                for (Size i = 0; i < N_; i++) {
                    bounded_[i] = upper[i] - lower[i];
                }
            }
        }
        inline void setProblem(Problem &P) { problem_ = &P; };
        inline void operator()(Array & steps, const Array &currentPoint,
            Real currentValue, const Array & currTemp) const {
            QL_REQUIRE(currTemp.size() == N_, "Incompatible input");
            QL_REQUIRE(steps.size() == N_, "Incompatible input");

            Array finiteDiffs(N_, 0.0);
            double finiteDiffMax = 0.0;
            Array ofssetPoint(currentPoint);
            for (Size i = 0; i < N_; i++) {
                ofssetPoint[i] += stepSize_;
                finiteDiffs[i] = bounded_[i] * std::abs((problem_->value(ofssetPoint) - currentValue) / stepSize_);
                ofssetPoint[i] -= stepSize_;
                if (finiteDiffs[i] < minSize_)
                    finiteDiffs[i] = minSize_;
                if (finiteDiffs[i] > finiteDiffMax)
                    finiteDiffMax = finiteDiffs[i];
            }
            for (Size i = 0; i < N_; i++) {
                Real tRatio = initialTemp_[i] / currTemp[i];
                Real sRatio = finiteDiffMax / finiteDiffs[i];
                if (sRatio*tRatio < functionTol_)
                    steps[i] = std::pow(std::fabs(std::log(functionTol_)),
                                        Integer(N_));
                else
                    steps[i] = std::pow(std::fabs(std::log(sRatio*tRatio)),
                                        Integer(N_));
            }
        }
    private:
        Problem *problem_;
        Real stepSize_, minSize_, functionTol_;
        Size N_;
        bool bound_;
        Array lower_, upper_, initialTemp_, bounded_;
    };
}
#endif // HYBRIDSIMULATEDANNEALINGFUNCTORS_H


#ifndef id_a165767b9f1e022e99272f79a8191030
#define id_a165767b9f1e022e99272f79a8191030
inline bool test_a165767b9f1e022e99272f79a8191030(int* i) { return i != 0; }
#endif
