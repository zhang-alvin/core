#include "phBC.h"
#include <apf.h>
#include <apfMesh.h>
#include <fstream>
#include <sstream>
#include <PCU.h>
#include <gmi.h>

namespace ph {

BC::BC()
{
  values = 0;
}

BC::~BC()
{
  delete [] values;
}

bool BC::operator<(const BC& other) const
{
  if (dim != other.dim)
    return dim < other.dim;
  return tag < other.tag;
}

static struct { const char* name; int size; } const knownSizes[4] =
{{"initial velocity", 3}
,{"comp1", 4}
,{"comp3", 4}
,{"traction vector", 3}
};

static int getSize(std::string const& name)
{
  for (int i = 0; i < 4; ++i)
    if (name == knownSizes[i].name)
      return knownSizes[i].size;
  return 1;
}

static void readBC(std::string const& line, BCs& bcs)
{
  std::stringstream ss(line);
  std::string name;
  std::getline(ss, name, ':');
  if (!bcs.fields.count(name)) {
    FieldBCs fbcs;
    fbcs.size = getSize(name);
    bcs.fields[name] = fbcs;
  }
  FieldBCs& fbcs = bcs.fields[name];
  BC bc;
  ss >> bc.tag >> bc.dim;
  bc.values = new double[fbcs.size];
  for (int i = 0; i < fbcs.size; ++i)
    ss >> bc.values[i];
  fbcs.bcs.insert(bc);
  bc.values = 0; //ownership of pointer transferred, prevent delete from here
}

void readBCs(const char* filename, BCs& bcs)
{
  double t0 = MPI_Wtime();
  std::ifstream file(filename);
  std::string line;
  while (std::getline(file, line, '\n')) {
    if (line[0] == '#')
      continue;
    readBC(line, bcs);
  }
  double t1 = MPI_Wtime();
  if (!PCU_Comm_Self())
    printf("\"%s\" loaded in %f seconds\n", filename, t1 - t0);
}

struct KnownBC
{
  const char* name;
  int offset;
  int bit;
  void (*apply)(double* values, int* bits,
    struct KnownBC const& bc, double* inval);
};

static void applyScalar(double* values, int* bits,
    double value, int offset, int bit)
{
  if (offset != -1)
    values[offset] = value;
  if (bit != -1)
    *bits |= (1<<bit);
}

static void applyScalar(double* outval, int* bits,
    KnownBC const& bc, double* inval)
{
  applyScalar(outval, bits, *inval, bc.offset, bc.bit);
}

static void applyMagnitude(double* values, KnownBC const& bc, double v)
{
  values[bc.offset + 3] = v;
}

static void applyComp1(double* values, int* bits,
    KnownBC const& bc, double* inval)
{
  int best = 0;
  for (int i = 1; i < 3; ++i)
    if (fabs(inval[i]) > fabs(inval[best]))
      best = i;
  apf::Vector3 v(inval);
  double mag = v.getLength();
  v = v / mag;
  applyScalar(values, bits, v[best], bc.offset + best, bc.bit + best);
  applyMagnitude(values, bc, mag);
}

static void applyComp3(double* values, int* bits,
    KnownBC const& bc, double* inval)
{
  apf::Vector3 v(inval);
  double mag = v.getLength();
  v = v / mag;
  for (int i = 0; i < 3; ++i)
    applyScalar(values, bits, v[i], bc.offset + i, bc.bit + i);
  applyMagnitude(values, bc, mag);
}

static void applyVector(double* values, int* bits,
    KnownBC const& bc, double* inval)
{
  for (int i = 0; i < 3; ++i)
    values[bc.offset + i] = inval[i];
  if (bc.bit != -1)
    *bits |= (1<<bc.bit);
}

static void applySurfID(double* values, int* bits,
    KnownBC const& bc, double* inval)
{
  bits[1] = *inval;
}

static KnownBC const essentialBCs[9] = {
  {"density",          0, 0, applyScalar},
  {"temperature",      1, 1, applyScalar},
  {"pressure",         2, 2, applyScalar},
  {"comp1",           3, 3, applyComp1},
  {"comp3",           3, 3, applyComp3},
  {"scalar_1",        12, 6, applyScalar},
  {"scalar_2",        13, 7, applyScalar},
  {"scalar_3",        14, 8, applyScalar},
  {"scalar_4",        15, 9, applyScalar},
};

static KnownBC const naturalBCs[10] = {
  {"mass flux",        0, 0, applyScalar},
  {"natural pressure", 1, 1, applyScalar},
  {"traction vector",  2, 2, applyVector},
  {"heat flux",        5, 3, applyScalar},
  {"turbulence wall", -1, 4, applyScalar},
  {"scalar_1 flux",    6, 5, applyScalar},
  {"scalar_2 flux",    7, 6, applyScalar},
  {"scalar_3 flux",    8, 7, applyScalar},
  {"scalar_4 flux",    9, 8, applyScalar},
  {"surf ID",         -1,-1, applySurfID},
};

static KnownBC const solutionBCs[7] = {
  {"initial pressure",         0,-1, applyScalar},
  {"initial velocity",         1,-1, applyVector},
  {"initial temperature",      4,-1, applyScalar},
  {"initial scalar_1",         5,-1, applyScalar},
  {"initial scalar_2",         6,-1, applyScalar},
  {"initial scalar_3",         7,-1, applyScalar},
  {"initial scalar_4",         8,-1, applyScalar},
};

/* starting from the current geometric entity,
   try to find an attribute (kbc) attached to
   a geometric entity by searching all upward
   adjacencies.
ex: for a mesh vertex classified on a model
    vertex, the model vertex is first checked
    for the attribute, then the model edges
    adjacent to the model vertex, then the model
    faces adjacent to those model edges.
   this is done by the following recursive function. */
double* checkForBC(gmi_model* gm, gmi_ent* ge,
    BCs& bcs, KnownBC const& kbc)
{
  std::string name(kbc.name);
  if (!bcs.fields.count(name))
    return 0;
  FieldBCs& fbcs = bcs.fields[name];
  BC key;
  key.tag = gmi_tag(gm, ge);
  key.dim = gmi_dim(gm, ge);
  FieldBCs::Set::iterator it = fbcs.bcs.find(key);
  if (it != fbcs.bcs.end()) {
    BC& bc = const_cast<BC&>(*it);
    return bc.values;
  }
  gmi_set* up = gmi_adjacent(gm, ge, key.dim + 1);
  for (int i = 0; i < up->n; ++i) {
    double* v = checkForBC(gm, up->e[i], bcs, kbc);
    if (v)
      return v;
  }
  gmi_free_set(up);
  return 0;
}

bool applyBCs(apf::Mesh* m, apf::MeshEntity* e,
    BCs& appliedBCs,
    KnownBC const* knownBCs,
    int nKnownBCs,
    double* values, int* bits)
{
  gmi_model* gm = m->getModel();
  gmi_ent* ge = (gmi_ent*)m->toModel(e);
  bool appliedAny = false;
  for (int i = 0; i < nKnownBCs; ++i) {
    double* bcvalues = checkForBC(gm, ge, appliedBCs, knownBCs[i]);
    if (!bcvalues)
      continue;
    knownBCs[i].apply(values, bits, knownBCs[i], bcvalues);
    appliedAny = true;
  }
  return appliedAny;
}

bool applyNaturalBCs(apf::Mesh* m, apf::MeshEntity* f,
    BCs& appliedBCs, double* values, int* bits)
{
  return applyBCs(m, f, appliedBCs, naturalBCs,
      sizeof(naturalBCs) / sizeof(KnownBC), values, bits);
}

bool applyEssentialBCs(apf::Mesh* m, apf::MeshEntity* v,
    BCs& appliedBCs, double* values, int* bits)
{
  return applyBCs(m, v, appliedBCs, essentialBCs,
      sizeof(essentialBCs) / sizeof(KnownBC), values, bits);
}

bool applySolutionBCs(apf::Mesh* m, apf::MeshEntity* v,
    BCs& appliedBCs, double* values)
{
  return applyBCs(m, v, appliedBCs, solutionBCs,
      sizeof(solutionBCs) / sizeof(KnownBC), values, 0);
}

void getBCFaces(apf::Mesh* m, BCs& bcs, std::set<apf::ModelEntity*>& faces)
{
  APF_ITERATE(BCs::Map, bcs.fields, it)
    APF_ITERATE(FieldBCs::Set, it->second.bcs, it2)
      if (it2->dim == 2)
        faces.insert(m->findModelEntity(it2->dim, it2->tag));
}

}
