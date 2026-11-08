/*
 * Blood Bank Management System
 * Menu-driven terminal application in C with MySQL
 *
 * Features: Add, View, Search (by blood group / name), Update, Delete,
 *           Blood group summary, Exit
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifdef _WIN32
#include <winsock2.h>   /* must come before mysql.h on Windows */
#endif
#include <mysql.h>

/* Database settings (host, user, password, ...) are kept in config.h.
   config.h is NOT uploaded to Git, so the password stays private.
   Copy config.example.h to config.h and edit it. */
#include "config.h"

MYSQL *conn;

/* ---------- Helper functions ---------- */

void read_line(const char *prompt, char *buf, size_t size)
{
    printf("%s", prompt);
    if (fgets(buf, (int)size, stdin) == NULL) {
        buf[0] = '\0';
        return;
    }
    buf[strcspn(buf, "\r\n")] = '\0';
}

int read_int(const char *prompt)
{
    char buf[32];
    read_line(prompt, buf, sizeof(buf));
    return atoi(buf);
}

/* Makes text safe to place inside an SQL string (prevents SQL injection).
   dest must be at least 2 * strlen(src) + 1 bytes. */
void escape(const char *src, char *dest)
{
    mysql_real_escape_string(conn, dest, src, (unsigned long)strlen(src));
}

void to_upper(char *s)
{
    for (; *s; s++)
        *s = (char)toupper((unsigned char)*s);
}

int valid_blood_group(const char *bg)
{
    const char *groups[] = {"A+", "A-", "B+", "B-", "AB+", "AB-", "O+", "O-"};
    for (int i = 0; i < 8; i++)
        if (strcmp(bg, groups[i]) == 0)
            return 1;
    return 0;
}

/* Runs a SELECT query and prints the result as a table. */
void show_query(const char *sql)
{
    if (mysql_query(conn, sql)) {
        printf("Query failed: %s\n", mysql_error(conn));
        return;
    }

    MYSQL_RES *res = mysql_store_result(conn);
    if (res == NULL) {
        printf("Could not read result: %s\n", mysql_error(conn));
        return;
    }

    unsigned int cols = mysql_num_fields(res);
    MYSQL_FIELD *fields = mysql_fetch_fields(res);

    printf("\n");
    for (unsigned int i = 0; i < cols; i++)
        printf("%-16s", fields[i].name);
    printf("\n");
    for (unsigned int i = 0; i < cols * 16; i++)
        printf("-");
    printf("\n");

    MYSQL_ROW row;
    unsigned long count = 0;
    while ((row = mysql_fetch_row(res)) != NULL) {
        for (unsigned int i = 0; i < cols; i++)
            printf("%-16s", row[i] ? row[i] : "-");
        printf("\n");
        count++;
    }

    if (count == 0)
        printf("No records found.\n");
    else
        printf("\n%lu record(s) found.\n", count);

    mysql_free_result(res);
}

/* ---------- Menu operations ---------- */

void add_donor(void)
{
    char name[51], gender[8], bg[8], phone[16], city[41], date[16];
    char e_name[110], e_gender[20], e_bg[20], e_phone[40], e_city[90], e_date[40];
    char date_sql[64];
    char sql[800];

    printf("\n--- Add New Donor ---\n");
    read_line("Name: ", name, sizeof(name));
    if (strlen(name) == 0) { printf("Name cannot be empty.\n"); return; }

    int age = read_int("Age (18-65): ");
    if (age < 18 || age > 65) { printf("Donor age must be between 18 and 65.\n"); return; }

    read_line("Gender (M/F/O): ", gender, sizeof(gender));
    to_upper(gender);
    if (strcmp(gender, "M") != 0 && strcmp(gender, "F") != 0 && strcmp(gender, "O") != 0) {
        printf("Gender must be M, F or O.\n");
        return;
    }

    read_line("Blood group (A+, A-, B+, B-, AB+, AB-, O+, O-): ", bg, sizeof(bg));
    to_upper(bg);
    if (!valid_blood_group(bg)) { printf("Invalid blood group.\n"); return; }

    read_line("Phone: ", phone, sizeof(phone));
    if (strlen(phone) == 0) { printf("Phone cannot be empty.\n"); return; }

    read_line("City: ", city, sizeof(city));
    if (strlen(city) == 0) { printf("City cannot be empty.\n"); return; }

    read_line("Last donation date (YYYY-MM-DD, press Enter to skip): ", date, sizeof(date));

    escape(name, e_name);
    escape(gender, e_gender);
    escape(bg, e_bg);
    escape(phone, e_phone);
    escape(city, e_city);

    if (strlen(date) == 0) {
        strcpy(date_sql, "NULL");
    } else {
        escape(date, e_date);
        snprintf(date_sql, sizeof(date_sql), "'%s'", e_date);
    }

    snprintf(sql, sizeof(sql),
             "INSERT INTO donors (name, age, gender, blood_group, phone, city, last_donation) "
             "VALUES ('%s', %d, '%s', '%s', '%s', '%s', %s)",
             e_name, age, e_gender, e_bg, e_phone, e_city, date_sql);

    if (mysql_query(conn, sql))
        printf("Could not add donor: %s\n", mysql_error(conn));
    else
        printf("Donor added successfully! New ID = %llu\n",
               (unsigned long long)mysql_insert_id(conn));
}

