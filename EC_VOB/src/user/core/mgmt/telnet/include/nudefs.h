/************************************************************************/
/*                                                                      */
/*              Copyright 1994, Integrated Systems Inc.                 */
/*                      ALL RIGHTS RESERVED                             */
/*                                                                      */
/*   This computer program is the property of Integrated Systems Inc.   */
/*   Santa Clara, California, U.S.A. and may not be copied              */
/*   in any form or by any means, whether in part or in whole,          */
/*   except under license expressly granted by Integrated Systems Inc.  */
/*                                                                      */
/*   All copies of this program, whether in part or in whole, and       */
/*   whether modified or not, must display this and all other           */
/*   embedded copyright and ownership notices in full.                  */
/*                                                                      */
/************************************************************************/
#ifndef _nudefs_h
#define _nudefs_h

#ifdef SPNAUTILS
/* 
 * COMMON FUNCTION and VARIABLES 
 */

/* functions */
#define psh_cmd                 _onu_psh_cmd
#define psh_get_dport           _onu_psh_get_dport
#define psh_printf              _onu_psh_printf
#define psh_putchar             _onu_psh_putchar
#define psh_fflush              _onu_psh_fflush
#define psh_gets                _onu_psh_gets
#define psh_getpw               _onu_psh_getpw
#define psh_get_ttychars        _onu_psh_get_ttychars
#define psys_inet_addr          _onu_psys_inet_addr
#define psys_access             _onu_psys_access
#define psys_stat_x             _onu_psys_stat_x
#define psys_inet_ntoa          _onu_psys_inet_ntoa
#define map_stdio_to_sock       _onu_map_stdio_to_sock
#define inet_ntoa               _onu_inet_ntoa

/* variables */
#define console_dev             _onu_console_dev
#define version                 _onu_version


/* Other Directories */

/* main */

/* bootpd */
#define bootpd_exit             _onu_bootpd_exit
#define iplookcmp               _onu_iplookcmp
#define haddrtoa                _onu_haddrtoa
#define insert_u_long           _onu_insert_u_long
#define readtab                 _onu_readtab
#define report                  _onu_report
#define hash_Init               _onu_hash_Init
#define hash_Reset              _onu_hash_Reset
#define hash_HashFunction       _onu_hash_HashFunction
#define hash_Insert             _onu_hash_Insert
#define hash_Lookup             _onu_hash_Lookup
#define hash_NextEntry          _onu_hash_NextEntry
#define hash_FirstEntry         _onu_hash_FirstEntry
#define get_network_conf        _onu_get_network_conf
#define dump_ifconf             _onu_dump_ifconf
#define get_route               _onu_get_route
#define bdump_route             _onu_bdump_route
#define get_outIP               _onu_get_outIP

#define vm_isi                  _onu_vm_isi
#define vm_rfc1048              _onu_vm_rfc1048
#define hwinfolist              _onu_hwinfolist

#define _bootpd_cfg             _onu__bootpd_cfg
#define ifconftable             _onu_ifconftable
#define routetable              _onu_routetable
#define hwhashtable             _onu_hwhashtable
#define iphashtable             _onu_iphashtable
#define nmhashtable             _onu_nmhashtable
#define logfp                   _onu_logfp


/* ftp */
#define ftp_lostpeer            _onu_ftp_lostpeer
#define ftp_getcmd              _onu_ftp_getcmd
#define ftp_makeargv            _onu_ftp_makeargv
#define ftp_help                _onu_ftp_help
#define ftp_setpeer             _onu_ftp_setpeer
#define ftp_setbinary           _onu_ftp_setbinary
#define ftp_setascii            _onu_ftp_setascii
#define ftp_settenex            _onu_ftp_settenex
#define ftp_setebcdic           _onu_ftp_setebcdic
#define ftp_quit                _onu_ftp_quit
#define ftp_domacro             _onu_ftp_domacro
#define ftp_glob                _onu_ftp_glob
#define ftp_blkfree             _onu_ftp_blkfree
#define ftp_hookup              _onu_ftp_hookup
#define ftp_login               _onu_ftp_login
#define ftp_command             _onu_ftp_command
#define ftp_getreply            _onu_ftp_getreply
#define ftp_empty               _onu_ftp_empty
#define ftp_sendrequest         _onu_ftp_sendrequest
#define ftp_recvrequest         _onu_ftp_recvrequest
#define ftp_pswitch             _onu_ftp_pswitch
#define ftp_getc                _onu_ftp_getc
#define ftp_putc                _onu_ftp_putc
#define ftp_fprintf             _onu_ftp_fprintf

#define ftp_cmdtab              _onu_ftp_cmdtab
#define ftp_NCMDS               _onu_ftp_NCMDS


/* ftpd */
#define ftpd_exit               _onu_ftpd_exit
#define ftpd_main               _onu_ftpd_main
#define ftpd_user               _onu_ftpd_user
#define ftpd_pass               _onu_ftpd_pass
#define ftpd_retrieve           _onu_ftpd_retrieve
#define ftpd_store              _onu_ftpd_store
#define ftpd_statfilecmd        _onu_ftpd_statfilecmd
#define ftpd_statcmd            _onu_ftpd_statcmd
#define ftpd_fatal              _onu_ftpd_fatal
#define reply                   _onu_reply
#define lreply                  _onu_lreply
#define ftpd_ack                _onu_ftpd_ack
#define ftpd_nack               _onu_ftpd_nack
#define yyerror                 _onu_yyerror
#define ftpd_delete             _onu_ftpd_delete
#define ftpd_cwd                _onu_ftpd_cwd
#define ftpd_makedir            _onu_ftpd_makedir
#define ftpd_removedir          _onu_ftpd_removedir
#define ftpd_pwd                _onu_ftpd_pwd
#define ftpd_renamefrom         _onu_ftpd_renamefrom
#define ftpd_renamecmd          _onu_ftpd_renamecmd
#define ftpd_dolog              _onu_ftpd_dolog
#define ftpd_dologout           _onu_ftpd_dologout
#define ftpd_passive            _onu_ftpd_passive
#define perror_reply            _onu_perror_reply
#define ftpd_send_file_list     _onu_ftpd_send_file_list
#define yyparse                 _onu_yyparse
#define ftpd_logwtmp            _onu_ftpd_logwtmp
#define ftpd_glob               _onu_ftpd_glob
#define socket_putchar          _onu_socket_putchar
#define socket_printl           _onu_socket_printl
#define socket_fflush           _onu_socket_fflush
#define socket_getc             _onu_socket_getc
#define socket_putc             _onu_socket_putc
#define ftpd_filesize           _onu_ftpd_filesize
#define ftpd_maperror           _onu_ftpd_maperror
#define ftpd_gethostname        _onu_ftpd_gethostname
#define ftpd_inet_ntoa          _onu_ftpd_inet_ntoa
#define ftpd_time               _onu_ftpd_time
#define ftpd_ctime              _onu_ftpd_ctime
#define ftpd_getpwnam           _onu_ftpd_getpwnam
#define ftpd_openlog            _onu_ftpd_openlog
#define ftpd_checkhlist         _onu_ftpd_checkhlist
#define ftpd_stat               _onu_ftpd_stat

