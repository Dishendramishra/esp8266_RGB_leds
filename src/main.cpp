#define FASTLED_ESP8266_RAW_PIN_ORDER
#include <FastLED.h>
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WebSocketsServer.h>
#include <ESP8266WebServer.h>

ESP8266WebServer server;
WebSocketsServer webSocket = WebSocketsServer(81);

#define NUM_OF_LEDS 15
#define DATA_PIN 2 // PIN D4

// Define the array of leds
CRGB leds[NUM_OF_LEDS];

char main[] PROGMEM = R"=====(<!DOCTYPE html><html><head> <style> .center { width: auto; margin-left: auto; margin-right: auto; } html, body { height: 100%; margin: 0; } .full-height { height: 100%; } </style> <script> var Socket; function init() { Socket = new WebSocket('ws://' + window.location.hostname + ':81/'); Socket.onmessage = function (event) { console.log(event.data); } } function getColor(eid) { t = document.getElementById(eid); Socket.send(t.style.backgroundColor); } </script></head><body onload="javascript:init()"> <div class="full-height"> <table border="1" style="width:100%;" class="full-height"> <tr height="100"> <td id="Tomato" onclick="getColor('Tomato');" style="background-color:Tomato;"></td> <td id="Orange" onclick="getColor('Orange');" style="background-color:Orange;"> </td> </tr> <tr height="100"> <td id="DodgerBlue" onclick="getColor('DodgerBlue');" style="background-color:DodgerBlue;"> </td> <td id="MediumSeaGreen" onclick="getColor('MediumSeaGreen');" style="background-color:MediumSeaGreen;"> </td> </tr> <tr height="100"> <td id="Violet" onclick="getColor('Violet');" style="background-color:Violet;"> </td> <td id="SlateBlue" onclick="getColor('SlateBlue');" style="background-color:SlateBlue;"> </td> </tr> <tr height="100"> <td id="White" onclick="getColor('White');" style="background-color:White;"> </td> <td id="Black" onclick="getColor('Black');" style="background-color:Black;"> </td> </tr> </table> </div></body></html>)=====";
char slider[] PROGMEM = R"=====(<!DOCTYPE html>
<html>

<head>
    <style>
        .center {
            margin-left: auto;
            margin-right: auto;
        }

        html,
        body {
            height: 100%;
            margin: 0;
        }

        .full-height {
            height: 100%;
        }

        input {
            background: linear-gradient(to right, #696e6e 0%, #696e6e 50%, #fff 50%, #fff 100%);
            border: solid 1px #696e6e;
            border-radius: 10px;
            height: 100%;
            width: 100%;
            outline: none;
            transition: background 450ms ease-in;
            -webkit-appearance: none;
        }
    </style>
</head>

<body onload="javascript:init()">
    <div class="full-height">
        <table class="full-height center" width="80%">
            <tr>
                <td> <input id="red" type="range" value="127" min=0 max=255
                        style="background: linear-gradient(to right, #ff6347 0%, #ff6347 50%, #fff 50%, #fff 100%);" />
                </td>
            </tr>
            <tr>
                <td> <input id="green" type="range" value="127" min=0 max=255
                        style="background: linear-gradient(to right, #3cb371 0%, #3cb371 50%, #fff 50%, #fff 100%);" />
                </td>
            </tr>
            <tr>
                <td> <input id="blue" type="range" value="127" min=0 max=255
                        style="background: linear-gradient(to right, #1e90ff 0%, #1e90ff 50%, #fff 50%, #fff 100%);" />
                </td>
            </tr>
            <tr>
                <td rowspan="2" colspan="2">
                    <div id="pellete"
                        style="background-color:rgb(127,127,127); height: 100%; width: 100%; margin-left: auto; margin-right: auto;">
                    </div>
                </td>
            </tr>
        </table>
    </div>
</body>
<script>

    red = document.getElementById("red")
    blue = document.getElementById("green")
    green = document.getElementById("blue")


    red.oninput = function () {
        this.style.background = 'linear-gradient(to right, #ff6347 0%, #ff6347 ' + this.value / this.max * 100 + '%, #fff ' + this.value / this.max * 100 + '%, white 100%)'
        generateColor();
    };
    blue.oninput = function () {
        this.style.background = 'linear-gradient(to right, #3cb371 0%, #3cb371 ' + this.value / this.max * 100 + '%, #fff ' + this.value / this.max * 100 + '%, white 100%)'
        generateColor();
    };
    green.oninput = function () {
        this.style.background = 'linear-gradient(to right, #1e90ff 0%, #1e90ff ' + this.value / this.max * 100 + '%, #fff ' + this.value / this.max * 100 + '%, white 100%)'
        generateColor();
    };

    var Socket;

    Number.prototype.pad = function (size) {
        var s = String(this);
        while (s.length < (size || 2)) { s = "0" + s; }
        return s;
    }

    function init() {
        Socket = new WebSocket('ws://' + window.location.hostname + ':81/');
        Socket.onmessage = function (event) {
            console.log(event.data);
        }
    }

    function generateColor() {
        color = document.getElementById("pellete");
        color.style.background = "rgb(" + parseInt((red.value)).pad(3) + "," + (parseInt(blue.value)).pad(3) + "," + (parseInt(green.value)).pad(3) + ")";
        Socket.send(parseInt((red.value)).pad(3) + "" + (parseInt(blue.value)).pad(3) + "" + (parseInt(green.value)).pad(3));
        console.log(parseInt((red.value)).pad(3) + "" + (parseInt(blue.value)).pad(3) + "" + (parseInt(green.value)).pad(3));
    }
</script>

</html>)=====";

