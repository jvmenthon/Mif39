
#include "QImageLoader/qimageloader.hpp"
#include "Wavefront/wavefront.hpp"

#include "TcpNetworking/simpletcpendpoint.hpp"

extern void __attach(void);
extern void __attachInterfaces(void);
extern void __attachGenerics(void);
extern void __attachAssets(void);
extern void __attachQImage(void);
extern void __attachWavefront(void);

/*
 * CLIENT
 * Se connecte au serveur avec un obj
 * Est averti en cas de changements dans le monde
 * Ne voit qu'a une distance maximale, il ne connaît pas toute la carte, elle est trop grande pour etre retenue côté client
 *
 *
 */
int main ( int argc, char** argv ) {
    /*
    __attach();
    __attachInterfaces();
    __attachGenerics();
    __attachAssets();*/
    __attachQImage();
    __attachWavefront();
    FileDescriptor file ( argv[1]);
    SharedResourceList ress = ResourceHolder::Load(file);
    SharedResourcePtr ptr = ress [0];
    ByteBuffer message = ResourceHolder::ToBuffer(ptr);

    SimpleTcpEndPoint::Options options;
    options.serverIP = "127.0.0.1";
    options.connectionPort = 3000;
    SimpleTcpEndPoint client ( options );
    if ( client.open() == false ) exit ( -1 );
   //while ( true ) {

        /* Ici il faut probablement envoyer l'obj client au serveur dans une requete TCP */
        client.send(message); std::cout << "Message : " << message.getData() << " bytes" << std::endl;
        //client.receive(message); std::cout << "Recv : " << message.getLength() << " bytes" << std::endl;

    //}
    client.close ();
    return 0;
}
