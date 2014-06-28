char * DeLink(char *data)
{
     static char output[1000000];
     char *fromlink,*tolink,*start;
     int i=0,a=0;
     start = data;
     fromlink = strstr(data,"<");
     if(fromlink == NULL){fromlink = strstr(data,"<");}
     tolink = strstr(fromlink,">");
     for(i=0,a=0;start < fromlink;i++,start++,a++){output[a] = data[i];}
     for(;fromlink < tolink;fromlink++){i++;}
     for(i+=1; i < strlen(data);i++,a++){output[a] = data[i];}
     output[a] = '\0';
     return output;
}
char * DeTagAll(char *data)
{
     while(strstr(data,"<") != NULL)
     {
      data = DeLink(data);
     }
     return data;
}
char * GetTextBetween(char *html,char *from,char *to)
{
     static char output[1000000];
     memset((char*)output,0,sizeof(output));
     char *fromptr,*toptr;
     int i;
     fromptr = strstr(html,from);if(fromptr == NULL){return NULL;}
     toptr = strstr(fromptr+strlen(from),to);
     if(toptr == NULL){return NULL;}

     for(i=0; fromptr < toptr-strlen(from);i++)
     {
      output[i] = fromptr[strlen(from)];
      fromptr++;
     }
     output[i] = '\0';
     return output;
}
unsigned int MakeInt(char c)
{
     char hex[10];
     sprintf(hex,"%x",c);
     unsigned int length = hexToInt(FormatHex(hex));
     return length;
}
char * ToText(char *data, int length)
{

     static char text[10000];
     int a=0,i;
     for(i=0; i < length;i++)
     {
         if(data[i] < 0)
         {
             text[a++] = MakeInt(data[i]);
             continue;
         }
         else if(data[i] >= 0x20)
         {
             text[a++] = data[i];
             continue;
         }

     }
     text[a] = '\0';
     return text;
}
static void CheckChatMessages(char *data, int bytes)
{
    /* channel message */
    if(data[7] == 0x0E && data[9] == 0x06)
    {
        /* Pull Name */
        char screenname[256],cookie[7],buffer[10000];
        memset((char*)&screenname,0,sizeof(screenname));
        memcpy(screenname,data+31,data[30]);
        /* Pull Message */
        char *message = DeTagAll(GetTextBetween(ToText(data,bytes),"<HTML>","</HTML>"));
        if(message == NULL){fWriteInTextView("malformed packet\n");return;}
        sprintf(buffer,": %s\n",message);

        /* Pull Cookie */
        memset((char*)&cookie,0,sizeof(cookie));
        memcpy(cookie,data+16,8);
        cookie[8] = '\0';
        char cookiebuf[300];
        sprintf(cookiebuf,"%s is %s ",screenname,cookie);
        gtk_statusbar_push(GTK_STATUSBAR(status_bar), id,cookiebuf);
        if(strcasecmp(cookie,"FROM_VBC")==0 || strcasecmp(cookie,"FROM_LNX")==0 ){    /* we got both || = OR */}


         if(strcasecmp("the.legend.of.vb",screenname)==0)
         {
            fWriteColorInTextView(screenname,"fg_blue");
            fWriteColorInTextView(buffer,"fg_green");
         }
         else
         {
            fWriteColorInTextView(screenname,"fg_blue");
            fWriteColorInTextView(buffer,"fg_red");
         }
    }
    /* User Entered */
    else if(data[7] == 0x0E && data[9] == 0x03)
    {
        char sn[100];
        char buffer[200];
        int i=17,tlvs=0,b;
        while(i < bytes)
        {
            memset((char*)&sn,0,sizeof(sn));
            memcpy(sn,data+i,data[i-1]);
            fWriteColorInTextView("OnlineHost","fg_blue");
            sprintf(buffer,": %s has entered the room.\n",sn);
            fWriteColorInTextView(buffer,"fg_red");
            add_to_list(chat_list,sn);
            /* Check How many TLV's */
            i=i+data[i-1] +3;
            tlvs=i;
            for(b=0; b < data[tlvs];b++)
            {
                /* Comb through the shit */
                i=i+4;//Get to the location of next TLV Length
                i=i+data[i];//add the next length
            }
            i=i+2;
        }
    }
    /* User Left */
    else if(data[7] == 0x0E && data[9] == 0x04)
    {
        char sn[100];
        char buffer[200];
        int i=17,tlvs=0,b;
        while(i < bytes)
        {
            memset((char*)&sn,0,sizeof(sn));
            memcpy(sn,data+i,data[i-1]);
            fWriteColorInTextView("OnlineHost","fg_blue");
            sprintf(buffer,": %s has left the room.\n",sn);
            fWriteColorInTextView(buffer,"fg_red");
            remove_item(store,sn);
            /* Check How many TLV's */
            i=i+data[i-1] +3;
            tlvs=i;
            for(b=0; b < data[tlvs];b++)
            {
                /* Comb through the shit */
                i=i+4;//Get to the location of next TLV Length
                i=i+data[i];//add the next length
            }
            i=i+2;
        }

    }


}


void CheckIfChuncked(char *data, int bytes)
{
    u_int16_t length=0;
    char buffer[100000];
    int i;
    for(i=0; i < bytes;)
    {
        /* Length of Packet */
        length = MakeInt(data[i+4]) * 0x100;
        length = length + MakeInt(data[i+5]);

        memset((char*)&buffer,0,sizeof(buffer));
        memcpy(buffer,data+i,length+6);
        i=i+length+6;//length + 6header

        if(length+6 < 2500 && length+6 > 10 && buffer[0] == 0x2a)
        {
            CheckChatMessages(buffer,length+6);
        }
    }
}

void ReceiveLoop()
{

    char data[100000];
    char buffer[100000];
    char pad[1000];
    int bytes;
    while(aim_quit == false)
    {
        nanosleep((struct timespec[]){0,50000000},NULL);
        /* Mainly for aimsock */
        while(g_main_context_iteration(NULL,FALSE));//update-ui
        bytes = recv(aimsock,data,10000,0);
        if(bytes < 0 && errno != EWOULDBLOCK){return "Error: recv returned negative";aim_quit=true;}
        if(bytes > 0)
        {
            //char buff[200];
            //sprintf(buff,"Received %d bytes\n",bytes);
            //fWriteInTextView(buff);
        }
        /* Mainly for chat->dsock */
        if(in_chat == true && dsock >= 0)
        {
             bytes = recv(dsock,data,100000,0);
             if(bytes < 0 && errno != EWOULDBLOCK){return "Error: recv returned negative";}
             if(bytes > 0)
             {
                CheckIfChuncked(data,bytes);
             }
        }
     }
     if(aim_quit == true)
     {
         /* Shut down sockets properly */
         gtk_widget_set_sensitive(start_vbc,TRUE);
         gtk_widget_set_sensitive(vbc_quit,FALSE);
         gtk_widget_set_sensitive(start_chat,FALSE);
         gtk_widget_set_sensitive(chat_exit,FALSE);
         fWriteColorInTextView("OnlineHost: Signed offline\n","fg_red");
         close(aimsock);
         close(dsock);
         close(isock);
         in_chat=false;
         aim_quit=false;
     }
}