#define ftpd_debug              _onu_ftpd_debug
#define ftpd_timeout            _onu_ftpd_timeout
#define ftpd_maxtimeout         _onu_ftpd_maxtimeout
#define ftpd_logging            _onu_ftpd_logging
#define ftpd_usedefault         _onu_ftpd_usedefault
#define cmdtab                  _onu_cmdtab
#define sitetab                 _onu_sitetab
#define yyr1                    _onu_yyr1

#define ftpd_hostname           _onu_ftpd_hostname

/* psh */
#define psh_help                _onu_psh_help
#define psh_create_env          _onu_psh_create_env
#define psh_pttydev_getmode     _onu_psh_pttydev_getmode
#define psh_pttydev_setmode     _onu_psh_pttydev_setmode
#define psh_pttydev_init        _onu_psh_pttydev_init
#define psh_pttydev_alloc       _onu_psh_pttydev_alloc
#define psh_pttydev_free        _onu_psh_pttydev_free
#define psh_pttydev_dup         _onu_psh_pttydev_dup
#define psh_read                _onu_psh_read
#define psh_write               _onu_psh_write
#define psh_getchar             _onu_psh_getchar
#define psh_parse_cmd           _onu_psh_parse_cmd
#define pshd_detach_terminal    _onu_pshd_detach_terminal
#define pshd_attach_terminal    _onu_pshd_attach_terminal
#define pshd_attach_console     _onu_pshd_attach_console
#define pshd_detach_console     _onu_pshd_detach_console
#define pshd_io_init            _onu_pshd_io_init
#define psh_set_echo            _onu_psh_set_echo
#define psys_init               _onu_psys_init
#define psys_exec               _onu_psys_exec
#define psys_pwd                _onu_psys_pwd
#define psys_help               _onu_psys_help
#define psys_sync               _onu_psys_sync
#define psys_clear              _onu_psys_clear
#define psys_maperror           _onu_psys_maperror
#define psys_cwdtmp             _onu_psys_cwdtmp
#define psys_getopt             _onu_psys_getopt
#define psys_getans             _onu_psys_getans
#define psys_chmod              _onu_psys_chmod
#define psys_stat               _onu_psys_stat
#define psys_getdir             _onu_psys_getdir
#define psys_getdevnum          _onu_psys_getdevnum
#define psys_getfstype          _onu_psys_getfstype
#define psys_getvoltype         _onu_psys_getvoltype
#define psys_inet_lnaof         _onu_psys_inet_lnaof
#define psys_inet_network       _onu_psys_inet_network
#define psys_touch              _onu_psys_touch
#define psys_rmdir              _onu_psys_rmdir
#define psys_mkdir              _onu_psys_mkdir
#define psys_rm                 _onu_psys_rm
#define psys_cat                _onu_psys_cat
#define psys_echo               _onu_psys_echo
#define psys_mv                 _onu_psys_mv
#define psys_cp                 _onu_psys_cp
#define psys_du                 _onu_psys_du
#define psys_ls                 _onu_psys_ls
#define psys_ping               _onu_psys_ping
#define psys_date               _onu_psys_date
#define psys_tail               _onu_psys_tail
#define psys_head               _onu_psys_head
#define psys_cmp                _onu_psys_cmp
#define psys_mount              _onu_psys_mount
#define psys_umount             _onu_psys_umount
#define psys_popd               _onu_psys_popd
#define psys_pushd              _onu_psys_pushd
#define psys_cd                 _onu_psys_cd
#define psys_suspend            _onu_psys_suspend
#define psys_resume             _onu_psys_resume
#define psys_sleep              _onu_psys_sleep
#define psys_kill               _onu_psys_kill
#define psys_getid              _onu_psys_getid
#define psys_setid              _onu_psys_setid
#define psys_console            _onu_psys_console
#define psys_nfsmount           _onu_psys_nfsmount
#define psys_pcmount            _onu_psys_pcmount
#define psys_mkfs               _onu_psys_mkfs
#define psys_pcmkfs             _onu_psys_pcmkfs
#define map_stdio_to_file       _onu_map_stdio_to_file
#define psys_setpri             _onu_psys_setpri
#define psys_getpri             _onu_psys_getpri
#define psys_setenv             _onu_psys_setenv
#define psh_tinit               _onu_psh_tinit
#define psys_arp                _onu_psys_arp
#define psys_ifconfig           _onu_psys_ifconfig
#define Perror                  _onu_Perror
#define printb                  _onu_printb
#define offtime                 _onu_offtime
#define psys_netstat            _onu_psys_netstat
#define psys_route              _onu_psys_route
#define psys_cdmount            _onu_psys_cdmount
            
#define psys_cmdtab             _onu_psys_cmdtab
#define psys_ncmds              _onu_psys_ncmds
#define cmds                    _onu_cmds
            
#define user_apptab             _onu_user_apptab
#define user_cmdtab             _onu_user_cmdtab
#define statfcn                 _onu_statfcn
#define sortfcn                 _onu_sortfcn
#define printfcn                _onu_printfcn
#define psys_mntaddr            _onu_psys_mntaddr
#define psys_mntdir             _onu_psys_mntdir
            
            
/* Routed */
#define timevaladd              _onu_timevaladd
#define rtlookup                _onu_rtlookup
#define rtfind                  _onu_rtfind
#define rtadd                   _onu_rtadd
#define rtchange                _onu_rtchange
#define rtdelete                _onu_rtdelete
#define rtdeleteall             _onu_rtdeleteall
#define rtdefault               _onu_rtdefault
#define rtinit                  _onu_rtinit
#define ifinit                  _onu_ifinit
#define addrouteforif           _onu_addrouteforif
#define gwkludge                _onu_gwkludge
#define toall                   _onu_toall
#define rsendmsg                _onu_rsendmsg
#define supply                  _onu_supply
#define rip_input               _onu_rip_input
#define inet_makeaddr           _onu_inet_makeaddr
#define inet_netof              _onu_inet_netof
#define inet_rtflags            _onu_inet_rtflags
#define inet_sendroute          _onu_inet_sendroute
#define if_ifwithaddr           _onu_if_ifwithaddr
#define if_ifwithdstaddr        _onu_if_ifwithdstaddr
#define if_ifwithnet            _onu_if_ifwithnet
#define if_iflookup             _onu_if_iflookup
#define timer                   _onu_timer
#define insque                  _onu_insque
#define remque                  _onu_remque
#define syslog                  _onu_syslog
#define rperror                 _onu_rperror
#define bcopy                   _onu_bcopy
#define routed_bzero            _onu_routed_bzero
#define bcmp                    _onu_bcmp
#define rmalloc                 _onu_rmalloc
#define rfree                   _onu_rfree
#define rexit                   _onu_rexit
#define random                  _onu_random
#define srandom                 _onu_srandom
#define gettimeofday            _onu_gettimeofday
#define rstrcmp                 _onu_rstrcmp
#define getpid                  _onu_getpid
#define print                   _onu_print
            
