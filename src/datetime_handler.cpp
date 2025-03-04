#include "datetime_handler.h"

// Function to send API request for date and time
// Returns payload from API call for dateTime
// Precondition: WiFi is connected
String sendDateTimeRequest()
{
    digitalWrite(LED_PIN, !digitalRead(LED_PIN));
    if (DEBUG)
        Serial.print("Sending API request for EST timezone (America New York)");
    
    HTTPClient http; // Declare HTTPClient object
    String httpAddress = "https://timeapi.io/api/time/current/zone?timeZone=America%2FNew_York"; // API address

    http.begin(httpAddress);           // Connect to API
    int httpResponseCode = http.GET(); // Get response from API
    String payload = http.getString(); // Get payload
    http.end();                        // Close connection

    if (DEBUG)
        Serial.println("Response code: " + String(httpResponseCode) + "\nPayload: " + payload);
    digitalWrite(LED_PIN, !digitalRead(LED_PIN));
    return payload; // Return payload
}

// Function to extract time from payload
// Returns time string in format HH:MM:SS
String extractTime(String payload)
{
    int timeIndex = payload.indexOf("dateTime") + 22;
    return payload.substring(timeIndex, timeIndex + 8);
}

// Function to extract date from payload
// Returns date string in format YYYY-MM-DD
String extractDate(String payload)
{
    int dateIndex = payload.indexOf("dateTime") + 11;
    return payload.substring(dateIndex, dateIndex + 10);
}

// Function to get formatted date and time string
// Returns formatted date and time string as "YYYY-MM-DD HH:MM:SS"
String getFormattedDateTime()
{
    String payload = sendDateTimeRequest(); // Get payload from API
    String time = extractTime(payload);     // Extract time
    String date = extractDate(payload);     // Extract date

    // save initial LED state
    bool ledState = digitalRead(LED_PIN);
    // if time or date are empty, retry API request up to API_TIMEOUT times
    int attempts = 0;
    while (time == "" || date == "")
    {
        if (attempts >= API_TIMEOUT)
        {
            return "TIMEOUT"; // Return empty string if API_TIMEOUT reached
        }
        delay(1000);                     // Delay for 1 second
        payload = sendDateTimeRequest(); // Get payload from API
        time = extractTime(payload);     // Extract time
        date = extractDate(payload);     // Extract date
        attempts++;                      // Increment attempts
        digitalWrite(LED_PIN, !digitalRead(LED_PIN));
    }
    // rewrite the LED state to what it was before
    digitalWrite(LED_PIN, ledState);
    return date + " " + time; // Return date and time
}

// Function to get delta time string from time between on and off time strings
// Returns delta time string as "HH:MM:SS" 
String getDeltaTime(String on, String off)
{
    int on_year = on.substring(0, 4).toInt();
    int on_month = on.substring(5, 7).toInt();
    int on_day = on.substring(8, 10).toInt();
    int on_hour = on.substring(11, 13).toInt();
    int on_minute = on.substring(14, 16).toInt();
    int on_second = on.substring(17, 19).toInt();

    int off_year = off.substring(0, 4).toInt();
    int off_month = off.substring(5, 7).toInt();
    int off_day = off.substring(8, 10).toInt();
    int off_hour = off.substring(11, 13).toInt();
    int off_minute = off.substring(14, 16).toInt();
    int off_second = off.substring(17, 19).toInt();

    int delta_year = off_year - on_year;
    int delta_month = off_month - on_month;
    int delta_day = off_day - on_day;
    int delta_hour = off_hour - on_hour;
    int delta_minute = off_minute - on_minute;
    int delta_second = off_second - on_second;

    if (delta_second < 0)
    {
        delta_second += 60;
        delta_minute--;
    }
    if (delta_minute < 0)
    {
        delta_minute += 60;
        delta_hour--;
    }
    if (delta_hour < 0)
    {
        delta_hour += 24;
        delta_day--;
    }
    if (delta_day < 0)
    {
        delta_day += 30;
        delta_month--;
    }
    if (delta_month < 0)
    {
        delta_month += 12;
        delta_year--;
    }

    // reformat into "HH:MM:SS", turning years, months, and days into hours, but hopefully/i doubt any machines will be on that long!
    delta_hour += delta_year * 8760 + delta_month * 730 + delta_day * 24;
    String hour_out = String(delta_hour);
    String minute_out = String(delta_minute);
    String second_out = String(delta_second);

    if (minute_out.length() == 1)
    {
        minute_out = "0" + minute_out;
    }
    if (second_out.length() == 1)
    {
        second_out = "0" + second_out;
    }

    String out = hour_out + ":" + minute_out + ":" + second_out;

    return out;
}

// Function to send machine on, machine off, and uptime data to web app
// Returns true if successful, false if not
// Precondition: WiFi is connected
bool sendToWebApp(String on_time, String off_time, String uptime)
{
    HTTPClient http;
    http.begin(WEB_APP_URL);
    http.addHeader("Content-Type", "application/json"); // Set JSON header

    // Create JSON payload
    String jsonPayload = "{\"machine\":\"" + String(MACHINE_NAME) + "\",\"on_time\":\"" + on_time + "\",\"off_time\":\"" + off_time + "\",\"uptime\":\"" + uptime + "\"}";

    if (DEBUG)
        Serial.println("Sending JSON: " + jsonPayload);

    int httpResponseCode = http.POST(jsonPayload);

    if (httpResponseCode > 0)
    {
        if (DEBUG)
            Serial.println("Response code: " + String(httpResponseCode));
        return true;
    }
    else
    {
        if (DEBUG)
        {
            Serial.println("Error sending request, response code: " + String(httpResponseCode));
            Serial.println(http.getString());
            Serial.println("WiFi status: " + String(WiFi.status()));
        }
        return false;
    }

    http.end();
    return false;
}

// Upload handler ensures proper data upload to web app using the sendToWebApp function
// Will time out after UPLOAD_TIMEOUT ms
// Returns true if successful, false if not
bool uploadHandler(String on_time, String off_time, String uptime)
{
    time_t begin_requests = millis();
    bool sent = false;

    // get led state before blinking
    bool ledState = digitalRead(LED_PIN);
    do
    {
        sent = sendToWebApp(on_time, off_time, uptime);
        if (sent)
        {
            // rewrite the LED state to what it was before
            digitalWrite(LED_PIN, ledState);
            return true;
        }
        delay(3000);
        digitalWrite(LED_PIN, !digitalRead(LED_PIN));

    } while (!sent || millis() - begin_requests < UPLOAD_TIMEOUT);

    if (!sent && DEBUG)
        Serial.println("Data upload timeout");

    // rewrite the LED state to what it was before
    digitalWrite(LED_PIN, ledState);
    return sent;
}