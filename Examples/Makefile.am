
AM_CPPFLAGS = -I${top_builddir} -I${top_srcdir}

EXAMPLES = \
    BasketLosses/BasketLosses \
    BermudanSwaption/BermudanSwaption \
    Bonds/Bonds \
    CallableBonds/CallableBonds \
    CDS/CDS \
    ConvertibleBonds/ConvertibleBonds \
    CVAIRS/CVAIRS \
    DiscreteHedging/DiscreteHedging \
    EquityOption/EquityOption \
    FittedBondCurve/FittedBondCurve \
    FRA/FRA \
    Gaussian1dModels/Gaussian1dModels \
    GlobalOptimizer/GlobalOptimizer \
    LatentModel/LatentModel \
    MarketModels/MarketModels \
    MultidimIntegral/MultidimIntegral \
    MulticurveBootstrapping/MulticurveBootstrapping \
    Replication/Replication \
    Repo/Repo


if AUTO_EXAMPLES
bin_PROGRAMS = $(EXAMPLES)
TESTS = $(EXAMPLES)
else
noinst_PROGRAMS = $(EXAMPLES)
endif

LDADDS = ../ql/libQuantLib.la ${PTHREAD_LIB}

BasketLosses_BasketLosses_SOURCES = BasketLosses/BasketLosses.cpp
BasketLosses_BasketLosses_LDADD = $(LDADDS)

BermudanSwaption_BermudanSwaption_SOURCES = BermudanSwaption/BermudanSwaption.cpp
BermudanSwaption_BermudanSwaption_LDADD = $(LDADDS)

Bonds_Bonds_SOURCES = Bonds/Bonds.cpp
Bonds_Bonds_LDADD = $(LDADDS)

CallableBonds_CallableBonds_SOURCES = CallableBonds/CallableBonds.cpp
CallableBonds_CallableBonds_LDADD = $(LDADDS)

CDS_CDS_SOURCES = CDS/CDS.cpp
CDS_CDS_LDADD = $(LDADDS)

ConvertibleBonds_ConvertibleBonds_SOURCES = ConvertibleBonds/ConvertibleBonds.cpp
ConvertibleBonds_ConvertibleBonds_LDADD = $(LDADDS)

CVAIRS_CVAIRS_SOURCES = CVAIRS/CVAIRS.cpp
CVAIRS_CVAIRS_LDADD = $(LDADDS)

DiscreteHedging_DiscreteHedging_SOURCES = DiscreteHedging/DiscreteHedging.cpp
DiscreteHedging_DiscreteHedging_LDADD = $(LDADDS)

EquityOption_EquityOption_SOURCES = EquityOption/EquityOption.cpp
EquityOption_EquityOption_LDADD = $(LDADDS)

FRA_FRA_SOURCES = FRA/FRA.cpp
FRA_FRA_LDADD = $(LDADDS)

FittedBondCurve_FittedBondCurve_SOURCES = FittedBondCurve/FittedBondCurve.cpp
FittedBondCurve_FittedBondCurve_LDADD = $(LDADDS)

Gaussian1dModels_Gaussian1dModels_SOURCES = Gaussian1dModels/Gaussian1dModels.cpp
Gaussian1dModels_Gaussian1dModels_LDADD = $(LDADDS)

GlobalOptimizer_GlobalOptimizer_SOURCES = GlobalOptimizer/GlobalOptimizer.cpp
GlobalOptimizer_GlobalOptimizer_LDADD = $(LDADDS)

LatentModel_LatentModel_SOURCES = LatentModel/LatentModel.cpp
LatentModel_LatentModel_LDADD = $(LDADDS)

MarketModels_MarketModels_SOURCES = MarketModels/MarketModels.cpp
MarketModels_MarketModels_LDADD = $(LDADDS)

MultidimIntegral_MultidimIntegral_SOURCES = MultidimIntegral/MultidimIntegral.cpp
MultidimIntegral_MultidimIntegral_LDADD = $(LDADDS)

MulticurveBootstrapping_MulticurveBootstrapping_SOURCES = MulticurveBootstrapping/MulticurveBootstrapping.cpp
MulticurveBootstrapping_MulticurveBootstrapping_LDADD = $(LDADDS)

Replication_Replication_SOURCES = Replication/Replication.cpp
Replication_Replication_LDADD = $(LDADDS)

Repo_Repo_SOURCES = Repo/Repo.cpp
Repo_Repo_LDADD = $(LDADDS)

EXTRA_DIST = CMakeLists.txt README.txt

dist-hook:
	for i in $(notdir $(EXAMPLES)); do \
	    cp $(srcdir)/$$i/$$i.vcxproj $(distdir)/$$i/ && \
	    cp $(srcdir)/$$i/$$i.vcxproj.filters $(distdir)/$$i/ && \
	    cp $(srcdir)/$$i/CMakeLists.txt $(distdir)/$$i/ \
	; done

.PHONY: check-examples

check-examples: $(EXAMPLES)
	for i in $(EXAMPLES); do echo "========" && echo $$i && $$i || break -1 ; done

