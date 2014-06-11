/* sequential version of server side. It handles just one client, the others must wait
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

   /* The address where server listens. It uses 'name' to determine the address type (IPv4/6) and
    * its binary value.
    */
   snprintf ( portStr, sizeof ( portStr ), "%d", port );
   if ( getaddrinfo ( name, portStr, NULL, &ai ) )
    {
      printf ( "addrinfo\n" );
      return -1;
    }
   /* Open socket, type of socket (family) by getaddrinfo return value,
    * stream = TCP
    */
   int fd = socket ( ai -> ai_family, SOCK_STREAM, 0 );
   if ( fd == -1 )
    {
      freeaddrinfo ( ai );
      printf ( "socket\n" );
      return -1;
    }

   /* connect socket with selected network interface
    */
   if ( bind ( fd, ai -> ai_addr, ai -> ai_addrlen ) == -1 )
    {
      close ( fd );
      freeaddrinfo ( ai );
      printf ( "bind\n" );
      return -1;
    }
   freeaddrinfo ( ai );
    /* Switch the socket to listening mode (it will not handle data communication, it will
    * just handle requests for connection) . 10 is max queue size waiting requests for connection
   */
   if ( listen ( fd, 10 ) == -1 )
    {
      close ( fd );
      printf ( "listen\n" );
      return -1;
    }
   return fd;
 }

/* handling of one client (all of his messages)
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
      // zero size -> closes client connection
      if ( ! l ) break;

       // converts small to big and vice versa
      for ( int i = 0; i < l; i ++ )
       if ( isalpha ( buffer[i] ) )
        buffer[i] ^= 0x20;
      write ( dataFd, buffer, l );
      // connection was not terminated, expecting another data
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
   // servers are still running, the program should never get here.
   close ( fd );
   return 0;
 }
