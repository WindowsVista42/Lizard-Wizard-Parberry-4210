/// \file Descriptors.cpp
/// \brief Code for the descriptors.
/// 
/// This consists of some constructors and destructors for some of the 
/// descriptors that make them easier to use elsewhere.

#include "Descriptors.h"

///////////////////////////////////////////////////////////////////////////////
// CMeshDesc functions.

/// Constructor.
/// \param t Mesh type.

CMeshDesc::CMeshDesc(const eMesh t):
  m_eMeshType(t){
} //constructor

/// Destructor.

CMeshDesc::~CMeshDesc(){
  delete m_pEffect;
} // destructor

///////////////////////////////////////////////////////////////////////////////
// CContactDesc functions.

/// Default constructor.

CContactDesc::CContactDesc(){
} //constructor

/// Constructor.
/// \param n Number of contacts.
/// \param impulse Impulse magnitude. Defaults to zero.

CContactDesc::CContactDesc(const UINT n, const float impulse):
  m_nNumContacts(n), m_fImpulse(impulse){
} //constructor