void view_donors(void)
{
    printf("\n--- All Donors ---");
    show_query("SELECT id, name, age, gender, blood_group, phone, city, last_donation "
               "FROM donors ORDER BY id");
}

void search_by_blood_group(void)
{
    char bg[8], e_bg[20], sql[300];

    printf("\n--- Search by Blood Group ---\n");
    read_line("Enter blood group: ", bg, sizeof(bg));
    to_upper(bg);
    if (!valid_blood_group(bg)) { printf("Invalid blood group.\n"); return; }

    escape(bg, e_bg);
    snprintf(sql, sizeof(sql),
             "SELECT id, name, age, gender, blood_group, phone, city, last_donation "
             "FROM donors WHERE blood_group = '%s' ORDER BY name", e_bg);
    show_query(sql);
}

void search_by_name(void)
{
    char name[51], e_name[110], sql[400];

    printf("\n--- Search by Name ---\n");
    read_line("Enter name (or part of name): ", name, sizeof(name));
    if (strlen(name) == 0) { printf("Nothing entered.\n"); return; }

    escape(name, e_name);
    snprintf(sql, sizeof(sql),
             "SELECT id, name, age, gender, blood_group, phone, city, last_donation "
             "FROM donors WHERE name LIKE '%%%s%%' ORDER BY name", e_name);
    show_query(sql);
}

void update_donor(void)
{
    char value[60], e_value[130], sql[400];

    printf("\n--- Update Donor ---\n");
    int id = read_int("Enter donor ID to update: ");
    if (id <= 0) { printf("Invalid ID.\n"); return; }

    printf("What do you want to update?\n");
    printf("  1. Phone\n  2. City\n  3. Last donation date\n");
    int choice = read_int("Choice: ");

    const char *column;
    if (choice == 1) column = "phone";
    else if (choice == 2) column = "city";
    else if (choice == 3) column = "last_donation";
    else { printf("Invalid choice.\n"); return; }

    read_line("New value: ", value, sizeof(value));
    if (strlen(value) == 0) { printf("Value cannot be empty.\n"); return; }

    escape(value, e_value);
    snprintf(sql, sizeof(sql),
             "UPDATE donors SET %s = '%s' WHERE id = %d", column, e_value, id);

    if (mysql_query(conn, sql)) {
        printf("Could not update: %s\n", mysql_error(conn));
        return;
    }

    if (mysql_affected_rows(conn) == 0)
        printf("No record changed (ID not found, or value is the same).\n");
    else
        printf("Donor updated successfully!\n");
}

void delete_donor(void)
{
    char confirm[8], sql[100];

    printf("\n--- Delete Donor ---\n");
    int id = read_int("Enter donor ID to delete: ");
    if (id <= 0) { printf("Invalid ID.\n"); return; }

    read_line("Are you sure? (y/n): ", confirm, sizeof(confirm));
    if (confirm[0] != 'y' && confirm[0] != 'Y') {
        printf("Delete cancelled.\n");
        return;
    }

    snprintf(sql, sizeof(sql), "DELETE FROM donors WHERE id = %d", id);
    if (mysql_query(conn, sql)) {
        printf("Could not delete: %s\n", mysql_error(conn));
        return;
    }

    if (mysql_affected_rows(conn) == 0)
        printf("No donor found with that ID.\n");
    else
        printf("Donor deleted successfully!\n");
}

void blood_summary(void)
{
    printf("\n--- Donors Available per Blood Group ---");
    show_query("SELECT blood_group, COUNT(*) AS total_donors "
               "FROM donors GROUP BY blood_group ORDER BY blood_group");
}

/* ---------- Main ---------- */

int main(void)
{
    conn = mysql_init(NULL);
    if (conn == NULL) {
        printf("mysql_init failed.\n");
        return 1;
    }

    if (mysql_real_connect(conn, DB_HOST, DB_USER, DB_PASS, DB_NAME,
                           DB_PORT, NULL, 0) == NULL) {
        printf("Database connection failed: %s\n", mysql_error(conn));
        mysql_close(conn);
        return 1;
    }
    printf("Connected to MySQL database '%s'.\n", DB_NAME);

    int choice;
    do {
        printf("\n=============================\n");
        printf("   BLOOD BANK MANAGEMENT\n");
        printf("=============================\n");
        printf(" 1. Add new donor\n");
        printf(" 2. View all donors\n");
        printf(" 3. Search by blood group\n");
        printf(" 4. Search by name\n");
        printf(" 5. Update donor\n");
        printf(" 6. Delete donor\n");
        printf(" 7. Blood group summary\n");
        printf(" 8. Exit\n");
        choice = read_int("Enter your choice (1-8): ");

        switch (choice) {
            case 1: add_donor();            break;
            case 2: view_donors();          break;
            case 3: search_by_blood_group(); break;
            case 4: search_by_name();       break;
            case 5: update_donor();         break;
            case 6: delete_donor();         break;
            case 7: blood_summary();        break;
            case 8: printf("Goodbye!\n");   break;
            default: printf("Invalid choice. Please enter 1-8.\n");
        }
    } while (choice != 8);

    mysql_close(conn);
    return 0;
}
