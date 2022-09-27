#ifndef htmlFiles_h
#define htmlFiles_h

#include <Arduino.h>

const char indexHTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>

<html>

<head>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <title>Recording Sign Setup</title>
    <style>
        html {
            font-family: Helvetica;
            display: inline-block;
            margin: 0px auto;
            text-align: center;
        }

        body {
            margin-top: 50px;
        }

        h1 {
            color: #444444;
            margin: 50px auto 30px;
        }

        p {
            font-size: 14px;
            color: #888;
            margin-bottom: 10px;
        }

        label {
            font-size: 16px;
        }

        input[type=text],
        input[type=number],
        input[type=password],
        select {
            width: 80%;
            max-width: 500px;
            padding: 15px 20px;
            margin: 10px;
            margin-bottom: 30px;
            display: inline-block;
            border: 1px solid #ccc;
            border-radius: 10px;
            box-sizing: border-box;
            font-size: 16px;
            outline-color: rgb(65, 175, 255);
        }

        input[type=submit] {
            border: none;
            color: #fefbfb;
            background-color: rgb(65, 175, 255);
            padding: 15px 30px;
            margin: 20px;
            text-align: center;
            font-size: 16px;
            width: 100px;
            border-radius: 10px;
            transition-duration: 0.4s;
        }
    </style>
</head>

<body>

    <h1>RECORDING SIGN SETUP</h1>

    <form action="/get">
     
            <label for="ssid">WiFi SSID</label><br>
            <input type="text" id="ssid" name="ssid" required="true" maxlength="50"><br>

            <label for="password">WiFi Password</label><br>
            <input type="password" id="password" name="password" required="true" maxlength="50"><br>

            <label for="ipAddress">IP Address</label><br>
            <input type="text" id="ipAddress" name="ipAddress" required="true" pattern="((25[0-5]|(2[0-4]|1\d|[1-9]|)\d)\.?\b){4}"><br>

            <label for="deviceName">Device Name</label><br>
            <input type="text" id="deviceName" name="deviceName" required="true" value="Recording Sign" maxlength="30"><br>

            <label for="setupType">Setup</label><br>
            <select id="setupType" name="setupType">
                <option value="LED">LED</option>
                <option value="RGB">RGB LED / Strip</option>
                <option value="RELAY">Relay</option>
                <option value="NEOPIXEL">NeoPixel</option>
            </select><br>

            <label for="pixelCount">Number of Pixels (for NeoPixel strip only)</label><br>
            <input type="number" id="pixelCount" name="pixelCount" required="true" value="16" min="1" max="1000"><br>

            <input type="submit" value="GO!">
        
    </form>


</body>

</html>
)rawliteral";

const char infoHTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>

<html>

<head>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <title>Recording Sign Setup</title>
    <style>
        html {
            font-family: Helvetica;
            display: inline-block;
            margin: 0px auto;
            text-align: center;
        }

        body {
            margin-top: 50px;
        }

        h1 {
            color: #444444;
            margin: 50px auto 30px;
        }

        p {
            font-size: 14px;
            color: #888;
            margin-bottom: 10px;
        }
    </style>
</head>

<body>

    <h1>All Done!</h1>
    <p>The recording sign will restart and connect to your wifi network.</p>

</body>

</html>
)rawliteral";

#endif