void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length)
{
  if (type == WStype_TEXT)
  {
    char *cmd = (char *)payload;
    String command = cmd;

    if (command == "tomato")
    {
      fill_solid(leds, NUM_OF_LEDS, CRGB(255, 0, 0));
      FastLED.show();
      Serial.print("Color: ");
      Serial.println(command);
    }
    else if (command == "orange")
    {
      fill_solid(leds, NUM_OF_LEDS, CRGB::Orange);
      FastLED.show();
      Serial.print("Color: ");
      Serial.println(command);
    }
    else if (command == "dodgerblue")
    {
      fill_solid(leds, NUM_OF_LEDS, CRGB(0, 0, 255));
      FastLED.show();
      Serial.print("Color: ");
      Serial.println(command);
    }
    else if (command == "mediumseagreen")
    {
      fill_solid(leds, NUM_OF_LEDS, CRGB(0, 255, 0));
      FastLED.show();
      Serial.print("Color: ");
      Serial.println(command);
    }
    else if (command == "violet")
    {
      fill_solid(leds, NUM_OF_LEDS, CRGB::DarkMagenta);
      FastLED.show();
      Serial.print("Color: ");
      Serial.println(command);
    }
    else if (command == "slateblue")
    {
      fill_solid(leds, NUM_OF_LEDS, CRGB::Purple);
      FastLED.show();
      Serial.print("Color: ");
      Serial.println(command);
    }
    else if (command == "white")
    {
      fill_solid(leds, NUM_OF_LEDS, CRGB(255, 255, 255));
      FastLED.show();
      Serial.print("Color: ");
      Serial.println(command);
    }
    else if (command == "black")
    {
      fill_solid(leds, NUM_OF_LEDS, CRGB(0, 0, 0));
      FastLED.show();
      Serial.print("Color: ");
      Serial.println(command);
    }
    else
    {
      int colors = command.toInt();
      int b = colors%1000;
      colors = colors/1000;
      int g = colors%1000;
      colors = colors/1000;
      Serial.println(colors);
      Serial.println(g);
      Serial.println(b);
      fill_solid(leds, NUM_OF_LEDS, CRGB(colors,g,b));
      FastLED.show();
    }
  }
  else if (type == WStype_CONNECTED)
  {
    IPAddress ip = webSocket.remoteIP(num);
    Serial.printf("Connected from %d.%d.%d.%d\n", ip[0], ip[1], ip[2], ip[3]);
    Serial.println();
  }
}

void setup()
{
  Serial.begin(115200);
  Serial.println();

  Serial.print("Setting soft-AP ... ");
  boolean result = WiFi.softAP("RGB_LEDs", "havealotoffun");
  if (result == true)
  {
    Serial.println("Ready");
  }
  else
  {
    Serial.println("Failed!");
  }

  FastLED.addLeds<SM16703, DATA_PIN, BRG>(leds, NUM_OF_LEDS);

  server.on("/", []() {
    server.send_P(200, "text/html", main);
  });

  server.on("/slider", []() {
    server.send_P(200, "text/html", slider);
  });

  server.begin();
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
}

void loop()
{
  webSocket.loop();
  server.handleClient();
}