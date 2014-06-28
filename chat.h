int SendMsg(char *chatsend)
{
     char text[10000],sendbuf[10000];
     //it doesnt like 255, 254 is the last message
     //

     //if(LOBYTE(seq4) == 255){seq4=seq4+4;}
     //SendMessage2Chat("OnlineHost:     seq4HI: "+str(HIBYTE(seq4))+" - seq4LO: "+str(LOBYTE(seq4))+"\n", 12268288);
     sprintf(text,"<HTML><FONT COLOR=\"#000000\">%s</HTML>",chatsend);
     uint8_t bytes = strlen(text) + 0x36;
     uint16_t textbytes1 = strlen(text)+22;
     uint16_t textbytes2 = strlen(text);

     sprintf(sendbuf,"%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%s"
     ,0x2A,0x02,0x00,seq4++,0x00,bytes
     /* Snac header */
     ,0x00,0x0E,0x00,0x05
     /* Any Flags */
     ,0x00,0x00
     /* Request ID */
     ,0x00,0x00,0x00,0x05
     /* Message Cookie (FROM_VBC) */
     ,0x46,0x52,0x4F,0x4D,0x5F,0x4C,0x4E,0x58
     /* Message Channel 0x03 */
     ,0x00,0x03
       /* Wisper Flag? = public can see message*/
       ,0x00,0x01
       /* Wisper Length 0?*/
       ,0x00,0x00
       /* Reflection Flag = sends me message from server also*/
       ,0x00,0x06
       /* Reflection Length 0*/
       ,0x00,0x00
       /* Message Information 0x05*/
       ,0x00,0x05
       /* Length of TLV 0x05*/
       ,(textbytes1>>8)&0x00FF,textbytes1&0x00FF
          /* Message Encoding 0x02 */
          ,0x00,0x02
          /* Length Message encoding */
          ,0x00,0x08
          /* Encoding string - us-ascii*/
          ,0x75,0x73,0x2D,0x61,0x73,0x63,0x69,0x69
          /* Country code or Message Language - en */
          ,0x00,0x03
          ,0x00,0x02
          ,0x65,0x6E
          /* Message HTML Text */
          ,0x00,0x01
          ,(textbytes2>>8)&0x00FF,textbytes2&0x00FF
          ,text);


     int error = send(dsock,sendbuf,60+strlen(text),0);
     if(error ==-1){return -1;}
     else{return error;}
}
char * CheckError(int error)
{
     if(error == 0x01){return "Invalid SNAC header";}
     if(error == 0x02){return "Server rate limit exceeded";}
     if(error == 0x03){return "Client rate limit exceeded";}
     if(error == 0x04){return "Recipient is not logged in";}
     if(error == 0x05){return "Service unavailable";}
     if(error == 0x06){return "Service not defined";}
     if(error == 0x07){return "You sent obsolete SNAC";}
     if(error == 0x08){return "Not supported by server";}
     if(error == 0x09){return "Not supported by client";}
     if(error == 0x0A){return "Refused by client";}
     if(error == 0x0B){return "Reply too big";}
     if(error == 0x0C){return "Responses lost";}
     if(error == 0x0D){return "Request denied";}
     if(error == 0x0E){return "Incorrect SNAC format";}
     if(error == 0x0F){return "Insufficient rights";}
     if(error == 0x10){return "In local permit/deny";}
     if(error == 0x11){return "Sender too evil";}
     if(error == 0x12){return "Receiver too evil";}
     if(error == 0x13){return "User unavailable";}
     if(error == 0x14){return "No match";}
     if(error == 0x15){return "List overflow";}
     if(error == 0x16){return "Request ambiguous";}
     if(error == 0x17){return "Server queue full";}
     if(error == 0x18){return "Not while on AOL";}
     if(error > 0x18){return "Error Unknown";}
}
char * PutStringInPacket(char *packet,int pktlen, char *string, int len, int inject)
{
    static char output[10000];
    int a=0,i=0;
    /* Append to the end */
    if(inject == -1)
    {
      for(i=0; i < pktlen;i++,a++)
      {
         output[a] = packet[i];
      }
      for(i=0; i < len;i++,a++)
      {
         output[a] = string[i];
      }
      output[a] = '\0';
      return output;
    }
    /* Append to middle or whatever */
    for(i=0; i < pktlen;i++,a++)
    {
        if(i == inject)
        {
            int j;
            for(j=0; j < len;j++,a++)
            {
                output[a] = string[j];
            }
        }
        output[a] = packet[i];
    }
    output[a] = '\0';
    return output;
}
char * EnterChatRoomWithEstablish(char *chatroom)
{
    int seq2=0x03;
    seq4=0x03;
    char sendbuf[10000],recvbuf[10000];
    char cookies[0x100],ip[20];
    int bytes;

     /* Request new service = 0D(FAMILY) = Chat navigation service */
    fWriteColorInTextView("starting chat request..","fg_red");
    char aim_request_1[]={0x2A,0x02,0x00,sequence++,0x00,0x0C,0x00,0x01,0x00,0x04,0x00,0x00,0x00,0x03,0x00,0x04,0x00,0x0D};
    bytes = send(aimsock,aim_request_1,18,0);
    while(aimsock)
    {
        while(g_main_context_iteration(NULL,FALSE));//update-ui

        bytes = recv(aimsock,recvbuf,10000,0);
        if(bytes < 0 && errno != EWOULDBLOCK){return "Error: recv returned negative";}
        if(bytes > 0)
        {
        /* Check(s)
           1. 0001 0005 - reponse to 0001 0004 = request new service
           2. 0001 0001 - error
           3. else + snac lookup
           */
           /* Response to 0001 0004 */
           if(recvbuf[7] == 0x01 && recvbuf[9] == 0x05)
           {
               /* Extract IP/cookies */
               fWriteColorInTextView("..done\n","fg_blue");
               memset(ip,0,sizeof(ip));
               memset(cookies,0,sizeof(cookies));
               memcpy(ip,recvbuf+26,recvbuf[25]);
               memcpy(cookies,recvbuf+30+strlen(ip),0x100);
               sprintf(sendbuf,"%s:5190\n",ip);
               fWriteColorInTextView("extracted chat request server ip:port..","fg_red");
               fWriteColorInTextView(sendbuf,"fg_blue");
               break;
           }
           else if(recvbuf[7] == 0x01 && recvbuf[9] == 0x01)
           {
               return CheckError(recvbuf[17]);
           }
           else
           {
               sprintf(sendbuf,"Unknown SNAC(%x,%x)",recvbuf[7],recvbuf[9]);
               return sendbuf;
           }
        }
    }

    /* Connect to BOS IP */
    fWriteColorInTextView("attempting to connect to chat request server ip:port..","fg_red");
    isock = Connect2Server(ip,5190);
    if(isock <= 0){return "Chat: BOS server failed";}
    u_long arg=1;ioctl(isock, FIONBIO, &arg);
    /* Respond with cookie packet */
    char cookiepkt[0x100+14];
    char cookiehdr[]={0x2A,0x01,0x00,seq2++,0x01,0x08,0x00,0x00,0x00,0x01,0x00,0x06,0x01,0x00};
    memcpy(cookiepkt,cookiehdr,14);
    memcpy(cookiepkt+14,cookies,0x100);
    bytes = send(isock,cookiepkt,270,0);
    while(isock)
    {
        while(g_main_context_iteration(NULL,FALSE));//update-ui
        bytes = recv(isock,recvbuf,10000,0);
        if(bytes < 0 && errno != EWOULDBLOCK){return "Error: recv returned negative";}
        if(bytes > 0)
        {
            /* Receive Server ACK  */
            if(recvbuf[4] == 0x00 && recvbuf[5] == 0x0c && recvbuf[6] == 0x00 && recvbuf[7] == 0x00 && recvbuf[8] == 0x00 && recvbuf[9] == 0x01)
            {
                fWriteColorInTextView("..done\n","fg_blue");
                /* Send - Client Ready(doesnt give response) */
                char bos_clientready_1[]={0x2A,0x02,0x00,seq2++,0x00,0x1A,0x00,0x01,0x00,0x02,0x00,0x00,0x00,0x00,0x00,0x02,0x00,0x01,0x00,0x03,0x00,0x10,0x06,0x29,0x00,0x0D,0x00,0x01,0x00,0x10,0x06,0x29};
                bytes = send(isock,bos_clientready_1,32,0);
                /* BOS - Chat info request */
                char chat_info_request[]={0x2A,0x02,0x00,seq2++,0x00,0x11+strlen(chatroom),0x00,0x0D,0x00,0x04,0x00,0x00,0x00,0x05,0x00,0x04,0x00,0x04,strlen(chatroom)+1,0x21,0x00,0x00,0x02};
                char *info_request = PutStringInPacket(chat_info_request,23,chatroom,strlen(chatroom),21);
                bytes = send(isock,info_request,23+strlen(chatroom),0);
                fWriteColorInTextView("sending chat info request..","fg_red");
            }
            /* supported snaclist */
            else if(recvbuf[7] == 0x01 && recvbuf[9]==0x03)
            {
                fWriteColorInTextView("..done\n","fg_blue");
                fWriteColorInTextView("sending chatroom request..","fg_red");
                /* you could grab the supporting snaclist */
                break;
            }
            else if(recvbuf[7] == 0x01 && recvbuf[9] == 0x01)
            {
                return CheckError(recvbuf[17]);
            }
            else if(recvbuf[7] == 0x0d && recvbuf[9] == 0x01)
            {
                /* Chat related error */
                if(recvbuf[17] != 0x0E)
                {
                    close(isock);
                    return CheckError(recvbuf[17]);
                }
                else{/* we already know this is wrong format. its an exploit *//* 50%*/break;}
            }
            else
            {
                sprintf(sendbuf,"Unknown SNAC(%x,%x)",recvbuf[7],recvbuf[9]);
                return sendbuf;
            }
        }
    }
    /* Request join chatroom */
    char aim_chat_request[]={0x2A,0x02,0x00,sequence++,0x00,0x16+strlen(chatroom),0x00,0x01,0x00,0x04,0x00,0x00,0x00,0x04,0x00,0x04,0x00,0x0E,0x00,0x01,0x00,strlen(chatroom)+6,0x00,0x04,strlen(chatroom)+1,0x21,0x00,0x00};
    /* Get rid of this put string in packet bullshit */
    char *chat_request = PutStringInPacket(aim_chat_request,28,chatroom,strlen(chatroom),26);
    bytes = send(aimsock,chat_request,28+strlen(chatroom),0);
    while(aimsock)
    {
        while(g_main_context_iteration(NULL,FALSE));//update-ui
        bytes = recv(aimsock,recvbuf,10000,0);
        if(bytes < 0 && errno != EWOULDBLOCK){return "Error: recv returned negative";}
        if(bytes > 0)
        {
           /* Response to 0001 0004 */
           if(recvbuf[7] == 0x01 && recvbuf[9] == 0x05)
           {
               /* Extract IP/cookies */
               fWriteColorInTextView("..done\n","fg_blue");
               fWriteColorInTextView("connecting to chat..","fg_red");
               memset(ip,0,sizeof(ip));
               memset(cookies,0,sizeof(cookies));
               memcpy(ip,recvbuf+26,recvbuf[25]);
               memcpy(cookies,recvbuf+30+strlen(ip),0x100);
               break;
           }
           else if(recvbuf[7] == 0x01 && recvbuf[9] == 0x01)
           {
               return CheckError(recvbuf[17]);
           }
           else
           {
               sprintf(sendbuf,"Unknown SNAC(%x,%x)",recvbuf[7],recvbuf[9]);
               return sendbuf;
           }

        }
    }


    /* Connect to New service(CHAT) */
    dsock = Connect2Server(ip,5190);
    ioctl(dsock, FIONBIO, &arg);
    if(dsock <= 0){return "BOS Chat Server Error";}
    /* Respond with cookie packet */
    cookiehdr[3] = seq4++;
    memcpy(cookiepkt,cookiehdr,14);
    memcpy(cookiepkt+14,cookies,0x100);
    /* Send back cookies auth */
    bytes = send(dsock,cookiepkt,270,0);

    /* Client Ready */
    char packet_client[16]={0x2A,0x02,0x00,seq4++,0x00,0x1A,0x00,0x01,0x00,0x02,0x00,0x00,0x00,0x00,0x00,0x02};
    char packet_payload[16]={0x00,0x01 //family #1
                            ,0x00,0x03 //family version
                            ,0x00,0x10 //family tool id
                            ,0x06,0x29 //family tool version

                            ,0x00,0x0E //family #14
                            ,0x00,0x01 //family version
                            ,0x00,0x10 //family tool id
                            ,0x06,0x29 //family tool version
                            };
    memset((char*)&sendbuf,0,sizeof(sendbuf));
    memcpy(sendbuf,packet_client,16);
    memcpy(sendbuf+16,packet_payload,16);

    bytes = send(dsock,sendbuf,32,0);
    fWriteColorInTextView("..in chatroom\n","fg_blue");
    in_chat=true;

    return NULL;
}
