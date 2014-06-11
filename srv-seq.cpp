/* sekvencni varianta serverove casti. Obslouzi jednoho klienta, ostatni zadajici klienti musi cekat
 *
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

/* obsluha jednoho klienta (vsechny jeho zpravy)
 */

void serveClient ( int listenFd )
 {
   char buffer[200];
   struct sockaddr remote;
   socklen_t remoteLen = sizeof ( remote );
   int dataFd = accept ( listenFd, &remote, &remoteLen );
   printf ( "New connection\n" );

   while ( 1 )
    {
      int l = read ( dataFd, buffer, sizeof ( buffer ));
      // nulova delka -> uzavreni spojeni klientem
      if ( ! l ) break;

      // prevod mala -> velka a naopak
      for ( int i = 0; i < l; i ++ )
       if ( isalpha ( buffer[i] ) )
        buffer[i] ^= 0x20;
      write ( dataFd, buffer, l );
      // spojeni nebylo ukonceno, jeste mohou prijit dalsi data.
    }
   close ( dataFd );
   printf ( "Close connection\n" );
 }

int main ( void )
 {
   int fd = openSrvSocket ( "ip6-localhost", 12345 );
   if ( fd < 0 ) return 1;
   while ( 1 )
    {
      serveClient ( fd );
    }
   // servery bezi stale, sem se rizeni nikdy nedostane.
   close ( fd );
   return 0;
 }
