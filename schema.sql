-- Blood Bank Management System: database setup
-- Run this once in MySQL Command Line Client or MySQL Workbench.

CREATE DATABASE IF NOT EXISTS bloodbank;
USE bloodbank;

CREATE TABLE IF NOT EXISTS donors (
    id            INT AUTO_INCREMENT PRIMARY KEY,
    name          VARCHAR(50)  NOT NULL,
    age           INT          NOT NULL,
    gender        CHAR(1)      NOT NULL,
    blood_group   VARCHAR(3)   NOT NULL,
    phone         VARCHAR(15)  NOT NULL,
    city          VARCHAR(40)  NOT NULL,
    last_donation DATE         NULL
);

-- Optional sample data so you can test View/Search immediately
INSERT INTO donors (name, age, gender, blood_group, phone, city, last_donation) VALUES
('Anil Kumar', 28, 'M', 'O+',  '9876543210', 'Kochi',     '2026-06-15'),
('Meera Nair', 24, 'F', 'A+',  '9847012345', 'Kottayam',  '2026-08-02'),
('Joseph Mathew', 35, 'M', 'B-', '9995551234', 'Thrissur', NULL);
