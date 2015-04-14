#include "QImageLoader/qimageloader.hpp"
#include "Wavefront/wavefront.hpp"
#include "AssetInterfaces/resourceholder.hpp"

#include "TcpNetworking/simpletcpstartpoint.hpp"


extern void __attach(void);
extern void __attachInterfaces(void);
extern void __attachGenerics(void);
extern void __attachAssets(void);
extern void __attachQImage(void);
extern void __attachWavefront(void);


/*
 * SERVEUR
 * Crée le monde
 * Accepte les connexions client et crée l'objet client dans le monde
 * Broadcast a tous les clients des informations de MAJ
 * Fournit au client des infos de modélisation 3D pour ceux qu'il ne connaît pas
 *
 */
int main ( int argc, char** argv ) {
    QUuid fake;
    SimpleTcpStartPoint::Options options;
    options.connectionPort = 3000;
    options.maximumConnectedClients = 1;
    SimpleTcpStartPoint server ( options );
    server.start();
    QUuid client;

    __attachWavefront();

    //Chargement de l'obj représentant le monde
    FileDescriptor file ("../Resources/UnformattedWorlds/StreetEnv/StreetEnv.obj");
    ResourceHolder::Load(file);
    //ResourceHolder::Usage();

    //Parcours du fichier?
    foreach ( QUuid id, ResourceHolder::AllKeys() ) {
        SharedResourcePtr res = ResourceHolder::GetByUUID(id);
        res->Usage();
        ByteBuffer buffer = ResourceHolder::ToBuffer( res );
        /*unsigned long long index = 0;
        SharedResourcePtr ptr = ResourceHolder::FromBuffer(buffer,index);
        ptr->Usage();*/
    }


    //Connexion client
    while ( client == fake ) {
        std::cout << "Waiting for client ..." << std::endl;
        client = server.listen();
        std::cout << "Cient connected !" << std::endl;
    }

    while ( true ) {
        /* Ici il va probablement falloir ajouter l'obj du client connecté au monde en lisant la requete TCP envoyée
         * par le client et en recréant le mesh en remplissant la structure de données */

        ByteBuffer message;
        bool res = server.receive(client,message);
        std::cout << "Data : " << message.getData() << "bytes" << std::endl;
        /*  server.send(client,message);
            std::cout << "Sent : " << message.getLength() << " bytes" << std::endl;
        */
        if(res) break;

    }

    /* Ici il va falloir broadcast à tous les clients les infos */
    server.stop ();
    return 0;
}