#define msg                     _onu_msg
#define afswitch                _onu_afswitch
#define af_max                  _onu_af_max
#define inet_default            _onu_inet_default
            
#define s                       _onu_s
#define sid                     _onu_sid
#define now                     _onu_now
#define lastbcast               _onu_lastbcast
#define lastfullupdate          _onu_lastfullupdate
#define nextbcast               _onu_nextbcast
#define needupdate              _onu_needupdate
#define timerupdate             _onu_timerupdate
#define packet                  _onu_packet
#define routed_syslog           _onu_routed_syslog
#define supplier                _onu_supplier
#define nethash                 _onu_nethash
#define hosthash                _onu_hosthash
#define ifnet                   _onu_ifnet
#define lookforinterfaces       _onu_lookforinterfaces
#define faketime                _onu_faketime
#define modify_route            _onu_modify_route
            
            
/* TFTP */
#define log_msg                 _onu_log_msg
#define nak                     _onu_nak
#define rw_allocate             _onu_rw_allocate
#define rw_deallocate           _onu_rw_deallocate
#define rw_init                 _onu_rw_init
#define w_init                  _onu_w_init
#define r_init                  _onu_r_init
#define readit                  _onu_readit
#define read_ahead              _onu_read_ahead
#define writeit                 _onu_writeit
#define write_behind            _onu_write_behind
#define synchnet                _onu_synchnet

#define TFTPD_SYSSTACK          _onu_TFTPD_SYSSTACK
#define TFTPD_MODE              _onu_TFTPD_MODE
            
#define formats                 _onu_formats
#define errmsgs                 _onu_errmsgs
            
#define log_fid                 _onu_log_fid
#define log_enabled             _onu_log_enabled
            
/* Telnet */
#define tn_init                 _onu_tn_init
#define telnet_exit             _onu_telnet_exit
#define tn_set_binmode          _onu_tn_set_binmode
#define tn_gets                 _onu_tn_gets
#define tn_init_telnet          _onu_tn_init_telnet
#define tn_willoption           _onu_tn_willoption
#define tn_wontoption           _onu_tn_wontoption
#define telsnd                  _onu_telsnd
#define Scheduler               _onu_Scheduler
#define telnet                  _onu_telnet
#define tn_xmitAO               _onu_tn_xmitAO
#define tn_xmitEL               _onu_tn_xmitEL
#define tn_xmitEC               _onu_tn_xmitEC
#define tn_dosynch              _onu_tn_dosynch
#define tn_intp                 _onu_tn_intp
#define tn_sendbrk              _onu_tn_sendbrk
#define tn_ring_init            _onu_tn_ring_init
#define tn_ring_mark            _onu_tn_ring_mark
#define tn_ring_at_mark         _onu_tn_ring_at_mark
#define tn_ring_clear_mark      _onu_tn_ring_clear_mark
#define tn_ring_supplied        _onu_tn_ring_supplied
#define tn_ring_consumed        _onu_tn_ring_consumed
#define tn_ring_empty_count     _onu_tn_ring_empty_count
#define tn_ring_empty_consecutive   _onu_tn_ring_empty_consecutive
#define tn_ring_full_count      _onu_tn_ring_full_count
#define tn_ring_full_consecutive    _onu_tn_ring_full_consecutive
#define tn_ring_supply_data     _onu_tn_ring_supply_data
#define tn_ring_consume_data    _onu_tn_ring_consume_data
#define tn_init_network         _onu_tn_init_network
#define tn_stilloob             _onu_tn_stilloob
#define tn_setneturg            _onu_tn_setneturg
#define tn_netflush             _onu_tn_netflush
#define tn_quit                 _onu_tn_quit
#define tn                      _onu_tn
#define tn_command              _onu_tn_command
#define tn_upcase               _onu_tn_upcase
#define tn_SetSockOpt           _onu_tn_SetSockOpt
#define tn_Dump                 _onu_tn_Dump
#define tn_printoption          _onu_tn_printoption
#define tn_printsub             _onu_tn_printsub
#define tn_init_sys             _onu_tn_init_sys
#define TerminalWrite           _onu_TerminalWrite
#define TerminalRead            _onu_TerminalRead
#define TerminalAutoFlush       _onu_TerminalAutoFlush
#define TerminalSpecialChars    _onu_TerminalSpecialChars
#define TerminalFlushOutput     _onu_TerminalFlushOutput
#define TerminalSaveState       _onu_TerminalSaveState
#define TerminalRestoreState    _onu_TerminalRestoreState
#define TerminalNewMode         _onu_TerminalNewMode
#define tn_NetClose             _onu_tn_NetClose
#define tn_sys_init             _onu_tn_sys_init
#define tn_process_rings        _onu_tn_process_rings
#define tn_init_terminal        _onu_tn_init_terminal
#define tn_ttyflush             _onu_tn_ttyflush
#define tn_getconnmode          _onu_tn_getconnmode
#define tn_setconnmode          _onu_tn_setconnmode
#define tn_setcommandmode       _onu_tn_setcommandmode
            
