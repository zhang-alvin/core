#include <apf.h>
#include <apfMDS.h>
#include <apfMesh2.h>
#include <gmi_null.h>
#include <gmi_mesh.h>
#include <PCU.h>
#include <lionPrint.h>
#include <pcu_util.h>
#include <cstdlib>

int main(int argc, char** argv)
{
  MPI_Init(&argc,&argv);
  PCU_Comm_Init();
  lion_set_verbosity(1);
  if ( argc != 3 ) {
    if ( !PCU_Comm_Self() )
      printf("Create a topological geometric model from a mesh\n"
             "Usage: %s <mesh> <out model (.dmg)>\n", argv[0]);
    MPI_Finalize();
    exit(EXIT_FAILURE);
  }
  gmi_register_null();
  gmi_register_mesh();
  apf::Mesh2* m = apf::loadMdsMesh(".null", argv[1]);
  apf::deriveMdsModel(m);
  gmi_model* g = m->getModel();
  gmi_write_dmg(g, argv[2]);
  m->destroyNative();
  apf::destroyMesh(m);
  PCU_Comm_Free();
  MPI_Finalize();
}

