#ifndef APFSIMDATAOF_H
#define APFSIMDATAOF_H

#include "apfFieldData.h"
#include <SimField.h>


namespace apf {

template <class T>
class SIMDataOf : public FieldDataOf<T>
{
public:
  SIMDataOf()
  {
    fd = 0;
  }
  SIMDataOf(pField fd_in)
  {
    fd = fd_in;
    pf = static_cast<pPolyField>(Field_def(fd));
  }
  ~SIMDataOf()
  {
    Field_release(fd);
    FieldDef_delete(pf);
  }
  virtual void init(FieldBase * f)
  {
    FieldData::field = f;
    mesh = f->getMesh();
    PCU_ALWAYS_ASSERT(f->getShape() == apf::getLagrange(f->getShape()->getOrder()));
    if (fd) {
      PCU_ALWAYS_ASSERT(f->getShape()->getOrder() == PolyField_entOrder(pf, 0));
      std::string name = f->getName();
      PCU_ALWAYS_ASSERT(name == Field_name(fd));
      PCU_ALWAYS_ASSERT(f->countComponents() == Field_numComp(fd));
      return;
    }
    pf = PolyField_new(f->getShape()->getOrder(), 0);
    fd = Field_new(static_cast<MeshSIM*>(mesh)->getMesh(),
		   f->countComponents(),
		   f->getName(),
		   "apf_field_data",
		   ShpLagrange,
		   1, // surfcont (always 1 in current simModSuite)
		   1, // num_time_derivatives to keep available
		   1, // num_section
		   pf);
    Field_apply(fd, mesh->getDimension(), NULL);
  }
  virtual bool hasEntity(MeshEntity *)
  {
    return true;
  }
  virtual void removeEntity(MeshEntity *)
  {}
  virtual void get(MeshEntity * e, T * data)
  {
    pEntity ent = reinterpret_cast<pEntity>(e);
    pDofGroup dof;
    int num_read = 0;
    for(int eindex = 0; (dof = Field_entDof(fd,ent,eindex)) ; eindex++)
    {
      int dofs_per_node = DofGroup_numComp(dof);
      DofGroup_values(dof,0,&data[num_read]);
      num_read += dofs_per_node;
    }
  }
  virtual void set(MeshEntity * e, T const * data)
  {
    pEntity ent = reinterpret_cast<pEntity>(e);
    pDofGroup dof;
    int num_wrote = 0;
    for(int eindex = 0; (dof = Field_entDof(fd,ent,eindex)); eindex++)
    {
      int dofs_per_node = DofGroup_numComp(dof);
      for(int ii = 0; ii < dofs_per_node; ii++)
        DofGroup_setValue(dof,ii,0,data[num_wrote+ii]);
      num_wrote += dofs_per_node;
    }
  }
  virtual bool isFrozen() {return false;}
  virtual FieldData * clone()
  {
    return NULL;
  }
  virtual void rename(const char* name)
  {
    Field_setName(fd, name);
  }
  pField getSimField() {return fd;}
private:
  Mesh * mesh;
  pField fd;
  pPolyField pf;
};

} // namespace apf

#endif