#define tn_doopt                _onu_tn_doopt
#define tn_dont                 _onu_tn_dont
#define tn_will                 _onu_tn_will
#define tn_wont                 _onu_tn_wont
#define tn_connected            _onu_tn_connected
#define tn_debug                _onu_tn_debug
#define tn_crmod                _onu_tn_crmod
#define tn_netdata              _onu_tn_netdata
#define tn_crlf                 _onu_tn_crlf
#define tn_telnetport           _onu_tn_telnetport
#define tn_SYNCHing             _onu_tn_SYNCHing
#define tn_flushout             _onu_tn_flushout
#define tn_autoflush            _onu_tn_autoflush
#define tn_autosynch            _onu_tn_autosynch
#define tn_localchars           _onu_tn_localchars
#define tn_donelclchars         _onu_tn_donelclchars
#define tn_donebinarytoggle     _onu_tn_donebinarytoggle
#define tn_dontlecho            _onu_tn_dontlecho
#define tn_globalmode           _onu_tn_globalmode
#define toplevel                _onu_toplevel
#define tn_modelist             _onu_tn_modelist
#define tn_NetTrace             _onu_tn_NetTrace
#define tn_opts                 _onu_tn_opts
#define tn_term_name            _onu_tn_term_name
#define tn_exit_param           _onu_tn_exit_param
#define tn_close_flag           _onu_tn_close_flag
#define tn_hisopts              _onu_tn_hisopts
#define tn_myopts               _onu_tn_myopts
#define tn_prompt               _onu_tn_prompt
#define tn_escape               _onu_tn_escape
#define tn_echoc                _onu_tn_echoc
#define peerdied                _onu_peerdied
#define tn_clocks               _onu_tn_clocks
#define tn_netoring             _onu_tn_netoring
#define tn_netiring             _onu_tn_netiring
#define tn_netobuf              _onu_tn_netobuf
#define tn_netibuf              _onu_tn_netibuf
#define tn_hostname             _onu_tn_hostname
#define tn_showoptions          _onu_tn_showoptions
#define tn_tin                  _onu_tn_tin
#define tn_net                  _onu_tn_net
#define tn_HaveInput            _onu_tn_HaveInput
#define tn_ttyoring             _onu_tn_ttyoring
#define tn_ttyiring             _onu_tn_ttyiring
#define tn_ttyobuf              _onu_tn_ttyobuf
#define tn_ttyibuf              _onu_tn_ttyibuf
#define tn_termEofChar          _onu_tn_termEofChar
#define tn_termEraseChar        _onu_tn_termEraseChar
#define tn_termFlushChar        _onu_tn_termFlushChar
#define tn_termIntChar          _onu_tn_termIntChar
#define tn_termKillChar         _onu_tn_termKillChar
#define tn_termQuitChar         _onu_tn_termQuitChar

/* Telnetd */
#endif  /* SPNAUTILS */

#ifdef PNAUTILS
/* 
 * COMMON FUNCTION and VARIABLES 
 */

/* functions */
#define psh_cmd                 _nu_psh_cmd
#define psh_get_dport           _nu_psh_get_dport
#define psh_printf              _nu_psh_printf
#define psh_putchar             _nu_psh_putchar
#define psh_fflush              _nu_psh_fflush
#define psh_gets                _nu_psh_gets
#define psh_getpw               _nu_psh_getpw
#define psh_get_ttychars        _nu_psh_get_ttychars
#define psys_inet_addr          _nu_psys_inet_addr
#define psys_access             _nu_psys_access
#define psys_stat_x             _nu_psys_stat_x
#define psys_inet_ntoa          _nu_psys_inet_ntoa
#define map_stdio_to_sock       _nu_map_stdio_to_sock
#define inet_ntoa               _nu_inet_ntoa
                                
/* variables */                 
#define console_dev             _nu_console_dev
#define version                 _nu_version
                                
                                
/* Other Directories */         
                                
/* main */                      
                                
/* bootpd */                    
#define bootpd_exit             _nu_bootpd_exit
#define iplookcmp               _nu_iplookcmp
#define haddrtoa                _nu_haddrtoa
#define insert_u_long           _nu_insert_u_long
#define readtab                 _nu_readtab
#define report                  _nu_report
#define hash_Init               _nu_hash_Init
#define hash_Reset              _nu_hash_Reset
#define hash_HashFunction       _nu_hash_HashFunction
#define hash_Insert             _nu_hash_Insert
#define hash_Lookup             _nu_hash_Lookup
#define hash_NextEntry          _nu_hash_NextEntry
#define hash_FirstEntry         _nu_hash_FirstEntry
#define get_network_conf        _nu_get_network_conf
#define dump_ifconf             _nu_dump_ifconf
#define get_route               _nu_get_route
#define bdump_route             _nu_bdump_route
#define get_outIP               _nu_get_outIP

#define vm_isi                  _nu_vm_isi
#define vm_rfc1048              _nu_vm_rfc1048
#define hwinfolist              _nu_hwinfolist

#define _bootpd_cfg             _nu__bootpd_cfg
#define ifconftable             _nu_ifconftable
#define routetable              _nu_routetable
#define hwhashtable             _nu_hwhashtable
#define iphashtable             _nu_iphashtable
#define nmhashtable             _nu_nmhashtable
#define logfp                   _nu_logfp


/* ftp */
#define ftp_lostpeer        _nu_ftp_lostpeer
#define ftp_getcmd          _nu_ftp_getcmd
#define ftp_makeargv        _nu_ftp_makeargv
#define ftp_help        _nu_ftp_help
#define ftp_setpeer     _nu_ftp_setpeer
#define ftp_setbinary       _nu_ftp_setbinary
#define ftp_setascii        _nu_ftp_setascii
#define ftp_settenex        _nu_ftp_settenex
#define ftp_setebcdic       _nu_ftp_setebcdic
#define ftp_quit        _nu_ftp_quit
#define ftp_domacro     _nu_ftp_domacro
#define ftp_glob        _nu_ftp_glob
#define ftp_blkfree     _nu_ftp_blkfree
#define ftp_hookup      _nu_ftp_hookup
#define ftp_login       _nu_ftp_login
#define ftp_command     _nu_ftp_command
#define ftp_getreply        _nu_ftp_getreply
#define ftp_empty       _nu_ftp_empty
#define ftp_sendrequest     _nu_ftp_sendrequest
#define ftp_recvrequest     _nu_ftp_recvrequest
#define ftp_pswitch     _nu_ftp_pswitch
#define ftp_getc        _nu_ftp_getc
#define ftp_putc        _nu_ftp_putc
#define ftp_fprintf     _nu_ftp_fprintf

#define ftp_cmdtab      _nu_ftp_cmdtab
#define ftp_NCMDS       _nu_ftp_NCMDS


