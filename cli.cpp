#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
using namespace std;

int openCliSocket ( const char * name, int port )
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
   /* Request to a server connection (now the communication starts)
    * The result is an opened connection or error.
    */
   if ( connect ( fd, ai -> ai_addr, ai -> ai_addrlen ) == - 1 )
    {
      close ( fd );
      freeaddrinfo ( ai );
      printf ( "connect\n" );
      return -1;
    }
   freeaddrinfo ( ai );
   return fd;
 }

int main ( int argc, char * argv [] )
 {
   int cnt, delay;

   if ( argc != 4
        || sscanf ( argv[1], "%d", &cnt ) != 1
        || sscanf ( argv[2], "%d", &delay ) != 1 )
    {
      printf ( "<cnt> <delay> <str>\n" );
      return 1;
    }

   int fd = openCliSocket ( "ip6-localhost", 12345 );

   // we send selected data, selected amount times
   for ( int i = 0; i < cnt; i ++ )
    {
      char buffer[200];

      snprintf ( buffer, sizeof ( buffer ), "%d: %s\n", i, argv[3] );
      write ( fd, buffer, strlen ( buffer ) );

      int l = read ( fd, buffer, sizeof ( buffer ) );
      write ( STDOUT_FILENO, buffer, l );
      // delay between data sending
      usleep ( delay * 1000 );
    }
   // close the client connection, server deallocates its memory and other stuff
   close ( fd );
   return 0;
 }