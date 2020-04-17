#include <gtk/gtk.h>

GtkWindow *window;
GtkWidget *headerBar;
GtkWidget *textView;
gchar *filename;

static void choose_file(GtkButton *button, gpointer *data)
{
    GtkWidget *dialog;
    gint res;
    dialog = gtk_file_chooser_dialog_new("Open File",
                                         window,
                                         GTK_FILE_CHOOSER_ACTION_OPEN,
                                         "Cancel",
                                         GTK_RESPONSE_CANCEL,
                                         "Open",
                                         GTK_RESPONSE_ACCEPT,
                                         NULL);
    res = gtk_dialog_run(GTK_DIALOG(dialog));
    if (res == GTK_RESPONSE_ACCEPT) {
        GtkFileChooser *chooser = GTK_FILE_CHOOSER(dialog);
        filename = gtk_file_chooser_get_filename(chooser);
        gtk_header_bar_set_subtitle(GTK_HEADER_BAR(headerBar), filename);

        gchar *contents;
        if (g_file_get_contents(filename, &contents, NULL, NULL)) {
            GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textView));
            gtk_text_buffer_set_text(buffer, contents, -1);
        }
    }
    gtk_widget_destroy(dialog);
}

static void new_document(GtkButton *button, gpointer *data)
{
}

static void save_file(GtkButton *button, gpointer *data)
{
    GtkTextBuffer *buffer;
    GtkTextIter start, end;
    gchar *contents;

    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textView));
    gtk_text_buffer_get_bounds(buffer, &start, &end);
    contents = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);

    if (filename == NULL) {
        GtkWidget *dialog;
        GtkFileChooser *chooser;
        gint res;
        dialog = gtk_file_chooser_dialog_new("Save File",
                                             window,
                                             GTK_FILE_CHOOSER_ACTION_SAVE,
                                             "Cancel",
                                             GTK_RESPONSE_CANCEL,
                                             "Save",
                                             GTK_RESPONSE_ACCEPT,
                                             NULL);
        chooser = GTK_FILE_CHOOSER(dialog);
        res = gtk_dialog_run(GTK_DIALOG(dialog));
        if (res == GTK_RESPONSE_ACCEPT) {
            filename = gtk_file_chooser_get_filename(chooser);
            g_file_set_contents(filename, contents, -1, NULL);
        }
        gtk_widget_destroy(dialog);
    } else {
        g_file_set_contents(filename, contents, -1, NULL);
    }
    g_free(contents);
}

static void create_ui()
{
    // window
    window = GTK_WINDOW(gtk_window_new(GTK_WINDOW_TOPLEVEL));
    gtk_window_set_default_size(window, 800, 800);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    // keyboard shortcuts
    GtkAccelGroup *accelGroup;
    accelGroup = gtk_accel_group_new();
    gtk_window_add_accel_group(window, accelGroup);
    // header bar
    headerBar = gtk_header_bar_new();
    gtk_header_bar_set_title(GTK_HEADER_BAR(headerBar), "my edit");
    gtk_window_set_titlebar(window, headerBar);
    // file chooser
    GtkWidget *chooserBtn;
    chooserBtn = gtk_button_new_with_label("Open");
    g_signal_connect(chooserBtn, "clicked", G_CALLBACK(choose_file), NULL);
    gtk_header_bar_pack_start(GTK_HEADER_BAR(headerBar), chooserBtn);
    // new doc
    GtkWidget *newDocBtn;
    newDocBtn = gtk_button_new_from_icon_name("document-new", GTK_ICON_SIZE_BUTTON);
    g_signal_connect(newDocBtn, "clicked", G_CALLBACK(new_document), NULL);
    gtk_header_bar_pack_start(GTK_HEADER_BAR(headerBar), newDocBtn);
    gtk_widget_add_accelerator(newDocBtn, "clicked", accelGroup, GDK_KEY_o, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    // save btn
    GtkWidget *saveBtn;
    saveBtn = gtk_button_new_with_label("Save");
    g_signal_connect(saveBtn, "clicked", G_CALLBACK(save_file), NULL);
    gtk_header_bar_pack_end(GTK_HEADER_BAR(headerBar), saveBtn);
    gtk_widget_add_accelerator(saveBtn, "clicked", accelGroup, GDK_KEY_s, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    // close btn
    gtk_header_bar_set_show_close_button (GTK_HEADER_BAR(headerBar), TRUE);
    // textview
    GtkWidget *scrolledWindow;
    scrolledWindow = gtk_scrolled_window_new(NULL, NULL);
    textView = gtk_text_view_new();
    gtk_container_add(GTK_CONTAINER(scrolledWindow), textView);
    gtk_container_add(GTK_CONTAINER(window), scrolledWindow);

    gtk_widget_show_all(GTK_WIDGET(window));

}

int main(int argc, char *argv[])
{
    gtk_init(&argc, &argv);

    create_ui();

    gtk_main();

    return 0;
}
