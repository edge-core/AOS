#!/bin/sh
# udhcpc script edited by Tim Riker <Tim@Rikers.org> 
[ -z "$1" ] && echo "Error: should be called from udhcpc" && exit 1

# lease_file must be the same as the one defined in cli_mgr.c
lease_file="/var/run/udhcpc.tftp_filename"
dns_file="/var/run/udhcpc.dns_filename"
rm -f $lease_file $dns_file
    #RESOLV_CONF="/etc/resolv.conf"
    #[ -n "$broadcast" ] && BROADCAST="broadcast $broadcast"
    #[ -n "$subnet" ] && NETMASK="netmask $subnet"
case "$1" in
        deconfig)
         #       /sbin/ifconfig $interface 0.0.0.0
                ;;
        renew|bound)
#                echo "got gateway IP=" $router
#                echo "got interface name=" $interface
#                echo "got IP address=" $ip
#                echo "got subnet netmask=" $subnet
#                echo "got serverid=" $serverid
#                echo "got sia[Addr=" $siaddr
#                echo "got opt tftp=" $tftp
#                echo "got opt bootfile=" $bootfile
#                echo "storing tftp and bootfile to $lease_file"

            # start with '!' indicating error
            # end with '.' indicating finishing writing to file
            if [ ${#tftp} -ge 128 ]; then
                echo "! Invalid tftp IP address -- lengh >= 128 !" > $lease_file
                if [ ${#bootfile} -ge 128 ]; then
                    echo "! Invalid bootfile name -- lengh >= 128 !" >> $lease_file
                fi
                exit 1
            fi

            echo  "# $tftp $bootfile ." > $lease_file
            
            # put dns data to file
            for i in $dns ; do
                echo  $i >> $dns_file
            done
            
        # assume /etc/network/interfaces already config CRAFT port properly

        #   /sbin/ifconfig $interface $ip $BROADCAST $NETMASK       
        #   if [ -n "$router" ] ; then      
#           echo "deleting routers"      
#           while route del default gw 0.0.0.0 dev $interface ; do        
#               :       
#           done        
    #       for i in $router ; do        
    #           route add default gw $i dev $interface       
    #       done    
    #   fi      
    #   echo -n > $RESOLV_CONF     
    #   [ -n "$domain" ] && echo search $domain >> $RESOLV_CONF     
    #   for i in $dns ; do      
    #       echo adding dns $i       
    #       echo nameserver $i >> $RESOLV_CONF     
    #   done    
        ;; 
esac 
exit 0 
