#include "apfArrayData.h"
#include "apfNumbering.h"
#include "apfTagData.h"

namespace apf {

template <class T>
class ArrayDataOf : public FieldDataOf<T>
{
  public:
    virtual void init(FieldBase* f)
    {
      /* this class inherits a variable (field),
         lets initialize it */
      this->field = f;
      /* this has to set up the array */
      FieldShape* s = f->getShape();
      const char* name = s->getName();
      Numbering* n = f->getMesh()->findNumbering(name);
// OLD: 
//  - destroying/re-creating the local numbering of default field shape causes 
//    a dangling pointer as the deleted local numbering is still kept in other array-type fields
//      if (n) apf::destroyNumbering(n);
//      num_var = numberOverlapNodes(f->getMesh(),name,s);
// NEW: 
//   - keep the local numbering for default field shape unless the mesh is modified
//      (e.g. migration, ghosting, load balancing, adaptation)
//   - after mesh is modified and before freeze the fields, remove all local numberings 
//     by calling "while (m->countNumberings()) destroyNumbering(m->getNumbering(0));"
      if (!n) n = numberOverlapNodes(f->getMesh(),name,s);   
      num_var = n;      
      arraySize = f->countComponents()*countNodes(num_var);
      dataArray = new T[arraySize];
    }
    virtual ~ArrayDataOf()
    {
      /* this has to destroy the array */
      delete [] dataArray;
    }
    virtual bool hasEntity(MeshEntity*)
    {
      /* this should get a bit more complex:
         return true if the field has nodes on (e) */
      return true;
    }
    virtual void removeEntity(MeshEntity*)
    {
      /* this will remain an empty function...
         I don't think  we want to remove entities from frozen fields */
      fail("removeEntity called on frozen field data");
    }
    virtual void get(MeshEntity* e, T* data)
    {
      /* this retrieves all the data associated with (e) */
      int first_node_index = getNumber(this->num_var,e,0,0);
      int num_nodes = this->field->countNodesOn(e);
      int num_components = this->field->countComponents();
      int start = first_node_index*num_components;
      for (int i=0; i<num_nodes*num_components; i++) {
         data[i] = this->dataArray[start+i];
      }
    }
    virtual void set(MeshEntity* e, T const* data)
    {
      /* this stores all the data associated with (e) */
      int first_node_index = getNumber(this->num_var,e,0,0);
      int num_nodes = this->field->countNodesOn(e);
      int num_components = this->field->countComponents();
      int start = first_node_index*num_components;
      for (int i=0; i<num_nodes*num_components; i++) {
          this->dataArray[start+i] = data[i];
      }
    }

    virtual bool isFrozen() {
      return true;
    }

    T* getDataArray() {
      return this->dataArray;
    }
    virtual FieldData* clone() {
      //FieldData* newData = new TagDataOf<double>();
      FieldData* newData = new ArrayDataOf<T>();
      newData->init(this->field);
      copyFieldData(static_cast<FieldDataOf<T>*>(newData),
                    static_cast<FieldDataOf<T>*>(this->field->getData()));
      return newData;
    }

  private:
    /* data variables go here */
    Numbering* num_var; 
    int arraySize;
    T* dataArray;
};

template <class T>
void freezeFieldData(FieldBase* field)
{
  /* make a new data store of array type */
  ArrayDataOf<T>* newData = new ArrayDataOf<T>();
  /* call the init function to setup storage */
  newData->init(field);
  /* get the old data store */
  FieldDataOf<T>* oldData = static_cast<FieldDataOf<T>*>(field->getData());
  /* call the set function to fill with values */
  copyFieldData<T>(oldData,newData);
  /* replace the old data store with this one */
  field->changeData(newData);
}

template <class T>
void unfreezeFieldData(FieldBase* field) {
  // make a new data store of tag type
  TagDataOf<T>* newData = new TagDataOf<T>();
  // call init function to setup storage
  newData->init(field);
  // get the old data store
  FieldDataOf<T>* oldData = static_cast<FieldDataOf<T>*>(field->getData());
  // call set function to fill with values
  copyFieldData<T>(oldData,newData);
  // replace old data store with this one
  field->changeData(newData);
}

/* instantiate here */
template void freezeFieldData<int>(FieldBase* field);
template void freezeFieldData<double>(FieldBase* field);
template void unfreezeFieldData<int>(FieldBase* field);
template void unfreezeFieldData<double>(FieldBase* field);

double* getArrayData(Field* f) {
  if (!isFrozen(f)) {
    return 0;
  } else {
    FieldDataOf<double>* p = f->getData();
    ArrayDataOf<double>* a = static_cast<ArrayDataOf<double>* > (p);
    return a->getDataArray();
  }
}

}