/* ftpd */
#define ftpd_exit       _nu_ftpd_exit
#define ftpd_main       _nu_ftpd_main
#define ftpd_user       _nu_ftpd_user
#define ftpd_pass       _nu_ftpd_pass
#define ftpd_retrieve       _nu_ftpd_retrieve
#define ftpd_store      _nu_ftpd_store
#define ftpd_statfilecmd    _nu_ftpd_statfilecmd
#define ftpd_statcmd        _nu_ftpd_statcmd
#define ftpd_fatal      _nu_ftpd_fatal
#define reply           _nu_reply
#define lreply          _nu_lreply
#define ftpd_ack        _nu_ftpd_ack
#define ftpd_nack       _nu_ftpd_nack
#define yyerror         _nu_yyerror
#define ftpd_delete     _nu_ftpd_delete
#define ftpd_cwd        _nu_ftpd_cwd
#define ftpd_makedir        _nu_ftpd_makedir
#define ftpd_removedir      _nu_ftpd_removedir
#define ftpd_pwd        _nu_ftpd_pwd
#define ftpd_renamefrom     _nu_ftpd_renamefrom
#define ftpd_renamecmd      _nu_ftpd_renamecmd
#define ftpd_dolog      _nu_ftpd_dolog
#define ftpd_dologout       _nu_ftpd_dologout
#define ftpd_passive        _nu_ftpd_passive
#define perror_reply        _nu_perror_reply
#define ftpd_send_file_list _nu_ftpd_send_file_list
#define yyparse         _nu_yyparse
#define ftpd_logwtmp        _nu_ftpd_logwtmp
#define ftpd_glob       _nu_ftpd_glob
#define socket_putchar      _nu_socket_putchar
#define socket_printl       _nu_socket_printl
#define socket_fflush       _nu_socket_fflush
#define socket_getc     _nu_socket_getc
#define socket_putc     _nu_socket_putc
#define ftpd_filesize       _nu_ftpd_filesize
#define ftpd_maperror       _nu_ftpd_maperror
#define ftpd_gethostname    _nu_ftpd_gethostname
#define ftpd_inet_ntoa      _nu_ftpd_inet_ntoa
#define ftpd_time       _nu_ftpd_time
#define ftpd_ctime      _nu_ftpd_ctime
#define ftpd_getpwnam       _nu_ftpd_getpwnam
#define ftpd_openlog        _nu_ftpd_openlog
#define ftpd_checkhlist     _nu_ftpd_checkhlist
#define ftpd_stat       _nu_ftpd_stat

#define ftpd_debug      _nu_ftpd_debug
#define ftpd_timeout        _nu_ftpd_timeout
#define ftpd_maxtimeout     _nu_ftpd_maxtimeout
#define ftpd_logging        _nu_ftpd_logging
#define ftpd_usedefault     _nu_ftpd_usedefault
#define cmdtab          _nu_cmdtab
#define sitetab         _nu_sitetab
#define yyr1            _nu_yyr1

#define ftpd_hostname       _nu_ftpd_hostname

/* psh */
#define psh_help            _nu_psh_help
#define psh_create_env          _nu_psh_create_env
#define psh_pttydev_getmode     _nu_psh_pttydev_getmode
#define psh_pttydev_setmode     _nu_psh_pttydev_setmode
#define psh_pttydev_init        _nu_psh_pttydev_init
#define psh_pttydev_alloc       _nu_psh_pttydev_alloc
#define psh_pttydev_free        _nu_psh_pttydev_free
#define psh_pttydev_dup         _nu_psh_pttydev_dup
#define psh_read            _nu_psh_read
#define psh_write           _nu_psh_write
#define psh_getchar         _nu_psh_getchar
#define psh_parse_cmd           _nu_psh_parse_cmd
#define pshd_detach_terminal        _nu_pshd_detach_terminal
#define pshd_attach_terminal        _nu_pshd_attach_terminal
#define pshd_attach_console     _nu_pshd_attach_console
#define pshd_detach_console     _nu_pshd_detach_console
#define pshd_io_init            _nu_pshd_io_init
#define psh_set_echo            _nu_psh_set_echo
#define psys_init           _nu_psys_init
#define psys_exec           _nu_psys_exec
#define psys_pwd            _nu_psys_pwd
#define psys_help           _nu_psys_help
#define psys_sync           _nu_psys_sync
#define psys_clear          _nu_psys_clear
#define psys_maperror           _nu_psys_maperror
#define psys_cwdtmp         _nu_psys_cwdtmp
#define psys_getopt         _nu_psys_getopt
#define psys_getans         _nu_psys_getans
#define psys_chmod          _nu_psys_chmod
#define psys_stat           _nu_psys_stat
#define psys_getdir         _nu_psys_getdir
#define psys_getdevnum          _nu_psys_getdevnum
#define psys_getfstype          _nu_psys_getfstype
#define psys_getvoltype         _nu_psys_getvoltype
#define psys_inet_lnaof         _nu_psys_inet_lnaof
#define psys_inet_network       _nu_psys_inet_network
#define psys_touch          _nu_psys_touch
#define psys_rmdir          _nu_psys_rmdir
#define psys_mkdir          _nu_psys_mkdir
#define psys_rm             _nu_psys_rm
#define psys_cat            _nu_psys_cat
#define psys_echo           _nu_psys_echo
#define psys_mv             _nu_psys_mv
#define psys_cp             _nu_psys_cp
#define psys_du             _nu_psys_du
#define psys_ls             _nu_psys_ls
#define psys_ping           _nu_psys_ping
#define psys_date           _nu_psys_date
#define psys_tail           _nu_psys_tail
#define psys_head           _nu_psys_head
#define psys_cmp            _nu_psys_cmp
#define psys_mount          _nu_psys_mount
#define psys_umount         _nu_psys_umount
#define psys_popd           _nu_psys_popd
#define psys_pushd          _nu_psys_pushd
#define psys_cd             _nu_psys_cd
#define psys_suspend            _nu_psys_suspend
#define psys_resume         _nu_psys_resume
#define psys_sleep          _nu_psys_sleep
#define psys_kill           _nu_psys_kill
#define psys_getid          _nu_psys_getid
#define psys_setid          _nu_psys_setid
#define psys_console            _nu_psys_console
#define psys_nfsmount           _nu_psys_nfsmount
#define psys_pcmount            _nu_psys_pcmount
#define psys_mkfs           _nu_psys_mkfs
#define psys_pcmkfs         _nu_psys_pcmkfs
#define map_stdio_to_file       _nu_map_stdio_to_file
#define psys_setpri         _nu_psys_setpri
#define psys_getpri         _nu_psys_getpri
#define psys_setenv         _nu_psys_setenv
#define psh_tinit           _nu_psh_tinit
#define psys_arp            _nu_psys_arp
#define psys_ifconfig           _nu_psys_ifconfig
#define Perror              _nu_Perror
#define printb              _nu_printb
#define offtime             _nu_offtime
#define psys_netstat            _nu_psys_netstat
#define psys_route          _nu_psys_route
#define psys_cdmount            _nu_psys_cdmount
            
#define psys_cmdtab         _nu_psys_cmdtab
#define psys_ncmds          _nu_psys_ncmds
#define cmds                _nu_cmds
            
