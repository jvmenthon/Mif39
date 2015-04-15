#include "mesh.h"

Mesh::Mesh() {}

Mesh::Mesh(QVector<QVector<float> > geo, QVector<QVector<int> > topology):geometry(geo), topo(topology)
{
    //ctor
}

Mesh::Mesh(QVector<QVector<float> > geo, QVector<QVector<int> > topology,QVector<QVector<float> > norm,QVector<QVector<float> > text ):geometry(geo), topo(topology),
    normals(norm),textures(text)
{
    //ctor
}

Mesh::~Mesh()
{
    //dtor
}

Mesh Mesh::Merge(Mesh B) //Merge de B sur A
{
    QVector<QVector<float> > vertex = geometry;
    QVector<QVector<int> > topology = topo;
    int vertexSize = vertex.size();
    int topoSize = topology.size();
    for(int i=0;i<B.GetGeom().size();i++) { vertex.push_back(B.GetGeom()[i]); }
    for(int i=0;i<B.GetTopo().size();i++) { topology.push_back(B.GetTopo()[i]); }
    for(int i=topoSize;i<topology.size();i++)
    {
        topology[i][0] += vertexSize;
    }

    return Mesh(vertex,topology);
}

void Mesh::SaveObj(std::string name)
{
    ofstream myfile;
    myfile.open (name.c_str());
    for(int i=0;i<geometry.size();i++)
    {
        myfile << "v " << geometry[i][0] << " " << geometry[i][1]  << " " << geometry[i][2] << "\n";
    }
    for(int i=0;i<normals.size();i++)
    {
        myfile << "vn " << normals[i][0] << " " << normals[i][1]  << " " << normals[i][2] << "\n";
    }
    for(int i=0;i<textures.size();i++)
    {
        myfile << "vt " << textures[i][0] << " " << textures[i][1]  << "\n";
    }

    myfile << "f ";
    for(int i=0;i<topo.size();i++)
    {
        if(i%3 == 2)
        {
            if(i == topo.size()-1)
            {
                myfile << topo[i][0]<<"/"<<topo[i][1]<<"/"<<topo[i][2]<< "\n" ;
            }
            else
            {
                myfile << topo[i][0]<<"/"<<topo[i][1]<<"/"<<topo[i][2] << "\n" << "f " ;
            }

        }
        else {myfile << topo[i][0]<<"/"<<topo[i][1]<<"/"<<topo[i][2] << " " ;}
    }

    myfile.close();
}

void Mesh::LoadObj(std::string name)
{



    QVector<float> TempTopology;
    std::ifstream File(name.c_str());
    string Line;
    string Name;

    while(std::getline(File, Line)){

      if(Line == "" || Line[0] == '#')// Skip everything and continue with the next line
        continue;

      std::istringstream LineStream(Line);
      LineStream >> Name;

      if(Name == "v"){// Vertex
        float Vertex[3];
        QVector<float> TempVertices;
        sscanf(Line.c_str(), "%*s %f %f %f", &Vertex[0], &Vertex[1], &Vertex[2]);
        TempVertices.push_back(Vertex[0]);
        TempVertices.push_back(Vertex[1]);
        TempVertices.push_back(Vertex[2]);
        geometry.push_back(TempVertices);
      }
      if(Name == "vn"){// Normales
        float Vertex[3];
        QVector<float> TempNormals;
        sscanf(Line.c_str(), "%*s %f %f %f", &Vertex[0], &Vertex[1], &Vertex[2]);
        TempNormals.push_back(Vertex[0]);
        TempNormals.push_back(Vertex[1]);
        TempNormals.push_back(Vertex[2]);
        normals.push_back(TempNormals);
      }
      if(Name == "vt"){// Textures
        float Vertex[2];
        QVector<float> TempTextures;
        sscanf(Line.c_str(), "%*s %f %f", &Vertex[0], &Vertex[1]);
        TempTextures.push_back(Vertex[0]);
        TempTextures.push_back(Vertex[1]);
        TempTextures.push_back(Vertex[2]);
        textures.push_back(TempTextures);
      }
      if(Name == "f"){// Faces
        float Vertex[3][3];
        QVector<int> TempTopology;
        sscanf(Line.c_str(), "%*s %f/%f/%f %f/%f/%f %f/%f/%f",
               &Vertex[0][0], &Vertex[0][1], &Vertex[0][2],
                &Vertex[1][0], &Vertex[1][1], &Vertex[1][2],
                &Vertex[2][0], &Vertex[2][1], &Vertex[2][2]);
        TempTopology.push_back(Vertex[0][0]);TempTopology.push_back(Vertex[0][1]);TempTopology.push_back(Vertex[0][2]);
        topo.push_back(TempTopology);
        TempTopology.clear();
        TempTopology.push_back(Vertex[1][0]);TempTopology.push_back(Vertex[1][1]);TempTopology.push_back(Vertex[1][2]);
        topo.push_back(TempTopology);
        TempTopology.clear();
        TempTopology.push_back(Vertex[2][0]);TempTopology.push_back(Vertex[2][1]);TempTopology.push_back(Vertex[2][2]);
        topo.push_back(TempTopology);
      }
    };
}
