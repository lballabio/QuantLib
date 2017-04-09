
#include <ql/experimental/math/multidimintegrator.hpp>
#include <ql/experimental/math/multidimquadrature.hpp>
#include <ql/math/integrals/trapezoidintegral.hpp>

#include <boost/function.hpp>
#include <boost/make_shared.hpp>
#include <boost/timer.hpp>

#include <iostream>
#include <iomanip>

using namespace QuantLib;
using namespace std;

#if defined(QL_ENABLE_SESSIONS)
namespace QuantLib {

    Integer sessionId() { return 0; }

}
#endif

// Correct value is: (e^{-.25} \sqrt{\pi})^{dimension}
struct integrand {
    Real operator()(const std::vector<Real>& arg) const {
        Real sum = 1.;
        for(Size i=0; i<arg.size(); i++) 
            sum *= std::exp(-arg[i]*arg[i]) * std::cos(arg[i]);
        return sum;
    }
};

int main() {
    boost::timer timer;
    std::cout << std::endl;

    /* 
    Integrates the function above over several dimensions, the size of the 
    vector argument is the dimension one.
    Both algorithms are not really on the same stand since the quadrature 
    will be incorrect to use if the integrand is not appropiately behaved. Over 
    dimension 3 you might need to modify the points in the integral to retain a 
    sensible computing time.
    */
    Size dimension = 3;
    Real exactSol = std::pow(std::exp(-.25) * 
        std::sqrt(M_PI), static_cast<Real>(dimension));

    boost::function<Real(const std::vector<Real>& arg)> f = integrand();

    #ifndef QL_PATCH_SOLARIS
    GaussianQuadMultidimIntegrator intg(dimension, 15);

    timer.restart();
    Real valueQuad = intg(f);
    Real secondsQuad = timer.elapsed();
    #endif

    std::vector<boost::shared_ptr<Integrator> > integrals;
    for(Size i=0; i<dimension; i++)
        integrals.push_back(
        boost::make_shared<TrapezoidIntegral<Default> >(1.e-4, 20));
    std::vector<Real> a_limits(integrals.size(), -4.);
    std::vector<Real> b_limits(integrals.size(), 4.);
    MultidimIntegral testIntg(integrals);

    timer.restart();
    Real valueGrid = testIntg(f, a_limits, b_limits);
    Real secondsGrid = timer.elapsed();

    cout << fixed << setprecision(4);
    cout << endl << "-------------- " << endl
         << "Exact: " << exactSol << endl
        #ifndef QL_PATCH_SOLARIS
         << "Quad: " << valueQuad << endl
        #endif
         << "Grid: " << valueGrid << endl
         << endl;

    cout
        #ifndef QL_PATCH_SOLARIS
        << "Seconds for Quad: " << secondsQuad << endl
        #endif
        << "Seconds for Grid: " << secondsGrid << endl;
    return 0;
}
