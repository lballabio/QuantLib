
// Simple definition of the discretized Black-Scholes operator.
class BlackScholesOperator : public TridiagonalOperator {
  public:
    BlackScholesOperator(
        double sigma, double nu,    // parameters of the
        Rate r,                     // Black-Scholes equation;
        int points,                 // number of discretized points;
        double h)                   // grid spacing.
    : TridiagonalOperator(
        // build the operator by adding basic ones
        - (sigma*sigma/2.0) * DPlusDMinus(points,h)
        - nu * DZero(points,h)
        + r * Identity<Array>(points)
    ) {}
};

// input the parameters of the problem - omitted

// instantiate the operator with the given parameters
TridiagonalOperator L = BlackScholesOperator(sigma, nu, r, points, h);

// choose the finite difference model (we use the Crank-Nicolson scheme)...
typedef FiniteDifferenceModel<CrankNicolson<TridiagonalOperator> > FDModel;
// ...and instantiate it
FDModel model(L);

// calculate initial condition - omitted
Array u = ...;

// now tell the model to roll it back from maturity to now (t=0)
unsigned int timeSteps = 100;
model.rollback(u,timeToMaturity,0,timeSteps);

// u now contains the solution. 
// Its central value is the one we're interested in.
double price = valueAtCenter(u);

// all done