#define user_apptab         _nu_user_apptab
#define user_cmdtab         _nu_user_cmdtab
#define statfcn             _nu_statfcn
#define sortfcn             _nu_sortfcn
#define printfcn            _nu_printfcn
#define psys_mntaddr            _nu_psys_mntaddr
#define psys_mntdir         _nu_psys_mntdir
            
            
/* Routed */
#define timevaladd          _nu_timevaladd
#define rtlookup            _nu_rtlookup
#define rtfind              _nu_rtfind
#define rtadd               _nu_rtadd
#define rtchange            _nu_rtchange
#define rtdelete            _nu_rtdelete
#define rtdeleteall         _nu_rtdeleteall
#define rtdefault           _nu_rtdefault
#define rtinit              _nu_rtinit
#define ifinit              _nu_ifinit
#define addrouteforif           _nu_addrouteforif
#define gwkludge            _nu_gwkludge
#define toall               _nu_toall
#define rsendmsg            _nu_rsendmsg
#define supply              _nu_supply
#define rip_input           _nu_rip_input
#define inet_makeaddr           _nu_inet_makeaddr
#define inet_netof          _nu_inet_netof
#define inet_rtflags            _nu_inet_rtflags
#define inet_sendroute          _nu_inet_sendroute
#define if_ifwithaddr           _nu_if_ifwithaddr
#define if_ifwithdstaddr        _nu_if_ifwithdstaddr
#define if_ifwithnet            _nu_if_ifwithnet
#define if_iflookup         _nu_if_iflookup
#define timer           _nu_timer
#define insque          _nu_insque
#define remque          _nu_remque
#define syslog          _nu_syslog
#define rperror         _nu_rperror
#define bcopy           _nu_bcopy
#define routed_bzero        _nu_routed_bzero
#define bcmp            _nu_bcmp
#define rmalloc         _nu_rmalloc
#define rfree           _nu_rfree
#define rexit           _nu_rexit
#define random          _nu_random
#define srandom         _nu_srandom
#define gettimeofday        _nu_gettimeofday
#define rstrcmp         _nu_rstrcmp
#define getpid          _nu_getpid
#define print           _nu_print
            
#define msg         _nu_msg
#define afswitch        _nu_afswitch
#define af_max          _nu_af_max
#define inet_default        _nu_inet_default
            
#define s           _nu_s
#define sid         _nu_sid
#define now         _nu_now
#define lastbcast       _nu_lastbcast
#define lastfullupdate      _nu_lastfullupdate
#define nextbcast       _nu_nextbcast
#define needupdate      _nu_needupdate
#define timerupdate     _nu_timerupdate
#define packet          _nu_packet
#define routed_syslog       _nu_routed_syslog
#define supplier        _nu_supplier
#define nethash         _nu_nethash
#define hosthash        _nu_hosthash
#define ifnet           _nu_ifnet
#define lookforinterfaces   _nu_lookforinterfaces
#define faketime        _nu_faketime
#define modify_route        _nu_modify_route
            
            
/* TFTP */
#define log_msg         _nu_log_msg
#define nak         _nu_nak
#define rw_allocate     _nu_rw_allocate
#define rw_deallocate       _nu_rw_deallocate
#define rw_init         _nu_rw_init
#define w_init          _nu_w_init
#define r_init          _nu_r_init
#define readit          _nu_readit
#define read_ahead      _nu_read_ahead
#define writeit         _nu_writeit
#define write_behind        _nu_write_behind
#define synchnet        _nu_synchnet

#define TFTPD_SYSSTACK      _nu_TFTPD_SYSSTACK
#define TFTPD_MODE      _nu_TFTPD_MODE
            
#define formats         _nu_formats
#define errmsgs         _nu_errmsgs
            
#define log_fid         _nu_log_fid
#define log_enabled     _nu_log_enabled
            
/* Telnet */
#define tn_init             _nu_tn_init
#define telnet_exit         _nu_telnet_exit
#define tn_set_binmode          _nu_tn_set_binmode
#define tn_gets             _nu_tn_gets
#define tn_init_telnet          _nu_tn_init_telnet
#define tn_willoption           _nu_tn_willoption
#define tn_wontoption           _nu_tn_wontoption
#define telsnd              _nu_telsnd
#define Scheduler           _nu_Scheduler
#define telnet              _nu_telnet
#define tn_xmitAO           _nu_tn_xmitAO
#define tn_xmitEL           _nu_tn_xmitEL
#define tn_xmitEC           _nu_tn_xmitEC
#define tn_dosynch          _nu_tn_dosynch
#define tn_intp             _nu_tn_intp
#define tn_sendbrk          _nu_tn_sendbrk
#define tn_ring_init            _nu_tn_ring_init
#define tn_ring_mark            _nu_tn_ring_mark
#define tn_ring_at_mark         _nu_tn_ring_at_mark
#define tn_ring_clear_mark      _nu_tn_ring_clear_mark
#define tn_ring_supplied        _nu_tn_ring_supplied
#define tn_ring_consumed        _nu_tn_ring_consumed
#define tn_ring_empty_count     _nu_tn_ring_empty_count
#define tn_ring_empty_consecutive   _nu_tn_ring_empty_consecutive
#define tn_ring_full_count      _nu_tn_ring_full_count
#define tn_ring_full_consecutive    _nu_tn_ring_full_consecutive
#define tn_ring_supply_data     _nu_tn_ring_supply_data
#define tn_ring_consume_data        _nu_tn_ring_consume_data
#define tn_init_network         _nu_tn_init_network
#define tn_stilloob         _nu_tn_stilloob
#define tn_setneturg            _nu_tn_setneturg
#define tn_netflush         _nu_tn_netflush
#define tn_quit             _nu_tn_quit
#define tn              _nu_tn
#define tn_command          _nu_tn_command
#define tn_upcase           _nu_tn_upcase
#define tn_SetSockOpt           _nu_tn_SetSockOpt
#define tn_Dump             _nu_tn_Dump
#define tn_printoption          _nu_tn_printoption
#define tn_printsub         _nu_tn_printsub
#define tn_init_sys         _nu_tn_init_sys
#define TerminalWrite           _nu_TerminalWrite
#define TerminalRead            _nu_TerminalRead
#define TerminalAutoFlush       _nu_TerminalAutoFlush
#define TerminalSpecialChars        _nu_TerminalSpecialChars
#define TerminalFlushOutput     _nu_TerminalFlushOutput
#define TerminalSaveState       _nu_TerminalSaveState
#define TerminalRestoreState        _nu_TerminalRestoreState
#define TerminalNewMode         _nu_TerminalNewMode
#define tn_NetClose         _nu_tn_NetClose
#define tn_sys_init         _nu_tn_sys_init
#define tn_process_rings        _nu_tn_process_rings
#define tn_init_terminal        _nu_tn_init_terminal
#define tn_ttyflush         _nu_tn_ttyflush
#define tn_getconnmode          _nu_tn_getconnmode
#define tn_setconnmode          _nu_tn_setconnmode
#define tn_setcommandmode       _nu_tn_setcommandmode
            
