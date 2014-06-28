

int aimsock;
int sequence;
bool aim_quit=false;
int dsock,isock;
bool in_chat=false;
int seq4;


static char * serverErrorList(int num)
{
   if(num == -1){return "error[gethostbyaddr failed]\n";}
   if(num == -2){return "error[creating socket failed]\n";}
   if(num == -3){return "error[establishing connection failed]\n";}
}
static int Connect2Server(char *address, int port)
{
   int outsock;
   struct sockaddr_in server;
   int error,bytes;
   struct hostent *he;
   he = gethostbyname(address);
   if(!he){return -1;}
   outsock = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
   if(outsock < -1){return -2;}
   memset((char *)&server,0,sizeof(server));
   server.sin_family = AF_INET;
   server.sin_port = htons(port);
   server.sin_addr.s_addr = * (u_long *)(he->h_addr);
   error = connect(outsock,(const struct sockaddr *)&server,sizeof(server));
   if(error < 0){return -3;}
   return outsock;
}
char * ToHex(char *data, int length)
{
     static char denulled[10000];
     char smallbuffer[3];
     memset((char*)&smallbuffer,0,sizeof(smallbuffer));
     memset((char*)&denulled,0,sizeof(denulled));
     char buffer[200];
     int i;
     for(i=0; i <= length;i++)
     {
      if(data[i] == 0x0){data[i] == '0';}
      sprintf(smallbuffer,"%x ",data[i]);
      if(smallbuffer[1] == 0x00){strcat(denulled,"0");}
      strcat(denulled,smallbuffer);
      memset((char*)&smallbuffer,0,sizeof(smallbuffer));
     }
     return denulled;
}
char * FormatHex(char *string)
{
    static char output[500];
    int a=0;
    /* Get rid of too many ffffff and no single hex digits*/
    int i;
    for(i=0; i < strlen(string);i++,a++)
    {
        if(string[i] == 'f' && string[i+1] == 'f' && string[i+2] == 'f'){i+=6;}
        if(string[i] == 0x20 && string[i+1] > 0x20 && string[i+2] == 0x20){output[a++] = 0x20;output[a] = 0x30;continue;}
        output[a] = string[i];
    }
    output[a] = '\0';
    return output;
}
unsigned int hexToInt(char* hexStr)
{
   unsigned int hexInt;
   sscanf( hexStr, "%x", &hexInt);
   return hexInt;
}
char * CreateCharacters(char *string,int len)
{
    if(len==0){len=strlen(string);}
    char sb[2];
    static char buffer[50];
    int a=0,i=0;
    for(i=0; i < strlen(string);i++,a++)
    {
        sb[0] = string[i];
        sb[1] = string[i+1];
        buffer[a] = hexToInt(sb);
        i=i+2;
    }
    buffer[a] = '\0';
    return buffer;
}
char * RoastOscar(char *string)
{
char key[16];
sprintf(key,"%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c",0xF3,0xb3,0x6c,0x99,0x95,0x3f,0xac,0xb6,0xc5,0xfa,0x6b,0x63,0x69,0x6c,0xc3,0x9a);
int string_len = strlen(string);
int key_length = 16;

   int i,position;
   char data[200];

   for(i = 0; i < string_len; i++)
   {
    data[i] = (char)((int)string[i] ^ (int)key[i]);
   }
   return CreateCharacters(FormatHex(ToHex(data,string_len-1)),string_len);
}
char * LoginErrorList(int error)
{
                if(error == 0x01){return "login[Invalid nick or password]";}
                if(error == 0x02){return "login[Service temporarily unavailable]";}
                if(error == 0x03){return "login[Some Weird Error]";}
                if(error == 0x04){return "login[Incorrect password]";}
                if(error == 0x05){return "login[Mismatch password]";}
                if(error == 0x06){return "login[Internal client error]";}
                if(error == 0x07){return "login[Invalid account]";}
                if(error == 0x08){return "login[Deleted account]";}
                if(error == 0x09){return "login[Expired account]";}
                if(error == 0x0A){return "login[No access to database]";}
                if(error == 0x0B){return "login[No access to resolver]";}
                if(error == 0x0C){return "login[Invalid database fields]";}
                if(error == 0x0D){return "login[Bad database status]";}
                if(error == 0x0E){return "login[Bad resolver status]";}
                if(error == 0x0F){return "login[Internal error]";}
                if(error == 0x10){return "login[Service temporarily offline]";}
                if(error == 0x11){return "login[Suspended account]";}
                if(error == 0x12){return "login[DB send error]";}
                if(error == 0x13){return "login[DB link error]";}
                if(error == 0x14){return "login[Reservation map error]";}
                if(error == 0x15){return "login[Reservation link error]";}
                if(error == 0x16){return "login[IP connections reached max]";}
                if(error == 0x17){return "login[IP connections reached max]";}
                if(error == 0x18){return "login[Rate limit exceeded]";}
                if(error == 0x19){return "login[User too heavily warned]";}
                if(error == 0x1A){return "login[Reservation timeout]";}
                if(error == 0x1B){return "login[Upgrade required]";}
                if(error == 0x1C){return "login[Upgrade recommended]";}
                if(error == 0x1D){return "login[Rate limit exceeded]";}
                if(error == 0x1E){return "login[Reconnect in a few minutes]";}
                if(error == 0x20){return "login[Invalid SecurID]";}
                if(error == 0x22){return "login[Account suspended(AGE < 13)]";}
                else{return "Login: Unknown Error";}
}
static char *Login2AIM(int oscar_sock, char *screenname, char *password)
{
   char rbuff[10000],sbuff[10000];
   char cookies[0x100],ip[20],sn[500],screenname_user[500];
   int bytes;
   sequence=0x1c;
   fWriteColorInTextView("waiting for aim server initial ack..","fg_red");
   /* NonBlocking socket */
   u_long arg=1;ioctl(oscar_sock, FIONBIO, &arg);
   /* Login */
   while(oscar_sock)
   {
      while(g_main_context_iteration(NULL,FALSE));//update-ui
      bytes = recv(oscar_sock,rbuff,10000,0);
      if(bytes < 0 && errno != EWOULDBLOCK){return "socket error[oscar_sock socket returned -1]";}
      if(bytes > 0)
      {
         /* Receive Server ACK */
         if(rbuff[4] == 0x00 && rbuff[5] == 0x04 && rbuff[6] == 0x00 && rbuff[7] == 0x00 && rbuff[8] == 0x00 && rbuff[9] == 0x01)
         {
            /* Send back User Login request */
            fWriteColorInTextView("..done\n","fg_blue");
            fWriteColorInTextView("sending encrypted password and screen name..","fg_red");
            char aim_id_string[]="";
            char *roasted = RoastOscar(password);
            char login_packet_one[14]={0x2A,0x01,0x00,0x01,0x00,0x47 + strlen(screenname) + strlen(password) + strlen(aim_id_string),0x00,0x00,0x00,0x01,0x00,0x01,0x00,strlen(screenname)};
            char login_packet_two[4]={0x00,0x02,0x00,strlen(password)};
            char login_packet_three[4]={0x00,0x03,0x00,strlen(aim_id_string)};
            char login_packet_four[55]={0x00,0x16,0x00,0x02,0x01,0x09,0x00,0x17,0x00,0x02,0x00,0x05,0x00,0x18,0x00,0x02,0x00,0x09,0x00,0x19,0x00,0x02,0x00,0x00,0x00,0x1A,0x00,0x02,0x17,0xC2,0x00,0x14,0x00,0x04,0x00,0x00,0x01,0x50,0x00,0x0F,0x00,0x02,0x65,0x6E,0x00,0x0E,0x00,0x02,0x75,0x73,0x00,0x4A,0x00,0x01,0x01};
            memset((char*)&sbuff,0,sizeof(sbuff));
            memcpy(sbuff,login_packet_one,14);
            memcpy(sbuff+14,screenname,strlen(screenname));
            memcpy(sbuff+14+strlen(screenname),login_packet_two,4);
            memcpy(sbuff+18+strlen(screenname),roasted,strlen(password));
            memcpy(sbuff+18+strlen(screenname)+strlen(password),login_packet_three,4);
            memcpy(sbuff+22+strlen(screenname)+strlen(password),aim_id_string,strlen(aim_id_string));
            memcpy(sbuff+22+strlen(screenname)+strlen(password)+strlen(aim_id_string),login_packet_four,55);
            bytes = send(oscar_sock,sbuff,77+strlen(screenname)+strlen(password)+strlen(aim_id_string),0);
         }
         /* Receive server cookies */
         if(rbuff[1] == 0x04)
         {
            /* Error packet found */
            if(rbuff[13+rbuff[9]+rbuff[13+rbuff[9]]+1] == 0x00 && rbuff[13+rbuff[9]+rbuff[13+rbuff[9]]+2] == 0x08)
            {
                int error = rbuff[13+rbuff[9]+rbuff[13+rbuff[9]]+6];
                char aim_error[1000];
                memset((char*)&aim_error,0,sizeof(aim_error));
                memcpy(aim_error,rbuff+14+rbuff[9],rbuff[13+rbuff[9]]); //tell you more about error
                return(LoginErrorList(error));//Error code sent

            }
            else if(rbuff[9+rbuff[9]+1] == 0x00 && rbuff[9+rbuff[9]+2] == 0x08)
            {
                    int error = rbuff[9+rbuff[9]+6];
                    return(LoginErrorList(error));//Error code sent
            }
            /* Received Message Cookie */
            else
            {
               /* Extract BOS IP/Cookie */
               fWriteColorInTextView("..done\n","fg_blue");
               memset((char*)&sn,0,sizeof(sn));
               memset((char*)&ip,0,sizeof(ip));
               memset((char*)&cookies,0,sizeof(cookies));
               memcpy(sn,rbuff+15,rbuff[14]);
               memcpy(screenname_user,rbuff+15,rbuff[14]);
               memcpy(ip,rbuff+19+strlen(sn),rbuff[18+strlen(sn)]-5);
               memcpy(cookies,rbuff+28+strlen(sn)+strlen(ip),0x100);
               sprintf(sbuff,"%s:5190\n",ip);
               fWriteColorInTextView("extracted aim server ip:port..","fg_red");
               fWriteColorInTextView(sbuff,"fg_blue");
            }
            close(oscar_sock);
            oscar_sock = 0;
         }
      }
   }

  char client_service_version[]={0x2A,0x02,0x00,0x0f,0x00,0x32,0x00,0x01,0x00,0x17,0x00,0x00,0x00,0x00,0x00,0x17,0x00,0x01,0x00,0x04,0x00,0x13,0x00,0x03,0x00,0x02,0x00,0x01,0x00,0x03,0x00,0x01,0x00,0x04,0x00,0x01,0x00,0x06,0x00,0x01,0x00,0x08,0x00,0x01,0x00,0x09,0x00,0x01,0x00,0x0A,0x00,0x01,0x00,0x0B,0x00,0x01};
  char client_rate_limit[]=     {0x2A,0x02,0x00,0x10,0x00,0x0A,0x00,0x01,0x00,0x06,0x00,0x00,0x00,0x00,0x00,0x06};


  char client_services[134]=    {0x2A,0x02,0x00,0x11,0x00,0x0A,0x00,0x13,0x00,0x07,0x00,0x00,0x00,0x00,0x00,0x07
                                ,0x2A,0x02,0x00,0x12,0x00,0x0A,0x00,0x01,0x00,0x0E,0x00,0x00,0x00,0x00,0x00,0x0E
                                ,0x2A,0x02,0x00,0x13,0x00,0x0A,0x00,0x13,0x00,0x02,0x00,0x00,0x00,0x00,0x00,0x02
                                ,0x2A,0x02,0x00,0x14,0x00,0x10,0x00,0x13,0x00,0x05,0x00,0x00,0x00,0x20,0x00,0x05,0x52,0x35,0x4A,0x2E,0x00,0x13
                                ,0x2A,0x02,0x00,0x15,0x00,0x0A,0x00,0x02,0x00,0x02,0x00,0x00,0x00,0x00,0x00,0x02
                                ,0x2A,0x02,0x00,0x16,0x00,0x0A,0x00,0x03,0x00,0x02,0x00,0x00,0x00,0x00,0x00,0x02
                                ,0x2A,0x02,0x00,0x17,0x00,0x0A,0x00,0x04,0x00,0x04,0x00,0x00,0x00,0x00,0x00,0x04
                                ,0x2A,0x02,0x00,0x18,0x00,0x0A,0x00,0x09,0x00,0x02,0x00,0x00,0x00,0x00,0x00,0x02};

  char set_icmb_params[32]=     {0x2A,0x02,0x00,0x19,0x00,0x1A
                                ,0x00,0x04,0x00,0x02
                                ,0x00,0x00
                                ,0x00,0x00,0x00,0x02

                                ,0x00,0x00 //channel setup
                                ,0x00,0x00,0x00,0x0B //msg flag
                                ,0x1F,0x40 //max message snac size
                                ,0x03,0xE7 //max sender warning level
                                ,0x03,0xE7 //max receiver warning level
                                ,0x00,0x00 //minimum message interval (sec)
                                ,0x00,0x00 //unknown parameter (also seen 03 E8)
                                };





char profilestring[]="";

  char set_user_info_1[57] =     {0x2A,0x02,0x00,0x1a,0x0,0xb7+strlen(profilestring),
                                 0x0,0x2,0x0,0x4,
                                 0x0,0x0,
                                 0x0,0x0,0x0,0x0,
                                 0x0,0x1, //MIMI type
                                 0x0,0x21,
                                 0x74,0x65,0x78,0x74,0x2F,0x78,0x2D,0x61,0x6F,0x6C,0x72,0x74,0x66,0x3B,0x20,0x63,0x68,0x61,0x72,0x73,0x65,0x74,0x3D,0x22,0x75,0x73,0x2D,0x61,0x73,0x63,0x69,0x69,0x22,
                                 0x0,0x2, //profile string
                                 0x0,strlen(profilestring)};

  char set_user_info_2[132] =    {0x0,0x5, //capibilities
                                 0x0,0x80,
                                 0x9,0x46,0x13,0x41,0x4C,0x7F,0x11,0xD1,0x82,0x22,0x44,0x45,0x53,0x54,0x0,0x0,0x9,0x46,0x13,0x43,0x4C,0x7F,0x11,0xD1,0x82,0x22,0x44,0x45,0x53,0x54,0x0,0x0,0x74,0x8F,0x24,0x20,0x62,0x87,0x11,0xD1,0x82,0x22,0x44,0x45,0x53,0x54,0x0,0x0,0x9,0x46,0x13,0x45,0x4C,0x7F,0x11,0xD1,0x82,0x22,0x44,0x45,0x53,0x54,0x0,0x0,0x9,0x46,0x13,0x46,0x4C,0x7F,0x11,0xD1,0x82,0x22,0x44,0x45,0x53,0x54,0x0,0x0,0x9,0x46,0x13,0x47,0x4C,0x7F,0x11,0xD1,0x82,0x22,0x44,0x45,0x53,0x54,0x0,0x0,0x9,0x46,0x13,0x48,0x4C,0x7F,0x11,0xD1,0x82,0x22,0x44,0x45,0x53,0x54,0x0,0x0,0x9,0x46,0x13,0x48,0x4C,0x7F,0x11,0xD1,0x82,0x22,0x44,0x45,0x53,0x54,0x0,0x0};

  /*char client_ready[96]=             {0x2A,0x02,0x00,0x1a,0x00,0x5A
                                     ,0x00,0x01,0x00,0x02
                                     ,0x00,0x00
                                     ,0x00,0x00,0x00,0x02
                                     ,0x00,0x01 - family number #01
                                     ,0x00,0x03 - family #1 version
                                     ,0x01,0x10 - family #1 tool id
                                     ,0x04,0x7B - family #1 tool version

                                     ,0x00,0x13 - family number #13
                                     ,0x00,0x02
                                     ,0x01,0x10
                                     ,0x04,0x7B

                                     ,0x00,0x02 - family number #02
                                     ,0x00,0x01
                                     ,0x01,0x01
                                     ,0x04,0x7B

                                     ,0x00,0x03 - family number #03(ICMB)
                                     ,0x00,0x01
                                     ,0x01,0x10
                                     ,0x04,0x7B

                                     ,0x00,0x15 - family number #15
                                     ,0x00,0x01
                                     ,0x01,0x10
                                     ,0x04,0x7B

                                     ,0x00,0x04 - family number #04
                                     ,0x00,0x01
                                     ,0x01,0x10
                                     ,0x04,0x7B

                                     ,0x00,0x06 - family number #06
                                     ,0x00,0x01
                                     ,0x01,0x10
                                     ,0x04,0x7B

                                     ,0x00,0x09 - family number #09
                                     ,0x00,0x01
                                     ,0x01,0x10
                                     ,0x04,0x7B

                                     ,0x00,0x0A - family number #0A
                                     ,0x00,0x01
                                     ,0x01,0x10
                                     ,0x04,0x7B

                                     ,0x00,0x0B - family number #0B
                                     ,0x00,0x01
                                     ,0x01,0x10
                                     ,0x04,0x7B};*/


   char client_ready[48]=          {0x2A,0x02,0x00,0x1b,0x00,0x2A
                                   ,0x00,0x01,0x00,0x02
                                   ,0x00,0x00
                                   ,0x00,0x00,0x00,0x02

                                   ,0x00,0x01 //family #01
                                   ,0x00,0x03 //3      // 4
                                   ,0x01,0x10 // 1,10  // 0,10
                                   ,0x04,0x7B //4, 7B  // 8,F1


                                     ,0x00,0x02 //- family number #02
                                     ,0x00,0x01
                                     ,0x01,0x01
                                     ,0x04,0x7B

                                   ,0x00,0x03 //family #03
                                   ,0x00,0x01
                                   ,0x01,0x10
                                   ,0x04,0x7B


                                     ,0x00,0x04 //- family number #04
                                     ,0x00,0x01
                                     ,0x01,0x10
                                     ,0x04,0x7B
                                   };

   fWriteColorInTextView("attempting to connect to aim server ip..","fg_red");
   aimsock = Connect2Server(ip,5190);if(aimsock <= 0){return "connect error[aimsock ip failed]";}
   ioctl(aimsock, FIONBIO, &arg);
   while(aimsock)
   {
      while(g_main_context_iteration(NULL,FALSE));//update-ui
      bytes = recv(aimsock,rbuff,10000,0);
      if(bytes < 0 && errno != EWOULDBLOCK){return "socket error[aimsock returned -1]";}
      if(bytes > 0)
      {
         /* Receive Server ACK  */
         if(rbuff[4] == 0x00 && rbuff[5] == 0x04 && rbuff[6] == 0x00 && rbuff[7] == 0x00 && rbuff[8] == 0x00 && rbuff[9] == 0x01)
         {
            /* Send BOS cookies/Receive supporting list */
            fWriteColorInTextView("..done\n","fg_blue");
            char cookiepkt[0x100+14];
            char cookiehdr[]={0x2A,0x01,0x00,0x0E,0x01,0x08,0x00,0x00,0x00,0x01,0x00,0x06,0x01,0x00};
            memcpy(cookiepkt,cookiehdr,14);
            memcpy(cookiepkt+14,cookies,0x100);
            send(aimsock,cookiepkt,270,0);
            fWriteColorInTextView("sending login cookie..","fg_red");
         }
         /* Received supporting list */
         if(rbuff[7] == 0x01 && rbuff[9] == 0x03)
         {
            fWriteColorInTextView("..done\n","fg_blue");
            send(aimsock,client_service_version,56,0);
            fWriteColorInTextView("sending login client services..","fg_red");
         }
         /* Received services version number */
         if(rbuff[7] == 0x01 && rbuff[9] == 0x18)
         {
            fWriteColorInTextView("..done\n","fg_blue");
            send(aimsock,client_rate_limit,16,0);
            fWriteColorInTextView("sending client sign in..","fg_red");
         }
         /* Received Rate Limitations */
         if(rbuff[7] == 0x01 && rbuff[9] == 0x07)
         {
            fWriteColorInTextView("..online!\n","fg_blue");
            send(aimsock,client_services,134,0);
            send(aimsock,set_icmb_params,32,0);
            /* Client Profile string */
            char set_user_info[189+strlen(profilestring)];
            memset((char*)&set_user_info,0,sizeof(set_user_info));
            memcpy(set_user_info,set_user_info_1,57);
            memcpy(set_user_info+57,profilestring,strlen(profilestring));
            memcpy(set_user_info+57+strlen(profilestring),set_user_info_2,132);
            send(aimsock,set_user_info,189 + strlen(profilestring),0);
            /* Its been this mofo the whole time */
            send(aimsock,client_ready,48,0);
            return NULL;
         }
      }
   }
}
