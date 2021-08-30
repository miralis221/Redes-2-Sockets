/*
** Fichero: suscriptor.c
** Autores:
** Erick JosE Mercado Hernandez 
** Javier MerchAn Benito 
** Usuario 
*/

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <net/if.h>

#define MAXLINE 4096        
#define PUERTO 4343
#define INTERFAZ "eth0"
#define MULTICAST "ff15::33"

/*
** Manejadora para la finalización ordenada del programa
*/
void handler(){
     printf("\nAlarma recibida\n");
     exit(0);
}

/*
** MAIN
*/
int main(int argc, char **argv){

	/*
    ** Declaración variables socket
    */
    int s;
	socklen_t len;
	struct sockaddr_in6	servaddr, recibaddr;
	extern const struct in6_addr in6addr_any;
	char equipo [INET6_ADDRSTRLEN];
	struct ipv6_mreq ipv6mreq;

	/*
    ** Declaración variables
    */
	char buff[MAXLINE];
	char ipmulticast[255]=MULTICAST;
	char interfaz[255]=INTERFAZ;
	int puerto=PUERTO;
	int recive;

	/*
    ** Decalaracion señal fin
    */
	struct sigaction fin;
	fin.sa_handler = (void *)handler;
	fin.sa_flags = 0;
	if ( sigaction(SIGINT, &fin, (struct sigaction *) 0) == -1) {
	        perror("sigaction(SIGINT)");
	        fprintf(stderr,"%s: Error al registrar SIGINT\n", argv[0]);
	        exit(-1);
	}

	/* 
    ** Comprobación si han introducido parametros, si esto no es así, se mantienen
    ** los valores por defecto declarados en los define. Error cuando hay mas de 4 args
    */
	if (argc == 4){
		strcpy(ipmulticast, argv[1]);
		strcpy(interfaz, argv[2]);
		puerto = atoi(argv[3]); 
	}
	else{
		if(argc>4){
            perror("Argumentos: demasiados argumentos introducidos\n");
            exit (-1);
        }
	}

	/*
    ** Creación socket
    */
	if ((s = socket(AF_INET6,SOCK_DGRAM,0)) <0){
			perror("Error al crear un socket");
			exit(1);
	}

	/*
    ** Inicialización socket
    */
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin6_family = AF_INET6;
	servaddr.sin6_addr   = in6addr_any;
	servaddr.sin6_port   = htons(puerto);	

	/*
    ** Longitud del socket
    */
	len = sizeof(struct sockaddr_in6);

	/*
	** Bind del socket
	*/
	if (bind(s, (struct sockaddr *)&servaddr, sizeof(servaddr))==-1){
		perror("Error al hacer bind");
		exit(-1);
	}

	/*
    ** Configuracion de la interfaz
    */
	ipv6mreq.ipv6mr_interface = if_nametoindex(interfaz);

	/*
    ** Configuracion de la ip multicast 
    */
	if (inet_pton(AF_INET6, ipmulticast, &ipv6mreq.ipv6mr_multiaddr) <=0){
		perror("inet_pton: error al convertir la direccion\n");
		exit(-1);
	}

	/*
    ** Configuracion añadir al grupo multicast
    */
	if (setsockopt(s, IPPROTO_IPV6, IPV6_ADD_MEMBERSHIP, &ipv6mreq, sizeof(ipv6mreq)) == -1){
		perror("IPV6_ADD_MEMBERSHIP: no fue posible annadir al grupo multicast\n");
		exit(-1);
	}

	/*
    **  Recepcion de los datagramas
    */
	for(;;){

		if ((recive = (recvfrom(s, buff, MAXLINE-1, 0, (struct sockaddr *)&recibaddr, &len)))==-1){
			perror("recvfrom: no se ha recibido mensaje");
			exit(-1);
		}
		buff[recive]='\0';
		if (inet_ntop (AF_INET6,&(recibaddr.sin6_addr),equipo,sizeof (equipo)) == NULL) {
            perror("inet_ntop: Error al mostrar la IP\n"); 
            exit(-1);                     
        }
		else {
			printf("Mensaje desde %s\n", equipo);
		}
		//fflush(stdout); 
		fprintf(stdout, "%s\n", buff);
	}

}
