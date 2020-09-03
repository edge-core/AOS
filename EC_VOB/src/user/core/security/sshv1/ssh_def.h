/*Isiah. 2002-05-28 */
#ifndef SSH_DEF_H
#define SSH_DEF_H

/*replace write() and read() with send() and recv() ,
  because phase2 not support write() and read() */
#define write(s,b,n) send((s),(b),(n),0)
#define read(s,b,n) recv((s),(b),(n),0)




#endif /* #ifndef SSH_DEF_H */