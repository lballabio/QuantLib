
#include "normaldistribution.h"
#include "dataformatters.h"

namespace QuantLib {

    namespace Math {
    
        const double NormalDistribution::pi_ = 3.14159265358979323846;

        double NormalDistribution::operator()(double x) const {
            double deltax = x-average_;
            return normalizationFactor_*exp(-deltax*deltax/denominator_);
        }

        NormalDistribution::NormalDistribution(double average,
            double sigma)
        : average_(average), sigma_(sigma) {

            QL_REQUIRE(sigma_>0.0,
                "NormalDistribution: sigma must be greater than 0.0 "
                + DoubleFormatter::toString(sigma_) + " not allowed)"
                );

            normalizationFactor_ = 1.0/(sigma_*sqrt(2.0*pi_));
            denominator_ = 2.0*sigma_*sigma_;
        }

    }

}
