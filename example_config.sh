cmake .. \
  -DCMAKE_C_COMPILER="mpicc" \
  -DCMAKE_CXX_COMPILER="mpicxx" \
  -DENABLE_SIMMETRIX=True \
  -DSIM_MPI="mpich3.1.2" \
  -DCMAKE_INSTALL_PREFIX="/lore/zhanga/core-zhang-alvin/install" \
  -DENABLE_ZOLTAN=ON \
  -DSIM_PARASOLID=ON \
  -DBUILD_SHARED_LIBS=True
