#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <gdk/gdkkeysyms.h>
#include <glib.h>

GtkWidget *term_view = NULL;
GtkWidget *win = NULL;
GtkWidget *entry_text=NULL;
GtkWidget *vbc_quit = NULL;
GtkWidget *start_chat = NULL;
GtkWidget *start_vbc = NULL;
GtkWidget *sn_text = NULL;
GtkWidget *pw_text = NULL;
GtkWidget *chat_combo=NULL;
GtkWidget *chat_list=NULL;
GtkWidget *chat_exit=NULL;
GtkWidget *chat_text=NULL;
gchar *text,*screen_name,*pass_word;
GtkTextBuffer *buffer;
GtkListStore *store;
gint id;
gchar *info;
GtkWidget *status_bar;

#include "ui.h"
#include "login.h"
#include "chat.h"
#include "rloop.h"


static void startVBC(GtkWidget *wid, GtkWidget *win)
{
   fWriteColorInTextView("loading server login.oscar.aol.com..","fg_red");
   screen_name = gtk_entry_get_text(GTK_ENTRY(sn_text));
   pass_word = gtk_entry_get_text(GTK_ENTRY(pw_text));
   if(strlen(screen_name)< 2 || strlen(pass_word)<2){fWriteColorInTextView("\nFix screen name or password\n","fg_red");return;}
   /* Connect to server */
   int oscar_sock =NULL;
   while(oscar_sock == 0){oscar_sock = Connect2Server("login.oscar.aol.com",5190);}
   if(oscar_sock < 0){fWriteColorInTextView(serverErrorList(oscar_sock),"fg_blue");close(oscar_sock);return;}
   fWriteColorInTextView("..done\n","fg_blue");
   /* Login AIM */
   char *error = Login2AIM(oscar_sock,screen_name,pass_word);
   if(error != NULL){fWriteColorInTextView(error,"fg_blue");fWriteInTextView("\n");return;}
   gtk_widget_set_sensitive(start_vbc,FALSE);
   gtk_widget_set_sensitive(vbc_quit,TRUE);
   gtk_widget_set_sensitive(start_chat,TRUE);
   ReceiveLoop();
}
static void startChat(GtkWidget *wid, GtkWidget *win)
{
   fClearBuffer();
   remove_all(chat_list);
   const char *chat_link = gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(chat_combo)->entry));
   char *error = EnterChatRoomWithEstablish((char *)chat_link);
   if(error != NULL){fWriteInTextView(error);return;}
   gtk_widget_set_sensitive(start_chat,FALSE);
   gtk_widget_set_sensitive(chat_exit,TRUE);
}
static void endChat(GtkWidget *wid, GtkWidget *win)
{
   close(isock);
   close(dsock);
   in_chat=false;
   gtk_widget_set_sensitive(chat_exit,FALSE);
   gtk_widget_set_sensitive(start_chat,TRUE);

}
static void VBCQuit(GtkWidget *wid, GtkWidget *win)
{
    aim_quit=true;
}
static gboolean keyPress(GtkWidget *widget, GdkEventKey *event, gpointer user_data)
{
   switch(event->keyval)
   {
      case GDK_KEY_Return:
      /* send chat */
      text = gtk_entry_get_text(GTK_ENTRY(entry_text));
      SendMsg(text);
      gtk_entry_set_text(GTK_ENTRY(entry_text),"");
      break;

      default:
       return FALSE;
   }
   return FALSE;
}
static gboolean mousePress(GtkWidget *widget, GdkEventKey *event, gpointer user_data)
{
}
int main (int argc, char *argv[])
{
  GtkWidget *fixedbox = NULL;

  /* Initialize GTK+ */
  g_log_set_handler ("Gtk", G_LOG_LEVEL_WARNING, (GLogFunc) gtk_false, NULL);
  gtk_init (&argc, &argv);
  g_log_set_handler ("Gtk", G_LOG_LEVEL_WARNING, g_log_default_handler, NULL);

  /* Create the main window */
  win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(win), "linux-vbc");
  gtk_window_set_default_size(GTK_WINDOW(win), 830, 540);
  gtk_window_set_position(GTK_WINDOW(win), GTK_WIN_POS_CENTER);
  g_signal_connect (win, "destroy", gtk_main_quit, NULL);
  /* Create a vertical box with buttons */



  /* Set Frame */
  GtkWidget *frame = gtk_frame_new(NULL);
  gtk_container_add(GTK_CONTAINER (win), frame);
  gtk_frame_set_shadow_type(GTK_FRAME(frame),GTK_SHADOW_ETCHED_IN);
  /* Set fixed container inside frame */
  fixedbox = gtk_fixed_new ();
  gtk_container_add (GTK_CONTAINER (frame), fixedbox);


  status_bar = gtk_statusbar_new();
  id = gtk_statusbar_get_context_id(GTK_STATUSBAR(status_bar), "info");
  info = "This was uninitialized";
  gtk_statusbar_push(GTK_STATUSBAR(status_bar), id, info);
  gtk_fixed_put(GTK_FIXED(fixedbox), status_bar, 5, 540);
  gtk_widget_set_size_request(status_bar, 200, 20);


  /* Add button - start vbc*/
  start_vbc = gtk_button_new_with_label ("Start");
  gtk_fixed_put(GTK_FIXED(fixedbox), start_vbc, 405, 10);
  gtk_widget_set_size_request(start_vbc, 80, 25);
  g_signal_connect (G_OBJECT (start_vbc), "clicked", G_CALLBACK (startVBC), (gpointer) win);
    /* Add button - start chat*/
  start_chat = gtk_button_new_with_label ("Chat");
  gtk_fixed_put(GTK_FIXED(fixedbox), start_chat, 405, 35);
  gtk_widget_set_size_request(start_chat, 80, 35);
  g_signal_connect (G_OBJECT (start_chat), "clicked", G_CALLBACK (startChat), (gpointer) win);
  gtk_widget_set_sensitive(start_chat,FALSE);
    /* Add button - quit aim*/
  vbc_quit = gtk_button_new_with_label ("End AIM");
  gtk_fixed_put(GTK_FIXED(fixedbox), vbc_quit, 485, 10);
  gtk_widget_set_size_request(vbc_quit, 80, 25);
  g_signal_connect (G_OBJECT (vbc_quit), "clicked", G_CALLBACK (VBCQuit), (gpointer) win);
  gtk_widget_set_sensitive(vbc_quit,FALSE);
    /* Add button - exit chat*/
  chat_exit = gtk_button_new_with_label ("End Chat");
  gtk_fixed_put(GTK_FIXED(fixedbox), chat_exit, 485, 35);
  gtk_widget_set_size_request(chat_exit, 80, 35);
  g_signal_connect (G_OBJECT (chat_exit), "clicked", G_CALLBACK (endChat), (gpointer) win);
  gtk_widget_set_sensitive(chat_exit,FALSE);
  /* Textview - consoleFRAME - term_view has scrollbar attached to textview in 1 widget*/
  term_view = createTextView();
  gtk_fixed_put(GTK_FIXED(fixedbox),term_view, 5, 75);
  gtk_widget_set_size_request(term_view, 660, 430);
  /* Textview - Actual - chat_text is now the new term_view */
  gtk_text_view_set_editable(GTK_TEXT_VIEW(chat_text),FALSE);
  buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(chat_text));//initialize color
  gtk_text_buffer_create_tag(buffer,"fg_red","foreground","green",NULL);
  buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(chat_text));//initialize color
  gtk_text_buffer_create_tag(buffer,"fg_blue","foreground","white",NULL);
  buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(chat_text));//initialize color
  gtk_text_buffer_create_tag(buffer,"fg_green","foreground","orange",NULL);
  GdkColor color;
  color.red =100;
  color.green=100;
  color.blue=100;
  gtk_widget_modify_base(chat_text,GTK_STATE_NORMAL,&color);
  g_signal_connect(G_OBJECT (chat_text), "button-press-event",G_CALLBACK (mousePress), NULL);//noclick
  /* entry - sendtext */
  entry_text = gtk_entry_new();
  gtk_fixed_put(GTK_FIXED(fixedbox),entry_text, 5, 510);
  gtk_widget_set_size_request(entry_text, 660, 30);
  g_signal_connect(G_OBJECT (entry_text), "key_press_event",G_CALLBACK (keyPress), NULL);
    /* entry - screen name */
  sn_text = gtk_entry_new();
  gtk_fixed_put(GTK_FIXED(fixedbox),sn_text, 5, 10);
  gtk_widget_set_size_request(sn_text, 200, 25);
  gtk_entry_set_text(GTK_ENTRY(sn_text),"the.legend.of.vb");
    /* entry - password */
  pw_text = gtk_entry_new();
  gtk_fixed_put(GTK_FIXED(fixedbox),pw_text, 205, 10);
  gtk_widget_set_size_request(pw_text, 200, 25);
  gtk_entry_set_text(GTK_ENTRY(pw_text),"lb13126");
  gtk_entry_set_visibility(GTK_ENTRY(pw_text),FALSE);
  /* ComboBox  - Chatlist */
  chat_combo = gtk_combo_new();
  gtk_fixed_put(GTK_FIXED(fixedbox),chat_combo, 5, 35);
  gtk_widget_set_size_request(chat_combo, 400, 35);
  GList *glist=NULL;
  glist = g_list_append(glist,"aol://2719:#-#-");
  glist = g_list_append(glist,"aol://2719:27-2-macorpc");
  glist = g_list_append(glist,"aol://2719:22-2-authorslounge");
  glist = g_list_append(glist,"aol://2719:58-2-investing");
  glist = g_list_append(glist,"aol://2719:198-2-democrats");
  glist = g_list_append(glist,"aol://2719:198-2-republicans");
  glist = g_list_append(glist,"aol://2719:26-2-aloneathome");
  glist = g_list_append(glist,"aol://2719:25-2-nyc");
  glist = g_list_append(glist,"aol://2719:22-2-authorslounge");
  glist = g_list_append(glist,"aol://2719:59-2-football");
  glist = g_list_append(glist,"aol://2719:21-2-bikerbar");
  gtk_combo_set_popdown_strings(GTK_COMBO(chat_combo),glist);
  /* Listbox */
  chat_list = gtk_tree_view_new();
  gtk_fixed_put(GTK_FIXED(fixedbox),chat_list, 670, 10);
  gtk_widget_set_size_request(chat_list, 150, 495);
  gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(chat_list),FALSE);
  gtk_widget_modify_base(chat_list,GTK_STATE_NORMAL,&color);
  init_list(chat_list);

  /* Enter the main loop */
  gtk_widget_show_all (win);
  gtk_main ();
  return 0;
}
