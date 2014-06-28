static void fWriteInTextView(gchar *data)
{
   GtkTextBuffer *buffer;
   GtkTextMark *mark;
   GtkTextIter iter;
   buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(chat_text));
   mark = gtk_text_buffer_get_insert(buffer);
   gtk_text_buffer_get_iter_at_mark(buffer, &iter, mark);
   gtk_text_buffer_insert(buffer, &iter, data, -1);
   gtk_text_view_scroll_mark_onscreen(GTK_TEXT_VIEW(chat_text), mark);
}
static void fWriteColorInTextView(gchar *data, gchar *color)
{
   GtkTextMark *mark;
   GtkTextIter iter;
   buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(chat_text));
   mark = gtk_text_buffer_get_insert(buffer);
   gtk_text_buffer_get_iter_at_mark(buffer, &iter, mark);
   gtk_text_buffer_insert_with_tags_by_name(buffer,&iter,data,-1,color,NULL, NULL);
   gtk_text_view_scroll_mark_onscreen(GTK_TEXT_VIEW(chat_text), mark);
}
static void fClearBuffer()
{
    GtkTextIter start, end;
    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(chat_text));
    gtk_text_buffer_get_start_iter(buffer,&start);
    gtk_text_buffer_get_end_iter(buffer,&end);
    gtk_text_buffer_delete(buffer,&start,&end);
}
enum
{
    LIST_ITEM = 0,
    N_COLUMNS
};
static void init_list(GtkWidget *list)
{
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;
    GtkListStore *store;

    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("List Items",renderer,"text",LIST_ITEM,NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(list), column);
    store = gtk_list_store_new(N_COLUMNS, G_TYPE_STRING);
    gtk_tree_view_set_model(GTK_TREE_VIEW(list), GTK_TREE_MODEL(store));
    g_object_unref(store);
    g_object_set(renderer,"foreground","white",NULL);
}
static void add_to_list(GtkWidget *list, const gchar *str)
{
    GtkTreeIter iter;
    store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(list)));
    gtk_list_store_append(store,&iter);
    gtk_list_store_set(store,&iter,LIST_ITEM,str,-1);
}
static void remove_item(GtkListStore *liststore,gchar *person)
{
    GtkTreeIter iter;
    gboolean valid=true;
    gchar *name;

    if(gtk_tree_model_get_iter_first(GTK_TREE_MODEL(liststore),&iter)==FALSE){return;}
    while(valid)
    {
        gtk_tree_model_get(GTK_TREE_MODEL(liststore),&iter,0,&name,-1);
        if(strcasecmp(name,person)==0)
        {
            gtk_list_store_remove(liststore,&iter);
        }

        valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(liststore),&iter);
    }
}
static void remove_all(GtkWidget *list)
{
   GtkListStore *store;
   GtkTreeIter iter;
   GtkTreeModel *model;

   store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(list)));
   model = gtk_tree_view_get_model(GTK_TREE_VIEW(list));
   if(gtk_tree_model_get_iter_first(model,&iter)==FALSE){return;}
   gtk_list_store_clear(store);
}
GtkWidget *createTextView()
{
   chat_text = gtk_text_view_new();
   GtkWidget *scrolledwindow = gtk_scrolled_window_new(NULL,NULL);
   GtkWidget *console = gtk_table_new(3,1,FALSE);

   gtk_container_add(GTK_CONTAINER(scrolledwindow),chat_text);
   gtk_table_attach_defaults(GTK_TABLE(console),scrolledwindow,0,1,0,1);
   return console;
}
void show_entry_dialog()
{
    GtkWidget *dialog;
    GtkWidget *entry;
    GtkWidget *content_area;

    dialog = gtk_dialog_new();
    gtk_window_set_title(GTK_WINDOW(dialog),"Max Receive length:");
    gtk_dialog_add_button(GTK_DIALOG(dialog), "OK", 0);
    gtk_dialog_add_button(GTK_DIALOG(dialog), "CANCEL", 1);

    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    entry = gtk_entry_new();
    gtk_container_add(GTK_CONTAINER(content_area), entry);

    gtk_widget_show_all(dialog);
    gint result = gtk_dialog_run(GTK_DIALOG(dialog));

    gchar entry_line = NULL; // variable for entered text

    switch(result)
    {
    case 0:
        //ok was pressed
        entry_line = gtk_entry_get_text(GTK_ENTRY(entry));



//entry_line = text

        break;

    case 1:
        break;
    default:
        break;
    }

    gtk_widget_destroy(dialog);
}

