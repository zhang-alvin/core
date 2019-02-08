#include <stdio.h>
#include <SimUtil.h>
#include <MeshSim.h>
#include <SimModel.h>
#include "gmi_sim.h"
#include "gmi_mesh.h"

int main(int argc, char** argv)
{
  if (argc != 3) {
    printf("Convert parasolid or simmetrix geomsim model to "
           "a gmi topological model\n");
    printf("Usage: %s <input model> <output model prefix>.dmg\n", argv[0]);
    return 0;
  }
  MS_init();
  SimModel_start();
  Sim_readLicenseFile(0);
  gmi_sim_start();
  gmi_register_mesh();
  gmi_register_sim();
  gmi_model* m = gmi_load(argv[1]);
  gmi_write_dmg(m, argv[2]);
  gmi_destroy(m);
  gmi_sim_stop();
  Sim_unregisterAllKeys();
  SimModel_stop();
  MS_exit();
  return 0;
}
