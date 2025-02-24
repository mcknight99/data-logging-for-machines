#ifndef DATETIME_HANDLER_H
#define DATETIME_HANDLER_H

#include <HTTPClient.h>
#include <ctime>
#include "VARS.h"
#include "auth.h"
#include "wifi_handler.h"

struct DataLog{
    String on_time;
    String off_time;
    String uptime;
};

// Global Variable Declaration; do not touch
extern std::vector<DataLog> data_log_backlog;

String sendDateTimeRequest(); // Send API request for date and time using IP address
String extractTime(String payload); // Extract time string from payload in format HH:MM:SS
String extractDate(String payload); // Extract date string from payload in format YYYY-MM-DD
String getFormattedDateTime(); // Get formatted date and time string as "YYYY-MM-DD HH:MM:SS"
String getDeltaTime(String on, String off); // Get delta time string as "HH:MM:SS" from on and off time strings
bool sendToWebApp(String on_time, String off_time, String uptime); // Send on time, off time, and uptime to web app
bool uploadHandler(String on_time, String off_time, String uptime); // Upload handler for sending data to web app

#endif
