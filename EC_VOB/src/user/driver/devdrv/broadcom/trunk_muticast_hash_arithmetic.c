

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MASK  0x03
#define LENGTH 6

#define LONG2CH(a, b) \
    (b)[0] = (a) & 0xff; \
    (b)[1] = ((a)>> 8) & 0xff; \
    (b)[2] = ((a)>> 16) & 0xff; \
    (b)[3] = ((a)>> 24) & 0xff; \
    (b)[4] = ((a)>> 32) & 0xff; \
    (b)[5] = ((a)>> 40) & 0xff; 
    

void trunk_muticast_hash_caculate(unsigned long long da,unsigned long long sa,
                unsigned long module_id, unsigned long port_id,unsigned long result[] )
{

    unsigned char index = 0;
    unsigned char ch;
    int i;
    unsigned char da_arr[LENGTH];
    unsigned char sa_arr[LENGTH];
    LONG2CH(da, da_arr);
    LONG2CH(sa, sa_arr);
    ch = ((module_id & MASK) << 2) | (port_id & MASK);
    da_arr[0] &= 0x0f;
    sa_arr[0] &= 0x0f;
    ch &= 0x0f;
    index ^= da_arr[0] ^ sa_arr[0] ^ ch;
    if(index >=8)
    {
    index = index - 8;
    }

    if ((index % 2)==0)
    {
     index = index + 1;
    }
    else
    {
     index = index - 1;
    }
    
    
    result[index]++;
}

void usage()
{
     printf("trunk_muticast_hash_arithmetic da sa module_id port_id da_step sa_step count\n,for example : trunk_muticast_hash_arithmetic 0xffffffffffff 0x000000000401 0 7 0 0 120\n");
}

int main(int argc, char *argv[])
{
    unsigned long long da;
    unsigned long long sa;
    int i,j;
    unsigned char da_arr[LENGTH];
    unsigned char sa_arr[LENGTH];
    unsigned long result[16]={0};
    unsigned long module_id;
    unsigned long port_id;
    unsigned long da_step;
    unsigned long sa_step;
    unsigned long count;
    
    if (argc < 8) {
        usage();
        return -1;
    }
    da = strtoull(argv[1],NULL,16);
    sa = strtoull(argv[2],NULL,16);
    module_id = strtoul(argv[3],NULL,10);
    port_id = strtoul(argv[4],NULL,10);
    da_step = strtoul(argv[5],NULL,10);
    sa_step = strtoul(argv[6],NULL,10);
    count = strtoul(argv[7],NULL,10);
    
    for(i=0; i<count; i++) {
        da +=  da_step;
        sa +=  sa_step;
        trunk_muticast_hash_caculate(da,sa,module_id,port_id,result);
    }
    
    for(i=0;i<8;i++)
    {
        printf("the result[%d],  %d precent %d %%\n",i,result[i], (result[i] * 100) / (count));
    }
    return 0;
}
