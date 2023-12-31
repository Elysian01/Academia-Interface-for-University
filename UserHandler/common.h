#ifndef COMMON_FUNCTIONS
#define COMMON_FUNCTIONS

#include <stdio.h>     // Import for `printf` & `perror`
#include <unistd.h>    // Import for `read`, `write & `lseek`
#include <string.h>    // Import for string functions
#include <stdbool.h>   // Import for `bool` data type
#include <sys/types.h> // Import for `open`, `lseek`
#include <sys/stat.h>  // Import for `open`
#include <fcntl.h>     // Import for `open`
#include <stdlib.h>    // Import for `atoi`
#include <errno.h>     // Import for `errno`

#include "../Model/model.h"
#include "../config.h"
#include "../DAL/studentRepo.h"
#include "../DAL/facultyRepo.h"
#include "../DAL/courseRepo.h"

// Function Prototypes =================================

int login_handler(UserType user, int connFD);
Faculty getFacultyByLoginId(char *loginId);
Student getStudentByLoginId(char *loginId);

// =====================================================

// Function Definition =================================

int login_handler(UserType user, int connFD)
{
    ssize_t readBytes, writeBytes;            // Number of bytes written to / read from the socket
    char readBuffer[1000], writeBuffer[1000]; // Buffer for reading from / writing to the client
    char tempBuffer[1000];

    Student student;
    Faculty faculty;

    bzero(readBuffer, sizeof(readBuffer));
    bzero(writeBuffer, sizeof(writeBuffer));

    // Get login message for respective user type
    if (user == ADMIN)
        strcpy(writeBuffer, ADMIN_LOGIN_WELCOME);
    else if (user == FACULTY)
        strcpy(writeBuffer, FACULTY_LOGIN_WELCOME);

    // Append the request for LOGIN ID message
    strcat(writeBuffer, "\n");
    strcat(writeBuffer, LOGIN_ID);

    writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
    if (writeBytes == -1)
    {
        perror("Error writing WELCOME & LOGIN_ID message to the client!");
        return 0;
    }

    // In the below readBytes, we will get the login-id
    readBytes = read(connFD, readBuffer, sizeof(readBuffer));
    if (readBytes == -1)
    {
        perror("Error reading login ID from client!");
        return false;
    }

    bool userFound = false;

    // checking is user exist or not
    if (user == ADMIN)
    {
        if (strcmp(readBuffer, ADMIN_LOGIN_ID) == 0)
            userFound = true;
    }
    else if (user == FACULTY)
    {
        faculty = getFacultyByLoginId(readBuffer);
        if (faculty.id != -1)
            userFound = true;
    }
    else if (user == STUDENT)
    {
        // if the user is student then we have to check whether he is active or not
        student = getStudentByLoginId(readBuffer);
        if (student.id != -1 && student.isActive)
            userFound = true;
    }

    if (userFound)
    {
        bzero(writeBuffer, sizeof(writeBuffer));
        // if user exist only then we ask for password
        writeBytes = write(connFD, PASSWORD, strlen(PASSWORD));
        if (writeBytes == -1)
        {
            perror("Error writing PASSWORD message to client!");
            return 0;
        }

        bzero(readBuffer, sizeof(readBuffer));
        // get the password from client
        readBytes = read(connFD, readBuffer, sizeof(readBuffer));
        if (readBytes == -1)
        {
            perror("Error reading password from the client!");
            return 0;
        }

        char hashedPassword[1000];
        printf("%s %s\n", readBuffer, crypt(readBuffer, SALT_BAE));
        strcpy(hashedPassword, crypt(readBuffer, SALT_BAE));

        if (user == ADMIN)
        {
            if (strcmp(hashedPassword, ADMIN_PASSWORD) == 0)
                return 1;
        }
        if (user == STUDENT)
        {
            if (strcmp(hashedPassword, student.password) == 0)
                return student.id;
        }
        if (user == FACULTY)
        {
            if (strcmp(hashedPassword, faculty.password) == 0)
                return faculty.id;
        }

        bzero(writeBuffer, sizeof(writeBuffer));
        writeBytes = write(connFD, INVALID_PASSWORD, strlen(INVALID_PASSWORD));
    }
    else
    {
        bzero(writeBuffer, sizeof(writeBuffer));
        writeBytes = write(connFD, INVALID_LOGIN, strlen(INVALID_LOGIN));
    }

    return false;
}

Faculty getFacultyByLoginId(char *loginId)
{
    Faculty faculty;
    int totalFaculty = getNextFacultyId(); // returns total number of faculty
    for (int i = 1; i < totalFaculty; i++)
    {
        faculty = getFacultyById(i);
        if (strcmp(faculty.login, loginId) == 0)
        {
            return faculty;
        }
    }
    faculty.id = -1;
    return faculty;
}

Student getStudentByLoginId(char *loginId)
{
    Student student;
    int totalStudent = getNextStudentId();
    for (int i = 1; i < totalStudent; i++)
    {
        student = getStudentById(i);
        if (strcmp(student.login, loginId) == 0)
        {
            return student;
        }
    }
    student.id = -1;
    return student;
}

// =====================================================

#endif