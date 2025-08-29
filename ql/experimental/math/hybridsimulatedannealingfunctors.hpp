/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
Copyright (C) 2015 Andres Hernandez

This file is part of QuantLib, a free-software/open-source library
for financial quantitative analysts and developers - http://quantlib.org/

QuantLib is free software: you can redistribute it and/or modify it
under the terms of the QuantLib license.  You should have received a
copy of the license along with this program; if not, please email
<quantlib-dev@lists.sf.net>. The license is also available online at
<https://www.quantlib.org/license.shtml>.

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

#include <algorithm>
#include <cmath>
#include <random>
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
            generator_(seed), distribution_(0.0, 1.0) {};

        void operator()(Array &newPoint, const Array &currentPoint, const Array &temp) {
            QL_REQUIRE(newPoint.size() == currentPoint.size(), "Incompatible input");
            QL_REQUIRE(newPoint.size() == temp.size(), "Incompatible input");
            for (Size i = 0; i < currentPoint.size(); i++)
                newPoint[i] = currentPoint[i] * exp(sqrt(temp[i]) * distribution_(generator_));
        };
    private:
        std::mt19937 generator_;
        std::normal_distribution<Real> distribution_;
    };

    //! Gaussian Sampler
    /*!    Sample from normal distribution. This means that the parameter space
    must have support on the whole real line.
    */
    class SamplerGaussian
    {
    public:
        explicit SamplerGaussian(unsigned long seed = SeedGenerator::instance().get()) :
            generator_(seed), distribution_(0.0, 1.0) {};

        void operator()(Array &newPoint, const Array &currentPoint, const Array &temp) {
            QL_REQUIRE(newPoint.size() == currentPoint.size(), "Incompatible input");
            QL_REQUIRE(newPoint.size() == temp.size(), "Incompatible input");
            for (Size i = 0; i < currentPoint.size(); i++)
                newPoint[i] = currentPoint[i] + std::sqrt(temp[i]) * distribution_(generator_);
        };
    private:
        std::mt19937 generator_;
        std::normal_distribution<Real> distribution_;
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
      : generator_(seed), distribution_(0.0, 1.0),
        lower_(std::move(lower)), upper_(std::move(upper)){};

        void operator()(Array &newPoint, const Array &currentPoint, const Array &temp) {
            QL_REQUIRE(newPoint.size() == currentPoint.size(), "Incompatible input");
            QL_REQUIRE(newPoint.size() == temp.size(), "Incompatible input");
            for (Size i = 0; i < currentPoint.size(); i++){
                newPoint[i] = currentPoint[i] + std::sqrt(temp[i]) * distribution_(generator_);
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
        std::mt19937 generator_;
        std::normal_distribution<Real> distribution_;
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
      : generator_(seed), distribution_(0.0, 1.0),
        lower_(std::move(lower)), upper_(std::move(upper)){};

        void operator()(Array &newPoint, const Array &currentPoint, const Array &temp) {
            QL_REQUIRE(newPoint.size() == currentPoint.size(), "Incompatible input");
            QL_REQUIRE(newPoint.size() == temp.size(), "Incompatible input");
            for (Size i = 0; i < currentPoint.size(); i++){
                newPoint[i] = currentPoint[i] + std::sqrt(temp[i]) * distribution_(generator_);
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
        std::mt19937 generator_;
        std::normal_distribution<Real> distribution_;
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
            generator_(seed), distribution_(0.0, 1.0) {};

        void operator()(Array &newPoint, const Array &currentPoint, const Array &temp) {
            QL_REQUIRE(newPoint.size() == currentPoint.size(), "Incompatible input");
            QL_REQUIRE(newPoint.size() == temp.size(), "Incompatible input");
            for (Size i = 0; i < currentPoint.size(); i++)
                newPoint[i] = currentPoint[i] + temp[i] * distribution_(generator_);
        };
    protected:
        std::mt19937 generator_;
        std::cauchy_distribution<Real> distribution_;
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
        : lower_(std::move(lower)), upper_(std::move(upper)), generator_(seed) {
            QL_REQUIRE(lower_.size() == upper_.size(), "Incompatible input");
        };

        void operator()(Array &newPoint, const Array &currentPoint, const Array &temp) {
            QL_REQUIRE(newPoint.size() == currentPoint.size(), "Incompatible input");
            QL_REQUIRE(newPoint.size() == lower_.size(), "Incompatible input");
            QL_REQUIRE(newPoint.size() == temp.size(), "Incompatible input");
            for (Size i = 0; i < currentPoint.size(); i++) {
                newPoint[i] = lower_[i] - 1.0;
                while (newPoint[i] < lower_[i] || newPoint[i] > upper_[i]) {
                    Real draw = distribution_(generator_);
                    Real sign = static_cast<int>(0.5 < draw) - static_cast<int>(draw < 0.5);
                    Real y = sign*temp[i] * (std::pow(1.0 + 1.0 / temp[i],
                                                      std::abs(2 * draw - 1.0)) - 1.0);
                    newPoint[i] = currentPoint[i] + y*(upper_[i] - lower_[i]);
                }
            }
        };
    private:
        Array lower_, upper_;
        std::mt19937 generator_;
        std::uniform_real_distribution<Real> distribution_;
    };

    //! Always Downhill Probability
    /*!    Only points that improve on the current solution are accepted.
    Depending on the problem, this makes it very unlikely that the
    optimizer will be able to escape a local optimum.
    */
    struct ProbabilityAlwaysDownhill {
        bool operator()(Real currentValue, Real newValue, const Array &temp) {
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
        explicit ProbabilityBoltzmann(unsigned long seed = SeedGenerator::instance().get()) : generator_(seed) {};

        bool operator()(Real currentValue, Real newValue, const Array &temp) {
            Real temperature = *std::max_element(temp.begin(), temp.end());
            return (1.0 / (1.0 + exp((newValue - currentValue) / temperature))) > distribution_(generator_);
        }
    private:
        std::mt19937 generator_;
        std::uniform_real_distribution<Real> distribution_;
    };
    //! Boltzmann Downhill Probability
    /*!    Similarly to the Boltzmann Probability, but if new < current, then the point is
    always accepted.
    */
    class ProbabilityBoltzmannDownhill
    {
    public:
        explicit ProbabilityBoltzmannDownhill(unsigned long seed = SeedGenerator::instance().get()) : generator_(seed) {};

        bool operator()(Real currentValue, Real newValue, const Array &temp) {
            if (newValue < currentValue)
                return true;
            Real mTemperature = *std::max_element(temp.begin(), temp.end());
            return (1.0 / (1.0 + exp((newValue - currentValue) / mTemperature))) > distribution_(generator_);
        }
    private:
        std::mt19937 generator_;
        std::uniform_real_distribution<Real> distribution_;
    };
    //! Temperature Boltzmann
    /*!    For use with the Gaussian sampler
    */
    class TemperatureBoltzmann {
    public:
        TemperatureBoltzmann(Real initialTemp, Size dimension)
            : initialTemp_(dimension, initialTemp) {}
        void operator()(Array &newTemp, const Array &currTemp, const Array &steps) {
            QL_REQUIRE(currTemp.size() == initialTemp_.size(), "Incompatible input");
            QL_REQUIRE(currTemp.size() == newTemp.size(), "Incompatible input");
            for (Size i = 0; i < initialTemp_.size(); i++)
                newTemp[i] = initialTemp_[i] / std::log(steps[i]);
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
        void operator()(Array &newTemp, const Array &currTemp, const Array &steps) {
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
        void operator()(Array &newTemp, const Array &currTemp, const Array &steps) {
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
        void operator()(Array &newTemp, const Array &currTemp, const Array &steps) {
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
                exponent_[i] = -std::log(finalTemp_[i] / initialTemp_[i])*coeff;
        }
        void operator()(Array &newTemp, const Array &currTemp, const Array &steps) {
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
        void setProblem(Problem &P) {};
        void operator()(Array & steps, const Array &currentPoint,
            Real aCurrentValue, const Array & currTemp) {};
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
                                   const Array& lower = Array(),
                                   const Array& upper = Array(),
                                   Real stepSize = 1e-7,
                                   Real minSize = 1e-10,
                                   Real functionTol = 1e-10)
      : stepSize_(stepSize), minSize_(minSize), functionTol_(functionTol), N_(dimension),
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
        void setProblem(Problem &P) { problem_ = &P; };
        void operator()(Array & steps, const Array &currentPoint,
            Real currentValue, const Array & currTemp) {
            QL_REQUIRE(currTemp.size() == N_, "Incompatible input");
            QL_REQUIRE(steps.size() == N_, "Incompatible input");

            Array finiteDiffs(N_, 0.0);
            Real finiteDiffMax = 0.0;
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
        bool bound_ = false;
        Array lower_, upper_, initialTemp_, bounded_;
    };
}
#endif // HYBRIDSIMULATEDANNEALINGFUNCTORS_H
