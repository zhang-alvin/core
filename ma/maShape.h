/****************************************************************************** 

  Copyright 2013 Scientific Computation Research Center, 
      Rensselaer Polytechnic Institute. All rights reserved.
  
  The LICENSE file included with this distribution describes the terms
  of the SCOREC Non-Commercial License this program is distributed under.
 
*******************************************************************************/
#ifndef MA_SHAPE
#define MA_SHAPE

#include "maMesh.h"
#include "maTables.h"

namespace ma {

class SizeField;
class Adapt;

/* quick check of positivity of volumes based on vertices */
bool areTetsValid(Mesh* m, EntityArray& tets);

double measureTriQuality(Mesh* m, SizeField* f, Entity* tri, bool useMax=true);
double measureTetQuality(Mesh* m, SizeField* f, Entity* tet, bool useMax=true);
double measureElementQuality(Mesh* m, SizeField* f, Entity* e, bool useMax=true);

/* gets the quality of an element based on
 * the vertices used for curved elements
 */
double measureLinearTetQuality(Vector xyz[4]);
double measureQuadraticTetQuality(Mesh* m, Entity* tet);

double getWorstQuality(Adapt* a, EntityArray& e);
double getWorstQuality(Adapt* a, Entity** e, size_t n);

/* has worse quality than qualityToBeat
 */
bool hasWorseQuality(Adapt* a, EntityArray& e, double qualityToBeat);

/* measures the min and max edge lengths (in metric space)
 * among all the entities in tets
 */
Entity* getMaxEdgeLength(Adapt* a, EntityArray& tets, double& maxLength);
Entity* getMinEdgeLength(Adapt* a, EntityArray& tets, double& minLength);

/* checks whether a prism is safe to tetrahedronize.
 * the optional "good_diagonal_codes" integer
 * is a bit vector containing 8 bits, one
 * for each of the possible 3-bit diagonal
 * codes, indicating whether that diagonal
 * configuration is safe.
 */
bool isPrismOk(apf::Mesh* m, Entity* e,
    int* good_diagonal_codes = 0);
/* checks whether a pyramid is safe to tetrahedronize.
   good_rotation gives additional information if the pyramid
   is unsafe:
     -1  there is no safe way to tetrahedronize it
     0   the 0--2 diagonal is safe
     1   the 1--3 diagonal is safe  */
bool isPyramidOk(apf::Mesh* m, Entity* e,
    int* good_rotation = 0);
bool isLayerElementOk(Mesh* m, Entity* e);

CodeMatch matchSliver(
    Mesh* m,
    Entity* tet);

double improveQualities(Adapt* a);
void fixElementShapes(Adapt* a);
void alignElements(Adapt* a);
void printQuality(Adapt* a);

}

#endif
