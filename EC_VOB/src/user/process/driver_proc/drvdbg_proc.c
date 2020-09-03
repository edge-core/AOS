#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/ioctl.h>
#include <stddef.h>
#include <sys/errno.h>
//-----------------------------------------------------------

#define BOOL_T       unsigned char

#define I8_T         signed char          /* signed  8 bit integer  */
#define UI8_T        unsigned char        /* unsigned 8 bit integer  */

#define I16_T        short                /* signed  16 bit integer */
#define UI16_T       unsigned short       /* unsigned 16 bit integer */

#define I32_T	     long                 /* signed  32 bit integer */
#define UI32_T	     unsigned long        /* unsigned 32 bit integer */

/*  Return Value of SYSFUN  */
#define SYSFUN_OK                       0x00        /*  Successfully do the function            */
#define SYSFUN_RESULT_INVALID_ARG       0x03        /*  Invalid argument in calling function    */
#define SYSFUN_RESULT_ERROR             0xFFFFFFFF  /*  error with unknown reason */

/* FUNCTION NAME : SYSFUN_CreateIPCSocketServer
 * PURPOSE:
 *      Create a socket for inter-process communication -- for sending msg to Client.
 *
 * INPUT:
 *      ipc_socket_name  --  The name of the socket to be created. Null-terminated string.
 *      ipc_socket_id_p  --  The id of the created ipc socket will be put to this variable if sucess.
 *      non_blocking     --  1: Socket is nonblocking. If socket send buffer is full, we immediately drop the message. 
 *                           0: Socket is blocking
 * OUTPUT:
 *      ipc_socket_id_p  --  The id of the created ipc socket will be put to this variable if sucess.
 *
 * RETURN:
 *      SYSFUN_OK                   -- Sucess
 *      SYSFUN_RESULT_INVALID_ARG   -- Fail due to invalid argument
 *      SYSFUN_RESULT_ERROR         -- Fail
 *
 * NOTES:
 *      1. An IPC socket can only be created and hold by one process.
 *
 */
UI32_T SYSFUN_CreateIPCSocketServer(char* ipc_socket_name, int *ipc_socket_id_p, int non_blocking)
{
    struct sockaddr_un  my_sockaddr;
    int  addrlen, sockfd;
/*    int  nZero = 0; */

    if((ipc_socket_name==NULL)||(ipc_socket_id_p==NULL))
    {
        return SYSFUN_RESULT_INVALID_ARG;
    }
    
    sockfd = socket(PF_UNIX, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        printf("\r\n%s():Fail to create socket.", __FUNCTION__); fflush(stdout);
        return SYSFUN_RESULT_ERROR;
    }

    if (ioctl(sockfd, FIONBIO, (void *)&non_blocking) < 0)
    {
        printf("Failed to set server socket blocking mode. Error %d (%s).", errno, strerror(errno));
        return SYSFUN_RESULT_ERROR;
    }

    memset(&my_sockaddr, 0, sizeof(my_sockaddr));
    my_sockaddr.sun_family = AF_UNIX;
    snprintf(my_sockaddr.sun_path, sizeof(my_sockaddr.sun_path) - 1, "%s", ipc_socket_name);
    addrlen = offsetof(struct sockaddr_un, sun_path) + strlen(my_sockaddr.sun_path);
    unlink((const char *)my_sockaddr.sun_path);     /* remove old socket file if it exists */
    if (bind(sockfd, (const struct sockaddr *)&my_sockaddr, addrlen) < 0)
    {
        perror(__FUNCTION__);
        close(sockfd);
        return SYSFUN_RESULT_ERROR;
    }
#if 0
    /* disable send buffer for a better performance - because no copy the msg to send buffer
     * the following can get the default value:
     * $cat /proc/sys/net/core/wmem_default 
     */
    setsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, (char *)&nZero, sizeof(nZero));
#endif
    *ipc_socket_id_p = sockfd;
 
    return SYSFUN_OK;
}

/* FUNCTION NAME : SYSFUN_SendToIPCSocket
 * PURPOSE:
 *      Send message to IPC socket.
 *
 * INPUT:
 *      src_sockfd           -- The ipc socket id of the sender.
 *      dest_ipc_socket_name -- The ipc socket name where the message will be sent to.
 *                              The maximum length of dest_ipc_socket_name is SYS_ADPT_MAX_IPC_SOCKET_NAME_LEN.
 *      msg                  -- The message to be sent to the specified ipc socket.
 *      msg_len              -- The length of the message.
 *      (wait_time)          -- waiting time(in clock ticks) if the queue of the destination ipc socket is full.
 *                              SYSFUN_TIMEOUT_NOWAIT        : no wait.
 *                              SYSFUN_TIMEOUT_WAIT_FOREVER  : wait forever
 *                              other: the time waiting.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      Sucess -- SYSFUN_RESULT_OK
 *      Fail   -- SYSFUN_RESULT_ERROR.
 *
 * NOTES:
 *                 
 *
 */
