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
   /* Zadost o spojeni se serverem (ted se teprve zacne komunikovat).
    * vysledkem je bud otevrene datove spojeni nebo chyba.
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

   // posilame zadana data, zadany pocet krat
   for ( int i = 0; i < cnt; i ++ )
    {
      char buffer[200];

      snprintf ( buffer, sizeof ( buffer ), "%d: %s\n", i, argv[3] );
      write ( fd, buffer, strlen ( buffer ) );

      int l = read ( fd, buffer, sizeof ( buffer ) );
      write ( STDOUT_FILENO, buffer, l );
      // prodleva mezi zaslanymi daty
      usleep ( delay * 1000 );
    }
   // uzavreni spojeni klientem. Server zjisti uzavreni spojeni a
   // uvolni prostredky na sve strane.
   close ( fd );
   return 0;
 }