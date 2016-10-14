install(FILES
    all.hpp
    lognormalcmswapratepc.hpp
    lognormalcotswapratepc.hpp
    lognormalfwdrateballand.hpp
    lognormalfwdrateeuler.hpp
    lognormalfwdrateeulerconstrained.hpp
    lognormalfwdrateiballand.hpp
    lognormalfwdrateipc.hpp
    lognormalfwdratepc.hpp
    marketmodelvolprocess.hpp
    normalfwdratepc.hpp
    svddfwdratepc.hpp
    DESTINATION include/ql/models/marketmodels/evolvers)

add_subdirectory(volprocesses)
