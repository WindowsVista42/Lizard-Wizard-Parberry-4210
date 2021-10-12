#include "Defines.h"
#include "Abort.h"
#include <btBulletCollisionCommon.h>

Vector3 convert(btVector3 input);
btVector3 convert(Vector3 input);
std::string XMLFindItem(tinyxml2::XMLElement* xml, const char* group_tag, const char* item_tag, const char* name);
