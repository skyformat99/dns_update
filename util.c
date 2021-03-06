#include "util.h"

char log_file[100];
char server[100];
int  cmd_port;
char auth_key[100];





int udp_create()
{
    return socket( AF_INET, SOCK_DGRAM, 0 );
}

int udp_listen( char* ip, int port )
{
/* ip 表示绑定在哪个ip 上，如果为NULL,表示绑定在所有ip 上，
如果不为null,则绑定在指定ip地址上: 
函数调用成功返回 socket fd, 否则返回 -1  */

    int fd;
    struct sockaddr_in addr;

    fd = socket( AF_INET, SOCK_DGRAM, 0 );
    if( fd == -1 )
    {
        return -1;
    }

    int reuse=1;
    setsockopt( fd, SOL_SOCKET, SO_REUSEADDR, (char*)&reuse, sizeof(reuse) );
    
    addr.sin_family = AF_INET;
    addr.sin_port   = htons( port );

    if(ip == NULL || ip[0] == '*' )
    {
        addr.sin_addr.s_addr = INADDR_ANY;
        if( bind( fd, (struct sockaddr*)&addr, sizeof(addr) ) == -1 )
        {
            close( fd );
            return -1;
        }
    }
    else /*绑定ip  */
    {
        addr.sin_addr.s_addr = inet_addr(ip);
        if( bind( fd, (struct sockaddr*)&addr, sizeof(addr) ) == -1 )
        {
            close( fd );
            return -1;
        }
    }

    return fd;

}


int udp_send( int fd, char* ip, int port, char* msg, int len )
{
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port   = htons( port );
    addr.sin_addr.s_addr = inet_addr(ip);
    
    return sendto( fd, msg, len, 0, (struct sockaddr*)&addr, sizeof(addr) );
}


int udp_recv( int fd, char* buf, int size, char* ip, int* port )
{
    struct sockaddr_in addr;
    socklen_t len;
    int rc;
    
    len = sizeof(addr);
    
    rc = recvfrom( fd, buf, size, 0, (struct sockaddr *)&addr, &len );
    if( rc > 0 )
    {
        *port=ntohs(addr.sin_port);
        sprintf(ip,"%s",inet_ntoa(addr.sin_addr));
    }
    
    return rc;
}

int cfg_get_value( const char* file, const char* section, const char* key, char* value )
{
    char line[1024],cursec[1024],tmp[1024];
    char *p;
    FILE* f;

    f=fopen(file,"r");
    if(f==NULL)
        return -1;
    cursec[0]=0;
    
    while( fgets( line, sizeof(line), f ) )
    {
        if( line[0] == '[' ) /* section */
        {
            p = strchr(line,']');
            if( p == NULL)
            {
                continue;    
            }
            p[0] = 0;
            strcpy( cursec, line+1 );
        }
        else if (line[0]=='#') /* commnet */
            continue;
        else
        {
            if( strcmp( section, cursec ) != 0 )
                continue;
            
            while(line[strlen(line)-1]== ' ' || line[strlen(line)-1]== '\t' || line[strlen(line)-1]== '\r' || line[strlen(line)-1]== '\n')
                line[strlen(line)-1]= 0;
                
            while( line[0]==' ' || line[0]=='\t' ) /* 去掉头部的空格 */
            {
                memmove( line, line+1, strlen(line)-1 );
                line[strlen(line)-1]=0;
            }
            
            if( strlen(line) == 0 )
                continue;
            
            strcpy( tmp, line );
            p = strchr( tmp, '=' );
            if( p == NULL )
                continue;
            p[0] = 0;
            
            while( tmp[strlen(tmp)-1] == ' ' || tmp[strlen(tmp)-1] == '\t')
                tmp[strlen(tmp)-1]=0;
            if( strcmp(tmp,key) != 0 )
                continue;
            
            /* 处理值 */
            p = strchr( line, '=' );
            strcpy( tmp, p+1 );
            
            while( tmp[0] == ' ' || tmp[0] == '\t') /* 去掉头部的空格 */
            {
                memmove( tmp, tmp+1, strlen(tmp)-1 );
                tmp[strlen(tmp)-1]=0;
            }
            
            if( tmp[0] == '"' )
            {
                memmove( tmp, tmp+1, strlen(tmp)-1 );
                tmp[strlen(tmp)-1]=0;
            }
            
            if( tmp[strlen(tmp)-1] == '"' )
            {
                    tmp[strlen(tmp)-1]=0;
            }
            /*
            if(strlen(tmp)==0)
            {
                fclose(f);
                return -1;
            }
            */
            strcpy( value, tmp );
            fclose( f );
            return 0;
        }
    }
    
    fclose(f);
    
    return -1;
}

