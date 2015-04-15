#ifndef ENTITY_H
#define ENTITY_H

#include "mesh.h"

class Entity
{
public:

    Entity();
    Entity(string);
    ~Entity();
    void setObject(Mesh externalMesh) { object.SetGeom(externalMesh.GetGeom()); object.SetTopo(externalMesh.GetTopo()); }
    Mesh getObject() { return object; }

private:

    Mesh object;
    /* Attributs réseau
     * Attributs IA
     */
};

#endif // ENTITY_H
