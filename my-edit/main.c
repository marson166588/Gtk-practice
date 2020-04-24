#include <gtk/gtk.h>

GtkWindow *window;
GtkWidget *headerBar;
GtkWidget *noteBook;

GArray *text_view_array;
typedef struct {
    gboolean is_saved;
    gint page_number;
    gchar *filename;
    GtkWidget *scrolled_window;
    GtkWidget *text_view;
} TextViewData;

static void set_show_notebook_tabs();
static void set_notebook_tab_label(gchar *labelText, GtkWidget *child);
static void close_notebook_tab(GtkButton *button, GtkWidget *data);

static void print_array()
{
    gint i;
    TextViewData *d;
    for (i = 0; i < text_view_array->len; i++) {
        d = &g_array_index(text_view_array, TextViewData, i);
        g_print("i: %d, len: %d, is_saved: %d, page_number: %d, filename: %s\n", i, text_view_array->len, d->is_saved, d->page_number, d->filename);
    }
}

gchar *get_basename_from_path(const char *path)
{
    GFile *gFile;
    GFileInfo *gFileInfo;
    gchar *basename;

    gFile     = g_file_new_for_path(path);
    gFileInfo = g_file_query_info(gFile, G_FILE_ATTRIBUTE_STANDARD_DISPLAY_NAME, G_FILE_QUERY_INFO_NONE, NULL, NULL);
    basename  = g_file_info_get_attribute_as_string(gFileInfo, G_FILE_ATTRIBUTE_STANDARD_DISPLAY_NAME);

    g_object_unref(gFile);
    g_object_unref(gFileInfo);

    return basename;
}

static void insert_to_text_view_array(gboolean isSaved, gint pageNumber, gchar *filename, GtkWidget *scrolledWindow, GtkWidget *textview)
{
    g_array_remove_index(text_view_array, pageNumber);
    TextViewData textViewData = {
        is_saved: isSaved,
        page_number: pageNumber,
        filename: filename,
        scrolled_window: scrolledWindow,
        text_view: textview
    };
    g_array_insert_val(text_view_array, pageNumber, textViewData);
}

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

        gchar *filename, *contents;
        filename = gtk_file_chooser_get_filename(chooser);
        gtk_header_bar_set_subtitle(GTK_HEADER_BAR(headerBar), filename);

        gint pageNumber;
        TextViewData *pageData;
        GtkWidget *currentScrolledWindow;
        GtkWidget *currentTextView;
        pageNumber      = gtk_notebook_get_current_page(GTK_NOTEBOOK(noteBook));
        pageData        = &g_array_index(text_view_array, TextViewData, pageNumber);

        currentScrolledWindow = pageData->scrolled_window;
        currentTextView = pageData->text_view;

        GtkTextBuffer *buffer;
        GtkTextIter start, end;
        buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(currentTextView));
        gtk_text_buffer_get_bounds(buffer, &start, &end);
        contents = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);

        if (contents[0] == '\0') {
            g_print("not write any char\n");
        } else {
            g_print("open choose file");
            currentScrolledWindow = gtk_scrolled_window_new(NULL, NULL);
            currentTextView = gtk_text_view_new();
            gtk_container_add(GTK_CONTAINER(currentScrolledWindow), currentTextView);
            pageNumber = gtk_notebook_append_page(GTK_NOTEBOOK(noteBook), currentScrolledWindow, NULL);

            // reget buffer content
            buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(currentTextView));
            gtk_text_buffer_get_bounds(buffer, &start, &end);
            contents = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);
        }

        if (g_file_get_contents(filename, &contents, NULL, NULL)) {
            gtk_text_buffer_set_text(buffer, contents, -1);
        }

        // set tab label
        set_notebook_tab_label(get_basename_from_path(filename), currentScrolledWindow);
        // insert text_view_array
        insert_to_text_view_array(TRUE, pageNumber, filename, currentScrolledWindow, currentTextView);
        // show
        gtk_widget_show_all(GTK_WIDGET(window));
        gtk_notebook_set_current_page(GTK_NOTEBOOK(noteBook), pageNumber);
    }

    // print_array();
    gtk_widget_destroy(dialog);
    set_show_notebook_tabs();
}

static void new_document(GtkButton *button, gpointer *data)
{
    GtkWidget *newScrolledWindow = gtk_scrolled_window_new(NULL, NULL);
    GtkWidget *newTextView = gtk_text_view_new();
    gtk_container_add(GTK_CONTAINER(newScrolledWindow), newTextView);
    gint pageNumber = gtk_notebook_append_page(GTK_NOTEBOOK(noteBook), newScrolledWindow, NULL);

    char labelSuffix[2];
    sprintf(labelSuffix, "%d", pageNumber);
    gchar *labelText  = g_strjoin(" ", "Untitled Doc", labelSuffix, NULL);
    set_notebook_tab_label(labelText, newScrolledWindow);

    insert_to_text_view_array(FALSE, pageNumber, labelText, newScrolledWindow, newTextView);

    set_show_notebook_tabs();

    gtk_widget_show_all(GTK_WIDGET(window));
    gtk_notebook_set_current_page(GTK_NOTEBOOK(noteBook), pageNumber);
}

