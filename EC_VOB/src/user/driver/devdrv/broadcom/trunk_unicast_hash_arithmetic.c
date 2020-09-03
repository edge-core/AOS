

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MASK  0x07
#define LENGTH 6

#define LONG2CH(a, b) \
    (b)[0] = (a) & 0xff; \
    (b)[1] = ((a)>> 8) & 0xff; \
    (b)[2] = ((a)>> 16) & 0xff; \
    (b)[3] = ((a)>> 24) & 0xff; \
    (b)[4] = ((a)>> 32) & 0xff; \
    (b)[5] = ((a)>> 40) & 0xff; 
   
    

void trunk_unicast_hash_caculate(unsigned long long da,unsigned long long sa,
                unsigned long vlan_id,unsigned long ethertype,
                unsigned long module_id, unsigned long port_id,unsigned long result[] )
{

    unsigned char index = 0;
    unsigned char ch;
    int i;
    unsigned char da_arr[LENGTH];
    unsigned char sa_arr[LENGTH];

    LONG2CH(da, da_arr);
    LONG2CH(sa, sa_arr);
    
    for(i=0 ;i<LENGTH ;i++)
    {
    	index ^= (da_arr[i]&MASK);
    	index ^= (sa_arr[i]&MASK);
    }
   
    for(i=0 ;i<sizeof(unsigned short) ;i++)
    {
    	ch = (vlan_id >> (8 * i)) & 0xff;
    	index ^= (ch&MASK);
    	
    	ch = (ethertype >> (8 * i)) & 0xff;
    	index ^= (ch&MASK);
    } 
    index ^= (module_id & MASK);
    index ^= (port_id & MASK);

    result[index]++;
    	
  
    
}



void usage()
{
    printf("trunk_unicast_hash_arithmetic da sa vlan_id ethertype module_id port_id da_step sa_step vlan_step count\n,for example : trunk_unicast_hash_arithmetic 0x000000000b01 0x000000000401 1 0x0800 0 7 1 1 0 120\n");
}

int main(int argc, char *argv[])
{
    
    unsigned long long da;
    unsigned long long sa;
    int i,j;
    unsigned long result[8]={0};
    unsigned long vlan_id;
    unsigned long ethertype;
    unsigned long module_id;
    unsigned long port_id;
    unsigned long da_step;
    unsigned long sa_step;
    unsigned long vlan_step;
    unsigned long count;
    if (argc < 11) {
        usage();
        return -1;
    }
    da = strtoull(argv[1],NULL,16);
    sa = strtoull(argv[2],NULL,16);
    vlan_id = strtoul(argv[3],NULL,10);
    ethertype = strtoul(argv[4],NULL,16);
    module_id = strtoul(argv[5],NULL,10);
    port_id = strtoul(argv[6],NULL,10);
    da_step = strtoul(argv[7],NULL,10);
    sa_step = strtoul(argv[8],NULL,10);
    vlan_step = strtoul(argv[9],NULL,10);
    count = strtoul(argv[10],NULL,10);
    
    for(i=0; i<count; i++) {
        da +=  da_step;
        sa +=  sa_step;
        vlan_id += vlan_step;
        trunk_unicast_hash_caculate(da,sa,vlan_id,ethertype,module_id,port_id,result);
    }
   
    for(i=0;i<8;i++)
    {
        printf("the result[%d],  %d precent %d %%\n",i,result[i], (result[i] * 100) / count);
    }
    return 0;
}
