#include <ql/experimental/math/multidimintegrator.hpp>

namespace QuantLib {

    /* a higher than this number of dimension would presumably be impractical
       and another integration algorithm (MC) should be considered.
       Is this a library limit to define at library configuration?
    */
    const Size GaussianQuadMultidimIntegrator::maxNumFactors_ = 30; // move to header

    GaussianQuadMultidimIntegrator::GaussianQuadMultidimIntegrator(
        Size dimension, Size quadOrder, Real mu) 
        : integral_(quadOrder, mu),
          integralV_(quadOrder, mu),
          integrationEntries_(maxNumFactors_),
  /////        integrationEntriesV_(maxNumFactors_),
integrationEntriesVR_(maxNumFactors_),
          dimension_(dimension),
          integrandFctrs(dimension_, 0.)
        {
            // need a test on dimension integrity
            spawnFcts<maxNumFactors_>();
        }


    MultidimIntegral::MultidimIntegral(
            const std::vector<boost::shared_ptr<Integrator> >& integrators)
    : integrators_(integrators), 
      integrationLevelEntries_(maxDimensions_+1),
      varBuffer_() {
        QL_REQUIRE(integrators.size() <= maxDimensions_, 
            "Too many dimensions in integration.");
        spawnFcts<maxDimensions_>();
    }

}
