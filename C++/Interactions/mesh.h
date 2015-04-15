#ifndef MESH_H
#define MESH_H

#include <QVector>
#include <fstream>
#include <QString>
#include <sstream>

using namespace std;

class Mesh
{
public:
    Mesh();
    Mesh(QVector<QVector<float> > geo, QVector<QVector<int> > topology);
    Mesh(QVector<QVector<float> > geo, QVector<QVector<int> > topology,QVector<QVector<float> > norm,QVector<QVector<float> > text );
    virtual ~Mesh();

    QVector<QVector<float> > GetGeom() { return geometry; }
    void SetGeom(QVector<QVector<float> > val) { geometry = val; }
    QVector<QVector<int> > GetTopo() { return topo; }
    void SetTopo(QVector<QVector<int> > val) { topo = val; }
    QVector<QVector<float> > GetNorms() { return normals; }
    void SetNorms(QVector<QVector<float> > val) { normals = val; }
    QVector<QVector<float> > GetText() { return textures; }
    void SetText(QVector<QVector<float> > val) { textures = val; }

    Mesh Merge(Mesh B); //Merge de B sur A
    void LoadObj(string);
    void SaveObj(string);

private:

    QVector<QVector<float> > geometry;
    QVector<QVector<int> > topo;
    QVector<QVector<float> > normals;
    QVector<QVector<float> > textures;
};

#endif // MESH_H
