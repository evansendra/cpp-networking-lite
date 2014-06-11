/* sequence solution that's using select function to select an active connection.
 * Client handling switches in a time.
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

/* handling of one client. Get a part of data from client and process it.
 */

bool serveClient ( int dataFd )
 {
   char buffer[200];

   int l = read ( dataFd, buffer, sizeof ( buffer ));
   // zero size -> closes client connection
   if ( ! l ) return false;

   // converts small to big and vice versa
   for ( int i = 0; i < l; i ++ )
    if ( isalpha ( buffer[i] ) )
     buffer[i] ^= 0x20;
   write ( dataFd, buffer, l );
   // connection was not terminated, expecting another data
   return true;
 }

int main ( void )
 {
   int fd = openSrvSocket ( "ip6-localhost", 12345 );
   if ( fd < 0 ) return 1;

   // list of sockets related to the server
   // at the beginning there is just one socket for connection requests
   vector<int> sockets;
   sockets . push_back ( fd );

   while ( 1 )
    {
      fd_set rd;
      int max;

      // fills the set of sockets that are interesting
      FD_ZERO ( &rd );
      for ( vector<int>::size_type i = 0; i < sockets . size (); i ++ )
       {
         FD_SET ( sockets[i], &rd );
         if ( i == 0 || sockets[i] > max ) max = sockets[i];
       }

      // waiting for an empty socket (without data)
      // if there is on data -> process will sleep and will not need a CPU time.
      int res = select ( max + 1, &rd, NULL, NULL, NULL );

      if ( res > 0 )
       {

         // data on the socket sockets[0] -> new connected client
         if ( FD_ISSET ( sockets[0], &rd ) )
          {
            struct sockaddr remote;
            socklen_t remoteLen = sizeof ( remote );
            // create connection to this client, accept returns a new socket that
            // we use for sending data in this connection
            int dataFd = accept ( fd, &remote, &remoteLen );
            sockets . push_back ( dataFd );
            printf ( "New connection\n" );
          }
         for ( vector<int>::size_type i = 1; i < sockets . size (); i ++ )
          if ( FD_ISSET ( sockets[i], &rd ) )
           {
             // data on one of the sockets -> handling data
             if ( ! serveClient ( sockets[i] ) )
              {
                // if the connections is over -> deallocation...
                printf ( "Close connection\n" );
                close ( sockets[i] );
                sockets . erase ( sockets . begin () + i );
                i --;
              }
           }
       }
    }
   // servers are still running, the program should never get here.
   close ( fd );
   return 0;
 }
