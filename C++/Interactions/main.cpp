//#include "mainwindow.h"
#include <QApplication>
#include <QPushButton>
#include "entity.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QWidget w;
    w.setFixedSize(300,150);
    /*QPushButton start("Commencer le test",&w);
    start.move(50,50);
    w.show();*/

    /*string name = "/home/thomas/Documents/Image/Interactions/test.txt";
    ifstream File;
    File.open(name.c_str(),ios::in);
    string Line;
    char c;
    if(!File.is_open())
        cout<<"Opening failed"<<endl;
    cout<<"Begin parsing"<<endl;
    while(File){
        //getline(File,Line);
        c=(char)File.get();
        cout<<c;};
    File.close();*/


    Entity entityTest;
    Mesh testMesh;
    bool b = testMesh.LoadObj("/home/thomas/Documents/Image/Interactions/Meshes/3D Model Traffic Lights.obj");
    cout<<b<<endl;
    testMesh.SaveObj("/home/thomas/Documents/Image/Interactions/Meshes/TestSave.obj");
    //return a.exec();
    return 0;
}
