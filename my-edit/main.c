#include <gtk/gtk.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define BUF_SIZE 524288000

GtkWindow *window;
GtkWidget *textView;

static void choose_file(GtkButton *button, gchar *data)
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
        char *filename;
        GtkFileChooser *chooser = GTK_FILE_CHOOSER(dialog);
        filename = gtk_file_chooser_get_filename(chooser);
        gchar *contents;
        if (g_file_get_contents(filename, &contents, NULL, NULL)) {
            GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textView));
            gtk_text_buffer_set_text(buffer, contents, -1);
        }

        g_free(filename);
    }
    gtk_widget_destroy(dialog);
}

int main(int argc, char *argv[])
{
    gtk_init(&argc, &argv);

    // window
    window = GTK_WINDOW(gtk_window_new(GTK_WINDOW_TOPLEVEL));
    gtk_window_set_default_size(window, 600, 500);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    // header bar
    GtkWidget *headerBar;
    headerBar = gtk_header_bar_new();
    gtk_header_bar_set_title(GTK_HEADER_BAR(headerBar), "my edit");
    gtk_window_set_titlebar(window, headerBar);
    // file chooser
    GtkWidget *chooserBtn;
    chooserBtn = gtk_button_new_with_label("Open..");
    g_signal_connect(chooserBtn, "clicked", G_CALLBACK(choose_file), NULL);
    gtk_header_bar_pack_start(GTK_HEADER_BAR(headerBar), chooserBtn);
    // textview
    GtkWidget *scrolledWindow;
    scrolledWindow = gtk_scrolled_window_new(NULL, NULL);
    textView = gtk_text_view_new();
    gtk_container_add(GTK_CONTAINER(scrolledWindow), textView);
    gtk_container_add(GTK_CONTAINER(window), scrolledWindow);

    gtk_widget_show_all(GTK_WIDGET(window));

    gtk_main();

    return 0;
}
