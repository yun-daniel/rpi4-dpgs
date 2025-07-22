#include "tls_helper.h"
#include <gio/gio.h>

GTlsDatabase* load_tls_database(const char *ca_path)
{
    GError *error = NULL;
    GTlsDatabase *db = g_tls_file_database_new(ca_path, &error);
    if (!db) {
        g_printerr("load_tls_database error: %s\n",
                   error ? error->message : "unknown");
        if (error) g_error_free(error);
    }
    return db;
}
