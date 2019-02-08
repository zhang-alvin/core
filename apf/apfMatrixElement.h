/*
 * Copyright 2011 Scientific Computation Research Center
 *
 * This work is open source software, licensed under the terms of the
 * BSD license as described in the LICENSE file in the top-level directory.
 */

#ifndef APFMATRIXELEMENT_H
#define APFMATRIXELEMENT_H

#include "apfElementOf.h"

namespace apf {

class MatrixField;

class MatrixElement : public ElementOf<Matrix3x3>
{
  public:
    MatrixElement(MatrixField* f, VectorElement* e);
    void grad(Vector3 const& xi, Vector<27>& g);
    virtual ~MatrixElement();
};

}//namespace apf

#endif
