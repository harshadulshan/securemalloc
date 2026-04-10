#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "db_logger.h"
#include "sqlite3.h"

static sqlite3* db = NULL;

void db_init(const char* path) {
    int rc = sqlite3_open(path, &db);
    if (rc != SQLITE_OK) {
        printf("[DB] ERROR: Cannot open database: %s\n", sqlite3_errmsg(db));
        db = NULL;
        return;
    }

    const char* sql =
        "CREATE TABLE IF NOT EXISTS allocations ("
        "  id       INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  address  TEXT,"
        "  size     INTEGER,"
        "  file     TEXT,"
        "  line     INTEGER,"
        "  status   TEXT,"
        "  time     DATETIME DEFAULT CURRENT_TIMESTAMP"
        ");";

    char* err = NULL;
    rc = sqlite3_exec(db, sql, NULL, NULL, &err);
    if (rc != SQLITE_OK) {
        printf("[DB] ERROR creating table: %s\n", err);
        sqlite3_free(err);
        return;
    }

    printf("[DB] Database ready: %s\n", path);
}

void db_log_alloc(void* address, size_t size, const char* file, int line) {
    if (db == NULL) return;

    char sql[512];
    snprintf(sql, sizeof(sql),
        "INSERT INTO allocations (address, size, file, line, status) "
        "VALUES ('%p', %zu, '%s', %d, 'LIVE');",
        address, size, file, line);

    char* err = NULL;
    int rc = sqlite3_exec(db, sql, NULL, NULL, &err);
    if (rc != SQLITE_OK) {
        printf("[DB] ERROR logging alloc: %s\n", err);
        sqlite3_free(err);
    }
}

void db_log_free(void* address) {
    if (db == NULL) return;

    char sql[256];
    snprintf(sql, sizeof(sql),
        "UPDATE allocations SET status = 'FREED' "
        "WHERE address = '%p' AND status = 'LIVE';",
        address);

    char* err = NULL;
    int rc = sqlite3_exec(db, sql, NULL, NULL, &err);
    if (rc != SQLITE_OK) {
        printf("[DB] ERROR logging free: %s\n", err);
        sqlite3_free(err);
    }
}

static int print_row(void* unused, int cols, char** values, char** names) {
    (void)unused;
    for (int i = 0; i < cols; i++) {
        printf("%-12s: %s\n", names[i], values[i] ? values[i] : "NULL");
    }
    printf("---\n");
    return 0;
}

void db_report() {
    if (db == NULL) return;

    printf("\n=== DATABASE ALLOCATION REPORT ===\n");

    /* Summary */
    const char* summary =
        "SELECT status, COUNT(*) as count, SUM(size) as total_bytes "
        "FROM allocations GROUP BY status;";

    char* err = NULL;
    sqlite3_exec(db, summary, print_row, NULL, &err);
    if (err) { sqlite3_free(err); }

    /* Live leaks */
    printf("\n-- LIVE (not freed) --\n");
    const char* leaks =
        "SELECT address, size, file, line FROM allocations WHERE status='LIVE';";
    sqlite3_exec(db, leaks, print_row, NULL, &err);
    if (err) { sqlite3_free(err); }

    printf("==================================\n\n");
}

void db_close() {
    if (db != NULL) {
        sqlite3_close(db);
        db = NULL;
        printf("[DB] Database closed.\n");
    }
}