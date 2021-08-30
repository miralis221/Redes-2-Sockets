/*
** Fichero: difusor.c
** Autores:
** Erick JosE Mercado Hernandez DNI 43840208T
** Javier MerchAn Benito DNI 70923644Q
** Usuario i0923644
*/

#include <sys/types.h>   
#include <sys/socket.h> 
#include <sys/time.h>   
#include <time.h>            
#include <netinet/in.h>  
#include <arpa/inet.h> 
#include <errno.h>
#include <fcntl.h>              
#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>    
#include <sys/uio.h> 
#include <unistd.h>
#include <sys/wait.h>
#include <net/if.h>

#define MAXLINE 4096
#define MENSAJE "Nines, nosotros queriamos poner las capturas de wireshark"
#define MULTICAST "FF15::33"
#define INTERFAZ "eth0"
#define PUERTO 4343
#define SALTOS 10
#define INTERVALO 15

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
int main (int argc, char **argv){

    /*
    ** Declaración variables socket
    */
    int s;
    struct sockaddr_in6 receptoraddr;

    /*
    ** Declaración variables
    */
    char buff[MAXLINE]=MENSAJE;
    char ipmulticast[255]=MULTICAST;
    char interfaz[255]=INTERFAZ;
    int puerto=PUERTO;
    int saltos=SALTOS;
    int intervalo=INTERVALO;
    int len=0;
    int interfazNumero;

    /*
    ** Decalaracion señal fin
    */
    struct sigaction fin;
    fin.sa_handler = (void *) handler;
    fin.sa_flags = 0;
    if ( sigaction(SIGINT, &fin, (struct sigaction *) 0) == -1) {
        perror("sigaction(SIGINT)");
        fprintf(stderr,"%s: Error al registrar SIGINT\n", argv[0]);
        exit(-1);
    }

    /* 
    ** Comprobación si han introducido parametros, si esto no es así, se mantienen
    ** los valores por defecto declarados en los define. Error cuando hay mas de 7 args
    */
    if (argc==7){
        strcpy(buff,argv[1]);
        strcpy(ipmulticast,argv[2]);
        strcpy(interfaz,argv[3]);
        puerto=atoi(argv[4]);
        saltos=atoi(argv[5]);
        intervalo=atoi(argv[6]);
    }else{
        if(argc>7){
            perror("Argumentos: demasiados argumentos introducidos\n");
            exit (-1);
        }
    }

    /*
    ** Creación socket
    */
    if ( (s = socket(AF_INET6, SOCK_DGRAM, 0)) < 0){
        perror("Socket: error al crear el socket\n");
        exit (-1);
    }

    /*
    ** Inicialización socket
    */
    receptoraddr.sin6_family = AF_INET6;
    receptoraddr.sin6_port = htons(puerto);

    /*
    ** Longitud del socket
    */
    len = sizeof(struct sockaddr_in6);

    /*
    ** Configuracion para que se pueda recibir una copia de los datagramas
    */
   /* uint8_t loop;
    if(setsockopt(s, IPPROTO_IPV6, IPV6_MULTICAST_LOOP, &loop, sizeof(loop))==-1){							
        perror("MULTICAST_LOOP: no fue posible recibir copia de los datagramas\n");
        exit(-1);
    }*/

    /*
    ** Configuracion de la interfaz
    */
    interfazNumero=atoi(interfaz);
    if(setsockopt(s, IPPROTO_IPV6, IPV6_MULTICAST_IF, (char*) &interfazNumero, sizeof(interfazNumero))==-1){							
        perror("MULTICAST_IF: no fue posible configurar la interfaz por la que se enviaran los datagramas\n");
        exit(-1);
    }

    /*
    ** Configuracion del numero de saltos  
    */
    if(setsockopt(s, IPPROTO_IPV6, IPV6_MULTICAST_HOPS, &saltos, sizeof(saltos))==-1){							
        perror("MULTICAST_HOPS: no fue posible configurar el numero de saltos\n");
        exit(-1);
    }

    /*
    ** Configuracion de la ip multicast 
    */
    if (inet_pton(AF_INET6, ipmulticast, &receptoraddr.sin6_addr) <= 0) {
		perror("inet_pton: error al convertir la direccion\n");
		exit (-1);
	}

    /*
    **  Envio de los datagramas
    */
    for(;;){
        if (sendto(s, buff, strlen(buff), 0, (struct sockaddr *)&receptoraddr, len) == -1) {
            perror ("Error al enviar la hora\n");
            exit (-1);
	    }

        sleep(intervalo); /*intervalo de envio de mensaje*/

    }
    
}