#define tn_doopt            _nu_tn_doopt
#define tn_dont             _nu_tn_dont
#define tn_will             _nu_tn_will
#define tn_wont             _nu_tn_wont
#define tn_connected            _nu_tn_connected
#define tn_debug            _nu_tn_debug
#define tn_crmod            _nu_tn_crmod
#define tn_netdata          _nu_tn_netdata
#define tn_crlf             _nu_tn_crlf
#define tn_telnetport           _nu_tn_telnetport
#define tn_SYNCHing         _nu_tn_SYNCHing
#define tn_flushout         _nu_tn_flushout
#define tn_autoflush            _nu_tn_autoflush
#define tn_autosynch            _nu_tn_autosynch
#define tn_localchars           _nu_tn_localchars
#define tn_donelclchars         _nu_tn_donelclchars
#define tn_donebinarytoggle     _nu_tn_donebinarytoggle
#define tn_dontlecho            _nu_tn_dontlecho
#define tn_globalmode           _nu_tn_globalmode
#define toplevel            _nu_toplevel
#define tn_modelist         _nu_tn_modelist
#define tn_NetTrace         _nu_tn_NetTrace
#define tn_opts             _nu_tn_opts
#define tn_term_name        _nu_tn_term_name
#define tn_exit_param       _nu_tn_exit_param
#define tn_close_flag       _nu_tn_close_flag
#define tn_hisopts      _nu_tn_hisopts
#define tn_myopts       _nu_tn_myopts
#define tn_prompt       _nu_tn_prompt
#define tn_escape       _nu_tn_escape
#define tn_echoc        _nu_tn_echoc
#define peerdied        _nu_peerdied
#define tn_clocks       _nu_tn_clocks
#define tn_netoring     _nu_tn_netoring
#define tn_netiring     _nu_tn_netiring
#define tn_netobuf      _nu_tn_netobuf
#define tn_netibuf      _nu_tn_netibuf
#define tn_hostname     _nu_tn_hostname
#define tn_showoptions      _nu_tn_showoptions
#define tn_tin          _nu_tn_tin
#define tn_net          _nu_tn_net
#define tn_HaveInput        _nu_tn_HaveInput
#define tn_ttyoring     _nu_tn_ttyoring
#define tn_ttyiring     _nu_tn_ttyiring
#define tn_ttyobuf      _nu_tn_ttyobuf
#define tn_ttyibuf      _nu_tn_ttyibuf
#define tn_termEofChar      _nu_tn_termEofChar
#define tn_termEraseChar    _nu_tn_termEraseChar
#define tn_termFlushChar    _nu_tn_termFlushChar
#define tn_termIntChar      _nu_tn_termIntChar
#define tn_termKillChar     _nu_tn_termKillChar
#define tn_termQuitChar     _nu_tn_termQuitChar

/* Telnetd */

/* NFS daemon */
#ifdef NFSDDIR
#define xdr_nfsstat_e           _nu_xdr_nfsstat_e
#define xdr_nfsftype_e              _nu_xdr_nfsftype_e
#define xdr_nfsfhandle_t        _nu_xdr_nfsfhandle_t
#define xdr_nfstimeval_t            _nu_xdr_nfstimeval_t
#define xdr_nfsfattr_t          _nu_xdr_nfsfattr_t
#define xdr_nfssattr_t          _nu_xdr_nfssattr_t
#define xdr_nfspath_t           _nu_xdr_nfspath_t
#define xdr_nfsrddata_t         _nu_xdr_nfsrddata_t
#define xdr_nfswrdata_t         _nu_xdr_nfswrdata_t
#define xdr_nfsattrstat_u       _nu_xdr_nfsattrstat_u
#define xdr_nfsfhandle_fattr_t      _nu_xdr_nfsfhandle_fattr_t
#define xdr_nfsdiropargs_t      _nu_xdr_nfsdiropargs_t
#define xdr_nfsdiropres_u       _nu_xdr_nfsdiropres_u
#define xdr_nfsreadlinkres_u        _nu_xdr_nfsreadlinkres_u
#define xdr_nfsreadres_t        _nu_xdr_nfsreadres_t
#define xdr_nfsreadres_u        _nu_xdr_nfsreadres_u
#define xdr_nfscookie_t         _nu_xdr_nfscookie_t
#define xdr_nfsfsattr_t         _nu_xdr_nfsfsattr_t
#define xdr_nfsstatfsres_u      _nu_xdr_nfsstatfsres_u
#define xdr_nfsreaddirentry_t       _nu_xdr_nfsreaddirentry_t
#define xdr_nfsreaddirentry_u       _nu_xdr_nfsreaddirentry_u
#define xdr_nfsreaddirres_t     _nu_xdr_nfsreaddirres_t
#define xdr_nfsreaddirres_u     _nu_xdr_nfsreaddirres_u
#define xdr_nfssetattr_t        _nu_xdr_nfssetattr_t
#define xdr_nfsread_t           _nu_xdr_nfsread_t
#define xdr_nfswrite_t          _nu_xdr_nfswrite_t
#define xdr_nfscreate_t         _nu_xdr_nfscreate_t
#define xdr_nfsrename_t             _nu_xdr_nfsrename_t
#define xdr_nfslink_t           _nu_xdr_nfslink_t
#define xdr_nfssymlink_t            _nu_xdr_nfssymlink_t
#define xdr_nfsmkdir_t          _nu_xdr_nfsmkdir_t
#define xdr_nfsreaddir_t            _nu_xdr_nfsreaddir_t
#define nfsd_exit           _nu_nfsd_exit
#define nfsd_task                   _nu_nfsd_task
#define nfsprog_2           _nu_nfsprog_2
#define nfsd_terminate          _nu_nfsd_terminate
#define nfsproc_null_2              _nu_nfsproc_null_2
#define nfsproc_getattr_2       _nu_nfsproc_getattr_2
#define nfsproc_setattr_2           _nu_nfsproc_setattr_2
#define nfsproc_root_2          _nu_nfsproc_root_2
#define nfsproc_lookup_2            _nu_nfsproc_lookup_2
#define nfsproc_readlink_2      _nu_nfsproc_readlink_2
#define nfsproc_read_2          _nu_nfsproc_read_2
#define nfsproc_writecache_2        _nu_nfsproc_writecache_2
#define nfsproc_write_2             _nu_nfsproc_write_2
#define nfsproc_create_2        _nu_nfsproc_create_2
#define nfsproc_remove_2            _nu_nfsproc_remove_2
#define nfsproc_rename_2        _nu_nfsproc_rename_2
#define nfsproc_link_2              _nu_nfsproc_link_2
#define nfsproc_symlink_2       _nu_nfsproc_symlink_2
#define nfsproc_mkdir_2             _nu_nfsproc_mkdir_2
#define nfsproc_rmdir_2         _nu_nfsproc_rmdir_2
#define nfsproc_readdir_2           _nu_nfsproc_readdir_2
#define nfsproc_statfs_2        _nu_nfsproc_statfs_2
#define nfs_ExtractVolName          _nu_nfs_ExtractVolName
#define nfs_GetVolName          _nu_nfs_GetVolName
#define nfs_GetVolNum               _nu_nfs_GetVolNum
#define nfs_ChangeVolume        _nu_nfs_ChangeVolume
#define nfs_MapError                _nu_nfs_MapError
#define nfs_SanityCheck         _nu_nfs_SanityCheck
#define nfs_GetPathName             _nu_nfs_GetPathName
#define nfs_GetAttr         _nu_nfs_GetAttr
#define nfs_SetAttr                 _nu_nfs_SetAttr
#define nfs_GetEList            _nu_nfs_GetEList
#define nfs_GetNextHostAddr     _nu_nfs_GetNextHostAddr
#define nfs_GetUnixTime             _nu_nfs_GetUnixTime
#define xdr_mnt_fhandle_t       _nu_xdr_mnt_fhandle_t
#define xdr_mnt_fhstatus_t          _nu_xdr_mnt_fhstatus_t
#define xdr_mnt_dirpath_t       _nu_xdr_mnt_dirpath_t
#define xdr_mnt_name_t          _nu_xdr_mnt_name_t
#define xdr_mnt_mountentry_t        _nu_xdr_mnt_mountentry_t
#define xdr_mnt_mountlist_t     _nu_xdr_mnt_mountlist_t
#define xdr_mnt_grname_t            _nu_xdr_mnt_grname_t
#define xdr_mnt_groups_t        _nu_xdr_mnt_groups_t
#define xdr_mnt_exportentry_t       _nu_xdr_mnt_exportentry_t
#define xdr_mnt_exportlist_t        _nu_xdr_mnt_exportlist_t
#define mntd_task                   _nu_mntd_task
#define mntprog_1           _nu_mntprog_1
#define mntd_start                  _nu_mntd_start
#define mntd_eachresult         _nu_mntd_eachresult
#define mntd_terminate              _nu_mntd_terminate
#define mntproc_null_1          _nu_mntproc_null_1
#define mntproc_umntall_1           _nu_mntproc_umntall_1
#define mntproc_umnt_1          _nu_mntproc_umnt_1
#define mntproc_mnt_1               _nu_mntproc_mnt_1
#define mntproc_export_1        _nu_mntproc_export_1
#define mntproc_dump_1          _nu_mntproc_dump_1

