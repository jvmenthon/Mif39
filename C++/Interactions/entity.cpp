#include "entity.h"

Entity::Entity(){}

Entity::Entity(string objName)
{
    object.LoadObj(objName);
}
