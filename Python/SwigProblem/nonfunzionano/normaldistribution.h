#ifndef quantlib_normal_distribution_h
#define quantlib_normal_distribution_h

#include "qldefines.h"
#include "qlerrors.h"

#include <functional>

namespace QuantLib {

    namespace Math {

        class NormalDistribution : public std::unary_function<double,double> {
          public:
            NormalDistribution(double average = 0.0, double sigma = 1.0);
            // function
            double operator()(double x) const;
          private:
            static const double pi_;
            double average_, sigma_, normalizationFactor_, denominator_;
        };





    }

}


#endif
