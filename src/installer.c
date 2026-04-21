#include "installer.h"
#include <gio/gio.h>

typedef struct {
    gint ref_count;
    InstallProgressCallback progress_cb;
    InstallFinishedCallback finished_cb;
    gpointer user_data;
    GSubprocess *subprocess;
    GDataInputStream *stream;
} InstallTask;

static InstallTask *install_task_ref(InstallTask *task) {
    g_atomic_int_inc(&task->ref_count);
    return task;
}

static void install_task_unref(InstallTask *task) {
    if (g_atomic_int_dec_and_test(&task->ref_count)) {
        g_clear_object(&task->stream);
        g_clear_object(&task->subprocess);
        g_free(task);
    }
}

static void on_line_read(GObject *source_object, GAsyncResult *res, gpointer user_data) {
    InstallTask *task = user_data;
    gsize length;
    GError *error = NULL;
    char *line = g_data_input_stream_read_line_finish(task->stream, res, &length, &error);
    
    if (line != NULL) {
        if (task->progress_cb) {
            task->progress_cb(line, task->user_data);
        }
        g_free(line);
        g_data_input_stream_read_line_async(task->stream, G_PRIORITY_DEFAULT, NULL, on_line_read, task);
    } else {
        if (error) {
            g_clear_error(&error);
        }
        install_task_unref(task);
    }
}

static void on_process_finished(GObject *source_object, GAsyncResult *res, gpointer user_data) {
    InstallTask *task = user_data;
    GError *error = NULL;
    gboolean success = g_subprocess_wait_finish(G_SUBPROCESS(source_object), res, &error);
    
    if (error) {
        g_warning("Process error: %s", error->message);
        g_clear_error(&error);
        success = FALSE;
    } else {
        success = g_subprocess_get_if_exited(task->subprocess) && (g_subprocess_get_exit_status(task->subprocess) == 0);
    }
    
    if (task->finished_cb) {
        task->finished_cb(success, task->user_data);
    }
    
    install_task_unref(task);
}

void install_app_async(const char *command, InstallProgressCallback progress_cb, InstallFinishedCallback finished_cb, gpointer user_data) {
    InstallTask *task = g_new0(InstallTask, 1);
    task->ref_count = 1;
    task->progress_cb = progress_cb;
    task->finished_cb = finished_cb;
    task->user_data = user_data;
    
    GError *error = NULL;
    task->subprocess = g_subprocess_new(G_SUBPROCESS_FLAGS_STDOUT_PIPE | G_SUBPROCESS_FLAGS_STDERR_MERGE,
                                        &error,
                                        "bash", "-c", command, NULL);
    
    if (error) {
        g_warning("Could not launch subprocess: %s", error->message);
        g_clear_error(&error);
        if (finished_cb) finished_cb(FALSE, user_data);
        install_task_unref(task);
        return;
    }
    
    GInputStream *stdout_stream = g_subprocess_get_stdout_pipe(task->subprocess);
    task->stream = g_data_input_stream_new(stdout_stream);
    
    install_task_ref(task);
    g_data_input_stream_read_line_async(task->stream, G_PRIORITY_DEFAULT, NULL, on_line_read, task);
    
    install_task_ref(task);
    g_subprocess_wait_async(task->subprocess, NULL, on_process_finished, task);
    
    install_task_unref(task);
}
