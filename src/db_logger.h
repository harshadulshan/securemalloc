#ifndef DB_LOGGER_H
#define DB_LOGGER_H

void db_init(const char* path);
void db_log_alloc(void* address, size_t size, const char* file, int line);
void db_log_free(void* address);
void db_report();
void db_close();

#endif