#include <apf.h>
#include <apfMesh.h>
#include <apfMDS.h>
#include <gmi_mesh.h>
#include <PCU.h>
#include <lionPrint.h>
#ifdef HAVE_SIMMETRIX
#include <gmi_sim.h>
#include <SimUtil.h>
#endif
#include <chef.h>
#include <parma.h>
#include <pcu_util.h>
#include <cstdlib>

namespace {
  static FILE* openfile_read(ph::Input&, const char* path) {
    return fopen(path, "r");
  }

  struct GroupCode : public Parma_GroupCode {
    apf::Mesh2* mesh;
    ph::Input ctrl;
    void run(int) {
      apf::reorderMdsMesh(mesh,NULL);
      chef::preprocess(mesh,ctrl);
    }
  };

  void checkInputs(int argc, char** argv) {
    if ( argc != 3 ) {
      if ( !PCU_Comm_Self() )
        lion_oprint(1,"Usage: %s <control .inp> <reduction-factor>\n", argv[0]);
      MPI_Finalize();
      exit(EXIT_FAILURE);
    }
    if ( !PCU_Comm_Self() )
      lion_oprint(1,"Input control file %s reduction factor %s\n", argv[1], argv[2]);
  }
}

int main(int argc, char** argv) {
  MPI_Init(&argc,&argv);
  PCU_Comm_Init();
  PCU_Protect();
  lion_set_verbosity(1);
  checkInputs(argc,argv);
#ifdef HAVE_SIMMETRIX
  Sim_readLicenseFile(0);
  gmi_sim_start();
  gmi_register_sim();
#endif
  gmi_register_mesh();
  GroupCode code;
  ph::Input in;
  in.load(argv[1]);
  in.openfile_read = openfile_read;
  code.mesh = apf::loadMdsMesh(in.modelFileName.c_str(), in.meshFileName.c_str());
  chef::readAndAttachFields(in,code.mesh);
  apf::Unmodulo outMap(PCU_Comm_Self(), PCU_Comm_Peers());
  code.ctrl = in;
  Parma_ShrinkPartition(code.mesh, atoi(argv[2]), code);
#ifdef HAVE_SIMMETRIX
  gmi_sim_stop();
  Sim_unregisterAllKeys();
#endif
  PCU_Comm_Free();
  MPI_Finalize();
}
