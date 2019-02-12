/****************************************************************************** 

  Copyright 2013 Scientific Computation Research Center, 
      Rensselaer Polytechnic Institute. All rights reserved.
  
  The LICENSE file included with this distribution describes the terms
  of the SCOREC Non-Commercial License this program is distributed under.
 
*******************************************************************************/
#include <PCU.h>
#include "ma.h"
#include "maAdapt.h"
#include "maCoarsen.h"
#include "maRefine.h"
#include "maSnap.h"
#include "maShape.h"
#include "maBalance.h"
#include "maLayer.h"
#include "maDBG.h"
#include <pcu_util.h>
#include <cstdlib>
#include <iostream>

namespace ma {

void adapt(Input* in)
{
  print("version 2.0 !");
  double t0 = PCU_Time();
  validateInput(in);
  Adapt* a = new Adapt(in);
  preBalance(a);
  for (int i = 0; i < in->maximumIterations; ++i)
  {
    print("iteration %d",i);
    coarsen(a);
    coarsenLayer(a);
    midBalance(a);
    refine(a);
    snap(a);
  }
  allowSplitCollapseOutsideLayer(a);
  fixElementShapes(a);
  cleanupLayer(a);
  tetrahedronize(a);
  printQuality(a);
  postBalance(a);
  Mesh* m = a->mesh;
  delete a;
  delete in;
  double t1 = PCU_Time();
  print("mesh adapted in %f seconds",t1-t0);
  apf::printStats(m);
}

void checkEmpty(apf::Mesh* m)
{
  int numElements = m->count(m->getDimension());
  int numVertices = m->count(0);
  if(numElements == 0 )
  {
    std::cerr << "Element count reached 0\n";
    std::exit(EXIT_FAILURE);
  }
  else if(numVertices == 0 )
  {
    
    std::cerr << "Vertex count reached 0\n";
    std::exit(EXIT_FAILURE);
  }
  return;
}

void adaptVerbose(Input* in, bool verbose)
{
  print("version 2.0 - dev !");
  double t0 = PCU_Time();
  validateInput(in);
  Adapt* a = new Adapt(in);
 
  preBalance(a);
  checkEmpty(a->mesh);
  for (int i = 0; i < in->maximumIterations; ++i)
  {
    print("iteration %d",i);
    coarsen(a);
    if (verbose && in->shouldCoarsen)
      ma_dbg::dumpMeshWithQualities(a,i,"after_coarsen");
    coarsenLayer(a);
    midBalance(a);
    checkEmpty(a->mesh);
    refine(a);
    if (verbose)
      ma_dbg::dumpMeshWithQualities(a,i,"after_refine");
    snap(a);
    if (verbose && in->shouldSnap)
      ma_dbg::dumpMeshWithQualities(a,i,"after_snap");
    fixElementShapes(a);
    if (verbose && in->shouldFixShape)
      ma_dbg::dumpMeshWithQualities(a,i,"after_fix");
  }
  allowSplitCollapseOutsideLayer(a);
  fixElementShapes(a);
  if (verbose) ma_dbg::dumpMeshWithQualities(a,999,"after_final_fix");
  /* The following loop ensures that no long edges are left in
   * the mesh. Note that at this point all elements are of "good"
   * quality and a few refinement iterations will not depreciate
   * the overall quality of the mesh, significantly.
   */
  int count = 0;
  double lMax = ma::getMaximumEdgeLength(a->mesh, a->sizeField);
  print("Maximum (metric) edge length in the mesh is %f", lMax);
  while (lMax > 1.5) {
    print("%dth additional refine-snap call", count);
    refine(a);
    snap(a);
    lMax = ma::getMaximumEdgeLength(a->mesh, a->sizeField);
    count++;
    print("Maximum (metric) edge length in the mesh is %f", lMax);
    if (count > 5) break;
  }
  if (verbose)
    ma_dbg::dumpMeshWithQualities(a,999,"after_final_refine_snap_loop");
  printQuality(a);
  improveQualities(a);
  if (verbose)
    ma_dbg::dumpMeshWithQualities(a,999,"after_improveQualities");
  cleanupLayer(a);
  tetrahedronize(a);
  printQuality(a);
  postBalance(a);
  checkEmpty(a->mesh);
  Mesh* m = a->mesh;
  delete a;
  delete in;
  double t1 = PCU_Time();
  print("mesh adapted in %f seconds",t1-t0);
  apf::printStats(m);
}


void adapt(Mesh* m, AnisotropicFunction* f, SolutionTransfer* s)
{
  adapt(configure(m,f,s));
}

void adapt(Mesh* m, IsotropicFunction* f, SolutionTransfer* s)
{
  adapt(configure(m,f,s));
}

void runUniformRefinement(Mesh* m, int n, SolutionTransfer* s)
{
  adapt(configureUniformRefine(m,n,s));
}

void adaptMatching(Mesh* m, int n, SolutionTransfer* s)
{
  adapt(configureMatching(m,n,s));
}

}