static void save_file(GtkButton *button, gpointer *data)
{
    gint pageNumber;
    TextViewData *pageData;
    GtkWidget *scrolledWindow;
    GtkWidget *textView;
    pageNumber     = gtk_notebook_get_current_page(GTK_NOTEBOOK(noteBook));
    pageData       = &g_array_index(text_view_array, TextViewData, pageNumber);
    scrolledWindow = pageData->scrolled_window;
    textView       = pageData->text_view;

    GtkTextBuffer *buffer;
    GtkTextIter start, end;
    gchar *contents;
    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textView));
    gtk_text_buffer_get_bounds(buffer, &start, &end);
    contents = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);

    // print_array();
    if (pageData->is_saved) {
        g_file_set_contents(pageData->filename, contents, -1, NULL);
    } else {
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
            gchar *filename = gtk_file_chooser_get_filename(chooser);
            g_file_set_contents(filename, contents, -1, NULL);
            // update text_view_array
            insert_to_text_view_array(TRUE, pageNumber, filename, scrolledWindow, textView);
            // set tab label
            gtk_notebook_set_tab_label_text(GTK_NOTEBOOK(noteBook), scrolledWindow, get_basename_from_path(filename));
        }
        gtk_widget_destroy(dialog);
    }
    g_free(contents);
}

static void set_show_notebook_tabs()
{
    if (text_view_array->len == 1) {
        gtk_notebook_set_show_tabs(GTK_NOTEBOOK(noteBook), FALSE);
    } else {
        gtk_notebook_set_show_tabs(GTK_NOTEBOOK(noteBook), TRUE);
    }
}

static void set_notebook_tab_label(gchar *labelText, GtkWidget *child)
{
    GtkWidget *tabBox, *label, *closeBtn;

    tabBox   = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
    label    = gtk_label_new(labelText);
    closeBtn = gtk_button_new_from_icon_name("window-close-symbolic", GTK_ICON_SIZE_SMALL_TOOLBAR);

    gtk_button_set_relief(GTK_BUTTON(closeBtn), GTK_RELIEF_NONE);
    gtk_box_pack_start(GTK_BOX(tabBox), label, TRUE, TRUE, 0);
    gtk_box_pack_end(GTK_BOX(tabBox), closeBtn, FALSE, FALSE, 0);
    gtk_widget_show_all(tabBox);

    gtk_notebook_set_tab_label(GTK_NOTEBOOK(noteBook), child, tabBox);
    // close tab callback
    g_signal_connect(closeBtn, "clicked", G_CALLBACK(close_notebook_tab), child);
}

static void close_notebook_tab(GtkButton *button, GtkWidget *data)
{
    gint pageNumber = gtk_notebook_page_num(GTK_NOTEBOOK(noteBook), data);
    if (pageNumber > -1) {
        // remove page from notebook
        gtk_notebook_remove_page(GTK_NOTEBOOK(noteBook), pageNumber);
        // remove from garray
        g_array_remove_index(text_view_array, pageNumber);
    }
}

static void init_notebook()
{
    GtkWidget *newScrolledWindow   = gtk_scrolled_window_new(NULL, NULL);
    GtkWidget *newTextView = gtk_text_view_new();
    gtk_container_add(GTK_CONTAINER(newScrolledWindow), newTextView);

    noteBook = gtk_notebook_new();
    gtk_notebook_set_scrollable(GTK_NOTEBOOK(noteBook), TRUE);
    gint pageNumber   = gtk_notebook_append_page(GTK_NOTEBOOK(noteBook), newScrolledWindow, NULL);

    gchar *labelText  = "Untitled Doc";
    set_notebook_tab_label(labelText, newScrolledWindow);

    insert_to_text_view_array(FALSE, pageNumber, labelText, newScrolledWindow, newTextView);

    set_show_notebook_tabs();
}

static void create_ui()
{
    // window
    window = GTK_WINDOW(gtk_window_new(GTK_WINDOW_TOPLEVEL));
    gtk_window_set_default_size(window, 600, 500);
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
    gtk_header_bar_set_show_close_button(GTK_HEADER_BAR(headerBar), TRUE);
    // textview
    init_notebook();
    gtk_container_add(GTK_CONTAINER(window), noteBook);

    gtk_widget_show_all(GTK_WIDGET(window));
}

int main(int argc, char *argv[])
{
    text_view_array = g_array_new(FALSE, TRUE, sizeof(TextViewData));
    gtk_init(&argc, &argv);
    create_ui();
    gtk_main();

    return 0;
}
