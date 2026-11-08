# Blood Bank Management System

A menu-driven terminal application in C with MySQL (MCA mini project).

## Features
- Add, view, search (by blood group / name), update and delete donors
- Blood group summary

## Requirements
- gcc (MSYS2 UCRT64) and MySQL Server 8.0 (64-bit) on Windows

## Setup
1. Create the database: in MySQL run `source C:/bloodbank/schema.sql`
2. Copy `config.example.h` to `config.h` and set your MySQL password
3. Compile:
   ```
   $MYSQL = "C:\Program Files\MySQL\MySQL Server 8.0"
   gcc bloodbank.c -o bloodbank.exe -I "$MYSQL\include" -L "$MYSQL\lib" -lmysql
   ```
4. Copy `libmysql.dll`, `libssl*.dll`, `libcrypto*.dll` from the MySQL `lib`/`bin` folders next to the exe
5. Run `.\bloodbank.exe`