int resolve_host_ip( const char* host, char* ip )
{
    struct hostent* ent;
    char* addr;
    unsigned char b1,b2,b3,b4;

     ent = gethostbyname( host );
    if( ent == NULL )
        return -1;
    addr = *ent->h_addr_list;
    if( !addr )
        return -1;
    b1 = addr[0];
    b2 = addr[1];
    b3 = addr[2];
    b4 = addr[3];

    sprintf( ip, "%d.%d.%d.%d", b1, b2, b3, b4 );
    return 0;
}


int write_log( const char* fmt, ... )
{
    FILE* f;
    time_t t;
    struct tm* tm;
    va_list  ap;
    //struct timeval tmv;

    f = fopen( log_file, "a" );
    if( f == NULL )
        return -1;
    
    t = time( 0 );
    //gettimeofday( &tmv, NULL );
    
    tm = localtime( &t );
    
    if( *fmt )
    {
        va_start( ap, fmt );
        //fprintf(f,"[%02d-%02d %02d:%02d:%02d.%03d]  ",tm->tm_mon+1,tm->tm_mday,tm->tm_hour,tm->tm_min,tm->tm_sec,tmv.tv_usec / 1000 );
        fprintf( f, "[%02d-%02d %02d:%02d:%02d]  ", tm->tm_mon+1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec );
        vfprintf( f, fmt, ap);
        fprintf( f, "\n" );
        va_end( ap );
    }
    
    fclose( f );
    return 0;
}

int write_log_hex( char* buf, int buflen )
{
  FILE *fp;
  int i,g,k;
  unsigned char *abuf,*hbuf;


  if ( ( fp = fopen(log_file,"a+")) == NULL) return(-1);

  hbuf = (unsigned char *)buf;
  abuf = (unsigned char *)buf;
  fprintf( fp, "  *** hex message(begin) ***\n" );
  fprintf( fp, "-----------|--|--|--|--|--|--|--|--|--|--|--|--|--|--|--|--|------------------\n" );
  for( i = 0, g = buflen/16; i < g; i++ )
  {
    fprintf( fp, "M(%6.6d)=< ", i*16 );
    for( k = 0; k < 16; k ++) fprintf( fp, "%02x ", *hbuf++ );
        fprintf( fp, "> " );
    for( k = 0; k < 16; k++, abuf++)
        fprintf( fp, "%c", (*abuf>32) ? ((*abuf<128) ? *abuf : '*') : '.');
    fprintf( fp, "\n" );
  }
  if( (i = buflen%16) > 0)
  {
    fprintf( fp, "M(%6.6d)=< ", buflen-buflen%16 );
    for( k = 0; k < i;  k++) fprintf( fp, "%02x ", *hbuf++);
    for( k = i; k < 16; k++) fprintf( fp, "   ");
        fprintf( fp, "> ");
    for( k = 0; k < i; k++, abuf++)
        fprintf( fp, "%c",(*abuf>32) ? ((*abuf<128) ? *abuf : '*') : '.');
    fprintf( fp, "\n");
  }
  fprintf( fp, "-----------|--|--|--|--|--|--|--|--|--|--|--|--|--|--|--|--|------------------\n");
  fprintf( fp, "  *** hex message(end) ***\n");
  fflush( fp );
  fclose( fp );
  return(0);
}

int load_config()
{
    char tmp[512];
    
    if( cfg_get_value( "dns.cfg", "dns", "port", tmp ))
    {
        printf( "load config [dns][port] failed!\n" );
        return 1;
    }
    cmd_port = atoi( tmp );
    
    
    if( cfg_get_value( "dns.cfg", "dns", "auth_key", tmp ))
    {
        printf( "load config [dns][auth_key] failed!\n" );
        return 1;
    }
    snprintf( auth_key, sizeof(auth_key), "%s", tmp );
    
    
    return 0;
    
}

int hex2asc( char* hex, int len, char* buf, int buflen )
{
    char bits[] = "0123456789abcdef";
    
    unsigned char u;
    int i;

    if( buflen < len * 2 )
        return -1;
    buf[len*2]=0;
    
    for( i = 0; i < len; i++ )
    {
        u = hex[i];
        
        buf[i*2+0] = bits[ u >> 4 ];
        buf[i*2+1] = bits[ u & 0x0f ];
    }
    
    return 0;
    
}


char *strlower( char *s )
{
    int i, len = strlen(s);
    for( i = 0; i < len; i++ ){
        s[i] = ( s[i] >= 'A' && s[i] <= 'Z' ? s[i] +32 : s[i] );
    }
    return s;
}



