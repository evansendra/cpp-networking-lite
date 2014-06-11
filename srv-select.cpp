/* sekvencni reseni, ketre pomoci funkce select vybira aktivni spojeni. Obsluha jenotlivych
 * klientu se (dle pozadavku) strida v case.
 */
#include <cstdio>
#include <cctype>
#include <cstdlib>
#include <vector>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
using namespace std;

int openSrvSocket ( const char * name, int port )
 {
   struct addrinfo * ai;
   char portStr[10];

   /* Adresa, kde server posloucha. Podle name se urci typ adresy
    * (IPv4/6) a jeji binarni podoba
    */
   snprintf ( portStr, sizeof ( portStr ), "%d", port );
   if ( getaddrinfo ( name, portStr, NULL, &ai ) )
    {
      printf ( "addrinfo\n" );
      return -1;
    }
   /* Otevreni soketu, typ soketu (family) podle navratove hodnoty getaddrinfo,
    * stream = TCP
    */
   int fd = socket ( ai -> ai_family, SOCK_STREAM, 0 );
   if ( fd == -1 )
    {
      freeaddrinfo ( ai );
      printf ( "socket\n" );
      return -1;
    }

   /* napojeni soketu na zadane sitove rozhrani
    */
   if ( bind ( fd, ai -> ai_addr, ai -> ai_addrlen ) == -1 )
    {
      close ( fd );
      freeaddrinfo ( ai );
      printf ( "bind\n" );
      return -1;
    }
   freeaddrinfo ( ai );
   /* prepnuti soketu na rezim naslouchani (tedy tento soket nebude vyrizovat
    * datovou komunikaci, budou po nem pouze chodit pozadavky na pripojeni.
    * 10 je max velikost fronty cekajicich pozadavku na pripojeni.
    */
   if ( listen ( fd, 10 ) == -1 )
    {
      close ( fd );
      printf ( "listen\n" );
      return -1;
    }
   return fd;
 }

/* obsluha jednoho klienta. Vyzvedne (cast) dat zaslanou
 * klientem a zpracuje ji.
 */

bool serveClient ( int dataFd )
 {
   char buffer[200];

   int l = read ( dataFd, buffer, sizeof ( buffer ));
   // nulova delka -> uzavreni spojeni klientem
   if ( ! l ) return false;

   // prevod mala -> velka a naopak
   for ( int i = 0; i < l; i ++ )
    if ( isalpha ( buffer[i] ) )
     buffer[i] ^= 0x20;
   write ( dataFd, buffer, l );
   // spojeni nebylo ukonceno, jeste mohou prijit dalsi data.
   return true;
 }

int main ( void )
 {
   int fd = openSrvSocket ( "ip6-localhost", 12345 );
   if ( fd < 0 ) return 1;

   // seznam soketu, ktere se tykaji serveru
   // na pocatku pouze soket pro prijem zadosti o spojeni.
   vector<int> sockets;
   sockets . push_back ( fd );

   while ( 1 )
    {
      fd_set rd;
      int max;

      // vyplnime mnozinu soketu, ktere nas zajimaji
      FD_ZERO ( &rd );
      for ( vector<int>::size_type i = 0; i < sockets . size (); i ++ )
       {
         FD_SET ( sockets[i], &rd );
         if ( i == 0 || sockets[i] > max ) max = sockets[i];
       }

      // cekame, dokud na nejakem ze soketu nejsou k dispozici data
      // pokud nejsou -> proces bude uspan a nebude zadat o procesorovy cas.
      int res = select ( max + 1, &rd, NULL, NULL, NULL );

      if ( res > 0 )
       {

         // data na soketu sockets[0] -> nove pripojeny klient
         if ( FD_ISSET ( sockets[0], &rd ) )
          {
            struct sockaddr remote;
            socklen_t remoteLen = sizeof ( remote );
            // vytvorime spojeni k tomuto klientu, accept vraci novy soket, kterym
            // zasilame data pouze v tomto spojeni
            int dataFd = accept ( fd, &remote, &remoteLen );
            sockets . push_back ( dataFd );
            printf ( "New connection\n" );
          }
         for ( vector<int>::size_type i = 1; i < sockets . size (); i ++ )
          if ( FD_ISSET ( sockets[i], &rd ) )
           {
             // data na nekterem z datovych soketu -> obslouzit
             if ( ! serveClient ( sockets[i] ) )
              {
                // pokud spojeni skoncilo -> uvolnit prostredky
                printf ( "Close connection\n" );
                close ( sockets[i] );
                sockets . erase ( sockets . begin () + i );
                i --;
              }
           }
       }
    }
   // servery bezi stale, sem se rizeni nikdy nedostane.
   close ( fd );
   return 0;
 }
