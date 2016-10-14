install(FILES
    all.hpp
    bmaindex.hpp
    iborindex.hpp
    indexmanager.hpp
    inflationindex.hpp
    interestrateindex.hpp
    region.hpp
    swapindex.hpp
    DESTINATION include/ql/indexes)

add_subdirectory(ibor)
add_subdirectory(inflation)
add_subdirectory(swap)
