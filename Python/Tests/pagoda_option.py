
from QuantLib import Matrix
from QuantLib import getCovariance
from QuantLib import PagodaOption
from TestUnit import TestUnit
from math import fabs

class PagodaTest(TestUnit):
    def doTest(self):
        c = Matrix(7,7,0.0)
        c[0][0] = 1    ; c[0][1] = -0.02 ; c[0][2] =  0.315; c[0][3] =  0.127; c[0][4] =  0.348; c[0][5] =  0.357; c[0][6] =  0.112
        c[1][0] =-0.02 ; c[1][1] =  1    ; c[1][2] =  0.203; c[1][3] = -0.739; c[1][4] = -0.254; c[1][5] =  0.406; c[1][6] = -0.244
        c[2][0] = 0.315; c[2][1] =  0.203; c[2][2] =  1    ; c[2][3] = -0.312; c[2][4] =  0.302; c[2][5] = -0.216; c[2][6] = -0.616
        c[3][0] = 0.127; c[3][1] = -0.739; c[3][2] = -0.312; c[3][3] =  1    ; c[3][4] =  0.238; c[3][5] = -0.154; c[3][6] =  0.362
        c[4][0] = 0.348; c[4][1] = -0.254; c[4][2] =  0.302; c[4][3] =  0.238; c[4][4] =  1    ; c[4][5] = -0.35 ; c[4][6] = -0.439
        c[5][0] = 0.357; c[5][1] =  0.406; c[5][2] = -0.216; c[5][3] = -0.154; c[5][4] = -0.35 ; c[5][5] =  1    ; c[5][6] =  0.381
        c[6][0] = 0.112; c[6][1] = -0.244; c[6][2] = -0.616; c[6][3] =  0.362; c[6][4] = -0.439; c[6][5] =  0.381; c[6][6] =  1
         
        volatilities = [0.325, 0.365,  0.235,   0.426,   0.365,   0.377,   0.228]
        divYield  = [0.03807, 0.01227, 0.02489, 0.09885, 0.01244, 0.00466, 0.10827]
        divYield  = [0.0003807, 0.0001227, 0.0002489, 0.0009885, 0.0001244, 0.0000466, 0.0010827]
        portfolio = [0.10,    0.20,    0.20,    0.20,    0.10,    0.10,    0.10]
         
        fraction = 0.62
        roof = 0.20
        residualTime = 1
        riskFreeRate = 0.05
        timesteps = 12
        samples = 2000
        seed = 86421
        
        cov = getCovariance(volatilities, c)
        storedValue = 0.0411470297914
        storedError = 0.000936103530233
        
        pagoda = PagodaOption(portfolio,
                    fraction, roof, residualTime,
                    cov, divYield, riskFreeRate,
                    timesteps, samples, seed);        

        val = pagoda.value()
        err = pagoda.errorEstimate()
        self.printDetails(val, err)
        if fabs(storedValue-val) + fabs(storedError-err) < 1e-12:
            return 0
        else:
            return 1

if __name__ == '__main__':
    PagodaTest().test('pagoda option pricer')