UI32_T SYSFUN_SendToIPCSocket(int src_sockfd, char *dest_ipc_socket_name, void *msg, UI32_T msg_len)
{
  int addrlen;
  int bytesSent;
  struct sockaddr_un  dest_sockaddr;
  int flags = 0;

  memset(&dest_sockaddr, 0, sizeof(dest_sockaddr));
  dest_sockaddr.sun_family = AF_UNIX;
  snprintf(dest_sockaddr.sun_path, sizeof(dest_sockaddr.sun_path) - 1, "%s", dest_ipc_socket_name);
  addrlen = offsetof(struct sockaddr_un, sun_path) + strlen(dest_sockaddr.sun_path);
  bytesSent = sendto(src_sockfd, msg, msg_len, flags, (struct sockaddr*) &dest_sockaddr, addrlen);
  if (bytesSent != msg_len)
  {
    printf("Failed to send message with length %d to client address %s on socket with fd %d. Error (%s)\r\n.", (int)msg_len, dest_ipc_socket_name, src_sockfd, strerror(errno));
    return SYSFUN_RESULT_ERROR;
  }

  return SYSFUN_OK;
}

/* FUNCTION NAME : SYSFUN_RecvFromIPCSocket
 * PURPOSE:
 *      Send message to IPC socket.
 *
 * INPUT:
 *      sockfd               -- The ipc socket id to receive msg from.
 *      rxbuf                -- The buffer to store the rx msg
 *      buf_len              -- The length of the rxbuf.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      number of bytes it receives if successfully rx
 *
 * NOTES:
 *                 
 *
 */

int SYSFUN_RecvFromIPCSocket(int sockfd, char *rxbuf, UI32_T buf_len)
{
      return(recvfrom(sockfd, (void *)rxbuf, (int)buf_len, 0, 0, 0));
}  
#if 0
static void DRVDBG_PROC_Usage()
{
   printf("\r\n--Usage:-----\r\n");
   printf("drvdbg_proc b xxx : to execute BCM Diag command\r\n");
   printf("drvdbg_proc B     : to spawn BCM Diag shell\r\n");
   printf("drvdbg_proc p o   : to execute OF Packet-out to pipeline test\r\n");
   printf("drvdbg_proc n 1   : to execute dev_nicdrv_gateway backdoor\r\n");
   printf("drvdbg_proc n 2   : to execute dev_nicdrv backdoor\r\n");
   printf("drvdbg_proc n 3   : to execute lan backdoor\r\n"); 
   return;
}
#endif
int main(int argc, char *argv[])
{
/* to move the following to drvdbg_proc.h 
 */
#define SERVER_ADDR "/var/run/drvdbg_socket_server" /* need to match with driver_proc.c */
#define CLIENT_ADDR "/var/run/drvdbg_socket_client"

  int SocketFd = 0;
  int i;
  int size;
  unsigned long rc;
  char *ptr;
  char buffer[256];

  if (argc <=1)
  {
    //DRVDBG_PROC_Usage();
    printf("Invalid parameter(s)!\r\n");
    return -1;
  }

  size = argc;
  for (i = 1; i < argc; i++)
  {
    size += strlen(argv[i]);
  }

  if (size > sizeof(buffer))
  {
    return -2;
  }

  ptr = buffer;
  for (i = 1; i < argc; i++)
  {
    ptr = stpcpy(ptr, argv[i]);
    *ptr = ' ';
    ptr++;
  }
  ptr--;
  *(ptr--) = '\0';

  if (SYSFUN_OK != SYSFUN_CreateIPCSocketServer(SERVER_ADDR, &SocketFd, 0 /* blocking */))
  {
    printf("----- SYSFUN_CreateIPCSocketServer fail!\r\n");
    return -1;
  }
  printf("-----DRVDBG_PROC_SocketSend Msg = \"%s\".\r\n", buffer);

  system("stty -isig");

  /* wait for the client to finish processing
   */
  rc = SYSFUN_SendToIPCSocket(SocketFd, CLIENT_ADDR, buffer,  strlen(buffer) + 1);
  printf("-----Returned from DRVDBG_PROC_SocketSend with rc = %d.\r\n", rc);

  i = SYSFUN_RecvFromIPCSocket(SocketFd, buffer, 256);
  printf("Returned from SYSFUN_RecvFromIPCSocket %d bytes, buffer=%s.\r\n", i, buffer);

  system("stty isig");

  return rc;
}