#define gv_nfscfg           _nu_gv_nfscfg
#define gv_logbsize         _nu_gv_logbsize
#define gv_nfselist_cnt         _nu_gv_nfselist_cnt
#define gv_nfselist         _nu_gv_nfselist
#define nfs_rwbuf           _nu_nfs_rwbuf
#endif /* NFSDDIR */

#endif /* PNAUTILS */

#ifdef ONELIB
/* TODO */
#define sockcall    _nu_sockcall

struct sockcall {
    int pna;
    int (*close)();
    int (*socket)();
    int (*bind)();
    int (*select)();
    int (*recvfrom)();
    int (*sendto)();
    int (*ioctl)();
    int (*shutdown)();
    int (*connect)();
    int (*getsockname)();
    int (*setsockopt)();
    int (*send)();
    int (*recv)();
    int (*listen)();
    int (*accept)();
    int (*getpeername)();
    int (*shr_socket)();
    int (*get_id)();
    int (*set_id)();
};

extern struct sockcall *sockcall;

/* TODO - Redefine socket calls */
#define close           (*sockcall->close)
#define socket          (*sockcall->socket)
#define bind            (*sockcall->bind)
#define select          (*sockcall->select)
#define recvfrom        (*sockcall->recvfrom)
#define sendto          (*sockcall->sendto)
#define ioctl           (*sockcall->ioctl)
#define shutdown        (*sockcall->shutdown)
#define connect         (*sockcall->connect)
#define getsockname     (*sockcall->getsockname)
#define setsockopt      (*sockcall->setsockopt)
#define send            (*sockcall->send)
#define recv            (*sockcall->recv)
#define listen          (*sockcall->listen)
#define accept          (*sockcall->accept)
#define getpeername     (*sockcall->getpeername)
#define shr_socket      (*sockcall->shr_socket)
#define get_id          (*sockcall->get_id)
#define set_id          (*sockcall->set_id)



#define sockerr     _nu_sockerr
#define sockerrno   _nu_sockerrno

struct sockerrno {
    int eintr;
    int epipe;
    int eaddrinuse;
    int ewouldblock;
    int eexist;
    int enetunreach;
    int enobufs;
    int enxio;
    int eaddrnotavail;
    int eafnosupport;
};

extern struct sockerrno *sockerrno;

/* TODO - Redefine errno's */
#define EINTR           (sockerrno->eintr)
#define EPIPE           (sockerrno->epipe)
#define EADDRINUSE      (sockerrno->eaddrinuse)
#define EWOULDBLOCK     (sockerrno->ewouldblock)
#define EEXIST          (sockerrno->eexist)
#define ENETUNREACH     (sockerrno->enetunreach)
#define ENOBUFS         (sockerrno->enobufs)
#define ENXIO           (sockerrno->enxio)
#define EADDRNOTAVAIL   (sockerrno->eaddrnotavail)
#define EAFNOSUPPORT    (sockerrno->eafnosupport)
#endif /* ONELIB */

#ifdef ONELIB
#define IS_PNA      sockcall->pna
#else /* ONELIB */
#ifdef PNAUTILS
#define rtreq       rtentry
#define IS_PNA      1
#else
#define IS_PNA      0
#endif /* PNAUTILS */
#endif /* ONELIB */

/*	2001.10.28, William, mask-off, EINTR is defined in 
 *				errno.h in VxWorks; the value is 4 not 
 *				0x50505050. To avoid misusage, mask-off
 *				Jason's definition.
 * Jason Chen, 09-27-2000
 *	#define EINTR           0x50505050
 */
#ifndef	EINTR
#define	EINTR	4
#endif	/*	#ifndef EINTR	*/

#endif /* _nudefs_h */
