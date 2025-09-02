#include <libadwaita-1/adwaita.h>

typedef struct {
  GtkEntry *username_entry;
  GtkEntry *password_entry;
  GtkEntry *other_entry;
  GtkEntry *hours;
  GtkEntry *minutes;
  GtkLabel *clock_label;
  guint clock_source_id;
} AppWidgets;

typedef struct {
  GtkWidget *label;  // The label widget to display the time
  time_t start_time; // The time when the clock was started
  guint timer_id;    // The ID of the timer event source
  gboolean running;  // Flag to check if the clock is running
} ClockData;

static gboolean update_clock(gpointer user_data) {
  ClockData *data = (ClockData *)user_data;
  time_t current_time = time(NULL);
  double elapsed_seconds = difftime(current_time, data->start_time);

  // 2 hours and 15 minutes = (2 * 3600) + (15 * 60) = 7200 + 900 = 8100 seconds
  if (elapsed_seconds >= 8100) {
    gtk_label_set_text(GTK_LABEL(data->label), "Timer Finished");
    data->running = FALSE;
    data->timer_id = 0;     // Clear the timer ID
    return G_SOURCE_REMOVE; // Stop the timer
  }
  // Format the current time into HH:MM:SS
  char time_str[9];
  struct tm *time_info = localtime(&current_time);
  strftime(time_str, sizeof(time_str), "%H:%M:%S", time_info);

  // Update the label text with the current time
  gtk_label_set_text(GTK_LABEL(data->label), time_str);

  return G_SOURCE_CONTINUE; // Tell the GLib main loop to call this function
                            // again
}
// Start Recording or Stop Recording
static void start_stop_clock(GtkWidget *button, gpointer user_data) {
  ClockData *data = (ClockData *)user_data;

  if (data->running) {
    // If the clock is running, stop it
    if (data->timer_id > 0) {
      g_source_remove(data->timer_id);
      data->timer_id = 0;
    }
    data->running = FALSE;
    gtk_button_set_label(GTK_BUTTON(button), "Start Clock");
    gtk_label_set_text(GTK_LABEL(data->label), "Stopped");
  } else {
    // If the clock is stopped, start it
    data->start_time = time(NULL);
    data->running = TRUE;
    // Call update_clock immediately to show the time right away
    update_clock(data);
    // Schedule update_clock to be called every 1 second
    data->timer_id = g_timeout_add_seconds(1, update_clock, data);
    gtk_button_set_label(GTK_BUTTON(button), "Stop Clock");
  }
}

static void submit_button_clicked(GtkWidget *widget, gpointer data) {
  AppWidgets *widgets = (AppWidgets *)data;
  const char *username =
      gtk_editable_get_text(GTK_EDITABLE(widgets->username_entry));
  const char *password =
      gtk_editable_get_text(GTK_EDITABLE(widgets->password_entry));
  const char *other = gtk_editable_get_text(GTK_EDITABLE(widgets->other_entry));

  // Print the captured text to the console.
  g_print("Submit Clicked!\n");
  g_print("  Username: %s\n", username);
  g_print("  Password: %s\n", password);
  g_print("  Other Info: %s\n", other);
}

// Callback function for the "Clear" button.
// It clears all text from the input fields.
static void clear_button_clicked(GtkWidget *widget, gpointer data) {
  AppWidgets *widgets = (AppWidgets *)data;
  gtk_editable_set_text(GTK_EDITABLE(widgets->username_entry), "");
  gtk_editable_set_text(GTK_EDITABLE(widgets->password_entry), "");
  gtk_editable_set_text(GTK_EDITABLE(widgets->other_entry), "");
  g_print("Input fields cleared.\n");
}

static void activate_cb(GtkApplication *app) {
  GtkWidget *window = gtk_application_window_new(app);
  AppWidgets *widgets = g_slice_new(AppWidgets);

  gtk_window_set_title(GTK_WINDOW(window), "IPTV Stream download");
  gtk_window_set_default_size(GTK_WINDOW(window), 300, 300);
  // gtk_window_set_child(GTK_WINDOW(window), label);

  GtkWidget *header = adw_header_bar_new();
  adw_header_bar_set_title_widget(ADW_HEADER_BAR(header),
                                  adw_window_title_new("Time", NULL));

  GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
  gtk_widget_set_margin_start(box, 15);
  gtk_widget_set_margin_end(box, 15);
  gtk_widget_set_margin_top(box, 15);
  gtk_widget_set_margin_bottom(box, 15);
  gtk_window_set_child(GTK_WINDOW(window), box);

  gtk_box_append(GTK_BOX(box), GTK_WIDGET(header));

  // --- Input Field 1: Username ---
  widgets->username_entry = GTK_ENTRY(gtk_entry_new());
  gtk_entry_set_placeholder_text(widgets->username_entry, "Username");
  gtk_box_append(GTK_BOX(box), GTK_WIDGET(widgets->username_entry));

  // --- Input Field 2: Password ---
  widgets->password_entry = GTK_ENTRY(gtk_entry_new());
  gtk_entry_set_placeholder_text(widgets->password_entry, "Password");
  gtk_entry_set_visibility(widgets->password_entry, FALSE); // Masks the input
  gtk_box_append(GTK_BOX(box), GTK_WIDGET(widgets->password_entry));

  // --- Input Field 3: Other Info ---
  widgets->other_entry = GTK_ENTRY(gtk_entry_new());
  gtk_entry_set_placeholder_text(widgets->other_entry, "URL");
  gtk_box_append(GTK_BOX(box), GTK_WIDGET(widgets->other_entry));

  widgets->clock_label = GTK_LABEL(gtk_label_new(NULL));
  gtk_widget_add_css_class(GTK_WIDGET(widgets->clock_label),
                           "title-4"); // Use a nice Adwaita style for the text
  adw_header_bar_pack_end(ADW_HEADER_BAR(header),
                          GTK_WIDGET(widgets->clock_label));

  // Set initial time and start the timer to update it every second.
  update_clock(widgets->clock_label);
  widgets->clock_source_id =
      g_timeout_add_seconds(1, update_clock, widgets->clock_label);

  // --- Regular Button 1: Submit ---
  GtkWidget *submit_button = gtk_button_new_with_label("Record");
  g_signal_connect(submit_button, "clicked", G_CALLBACK(submit_button_clicked),
                   widgets);
  gtk_box_append(GTK_BOX(box), submit_button);

  // --- Regular Button 2: Clear ---
  GtkWidget *clear_button = gtk_button_new_with_label("Clear");
  g_signal_connect(clear_button, "clicked", G_CALLBACK(clear_button_clicked),
                   widgets);
  gtk_box_append(GTK_BOX(box), clear_button);

  // --- Exit Button ---
  GtkWidget *exit_button = gtk_button_new_with_label("Exit");
  gtk_widget_set_valign(exit_button, GTK_ALIGN_END); // Push to the bottom
  gtk_widget_set_vexpand(exit_button, TRUE);
  // Connect the "clicked" signal to the gtk_window_close function.
  g_signal_connect_swapped(exit_button, "clicked", G_CALLBACK(gtk_window_close),
                           window);
  gtk_box_append(GTK_BOX(box), exit_button);

  gtk_window_present(GTK_WINDOW(window));
}

int main(int argc, char *argv[]) {
  g_autoptr(AdwApplication) app = NULL;

  app = adw_application_new("org.example.Hello", G_APPLICATION_FLAGS_NONE);

  g_signal_connect(app, "activate", G_CALLBACK(activate_cb), NULL);

  return g_application_run(G_APPLICATION(app), argc, argv);
}
