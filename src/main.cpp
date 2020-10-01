#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <ESP8266WiFi.h>
#include <WebSocketsServer.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <DNSServer.h>

// Pattern types supported:
enum  pattern { NONE, RAINBOW_CYCLE, THEATER_CHASE, COLOR_WIPE, SCANNER, FADE };
// Patern directions supported:
enum  direction { FORWARD, REVERSE };

// LedStrip Completion Callback
void LedStripComplete()
{}

// NeoPattern Class - derived from the Adafruit_NeoPixel class
class NeoPatterns : public Adafruit_NeoPixel
{
    public:

    // Member Variables:  
    pattern  ActivePattern;  // which pattern is running
    direction Direction;     // direction to run the pattern
    
    unsigned long Interval;   // milliseconds between updates
    unsigned long lastUpdate; // last update of position
    
    uint32_t Color1, Color2;  // What colors are in use
    uint16_t TotalSteps;  // total number of steps in the pattern
    uint16_t Index;  // current step within the pattern
    
    void (*OnComplete)();  // Callback on completion of pattern
    
    // Constructor - calls base-class constructor to initialize strip
    NeoPatterns(uint16_t pixels, uint8_t pin, uint8_t type, void (*callback)())
    :Adafruit_NeoPixel(pixels, pin, type)
    {
        OnComplete = callback;
    }
    
    // Update the pattern
    void Update()
    {
        if((millis() - lastUpdate) > Interval) // time to update
        {
            lastUpdate = millis();
            switch(ActivePattern)
            {
                case RAINBOW_CYCLE:
                    RainbowCycleUpdate();
                    break;
                case THEATER_CHASE:
                    TheaterChaseUpdate();
                    break;
                case COLOR_WIPE:
                    ColorWipeUpdate();
                    break;
                case SCANNER:
                    ScannerUpdate();
                    break;
                case FADE:
                    FadeUpdate();
                    break;
                default:
                    break;
            }
        }
    }
  
    // Increment the Index and reset at the end
    void Increment()
    {
        if (Direction == FORWARD)
        {
           Index++;
           if (Index >= TotalSteps)
            {
                Index = 0;
                if (OnComplete != NULL)
                {
                    OnComplete(); // call the comlpetion callback
                }
            }
        }
        else // Direction == REVERSE
        {
            --Index;
            if (Index <= 0)
            {
                Index = TotalSteps-1;
                if (OnComplete != NULL)
                {
                    OnComplete(); // call the comlpetion callback
                }
            }
        }
    }
    
    // Reverse pattern direction
    void Reverse()
    {
        if (Direction == FORWARD)
        {
            Direction = REVERSE;
            Index = TotalSteps-1;
        }
        else
        {
            Direction = FORWARD;
            Index = 0;
        }
    }
    
    // Initialize for a RainbowCycle
    void RainbowCycle(uint8_t interval, direction dir = FORWARD)
    {
        ActivePattern = RAINBOW_CYCLE;
        Interval = interval;
        TotalSteps = 255;
        Index = 0;
        Direction = dir;
    }
    
    // Update the Rainbow Cycle Pattern
    void RainbowCycleUpdate()
    {
        for(int i=0; i< numPixels(); i++)
        {
            setPixelColor(i, Wheel(((i * 256 / numPixels()) + Index) & 255));
        }
        show();
        Increment();
    }

    // Initialize for a Theater Chase
    void TheaterChase(uint32_t color1, uint32_t color2, uint8_t interval, direction dir = FORWARD)
    {
        ActivePattern = THEATER_CHASE;
        Interval = interval;
        TotalSteps = numPixels();
        Color1 = color1;
        Color2 = color2;
        Index = 0;
        Direction = dir;
   }
    
    // Update the Theater Chase Pattern
    void TheaterChaseUpdate()
    {
        for(int i=0; i< numPixels(); i++)
        {
            if ((i + Index) % 3 == 0)
            {
                setPixelColor(i, Color1);
            }
            else
            {
                setPixelColor(i, Color2);
            }
        }
        show();
        Increment();
    }

    // Initialize for a ColorWipe
    void ColorWipe(uint32_t color, uint8_t interval, direction dir = FORWARD)
    {
        ActivePattern = COLOR_WIPE;
        Interval = interval;
        TotalSteps = numPixels();
        Color1 = color;
        Index = 0;
        Direction = dir;
    }
    
    // Update the Color Wipe Pattern
    void ColorWipeUpdate()
    {
        setPixelColor(Index, Color1);
        show();
        Increment();
    }
    
    // Initialize for a SCANNNER
    void Scanner(uint32_t color1, uint8_t interval)
    {
        ActivePattern = SCANNER;
        Interval = interval;
        TotalSteps = (numPixels() - 1) * 2;
        Color1 = color1;
        Index = 0;
    }

    // Update the Scanner Pattern
    void ScannerUpdate()
    { 
        for (int i = 0; i < numPixels(); i++)
        {
            if (i == Index)  // Scan Pixel to the right
            {
                 setPixelColor(i, Color1);
            }
            else if (i == TotalSteps - Index) // Scan Pixel to the left
            {
                 setPixelColor(i, Color1);
            }
            else // Fading tail
            {
                 setPixelColor(i, DimColor(getPixelColor(i)));
            }
        }
        show();
        Increment();
    }
    
    // Initialize for a Fade
    void Fade(uint32_t color1, uint32_t color2, uint16_t steps, uint8_t interval, direction dir = FORWARD)
    {
        ActivePattern = FADE;
        Interval = interval;
        TotalSteps = steps;
        Color1 = color1;
        Color2 = color2;
        Index = 0;
        Direction = dir;
    }
    
    // Update the Fade Pattern
    void FadeUpdate()
    {
        // Calculate linear interpolation between Color1 and Color2
        // Optimise order of operations to minimize truncation error
        uint8_t red = ((Red(Color1) * (TotalSteps - Index)) + (Red(Color2) * Index)) / TotalSteps;
        uint8_t green = ((Green(Color1) * (TotalSteps - Index)) + (Green(Color2) * Index)) / TotalSteps;
        uint8_t blue = ((Blue(Color1) * (TotalSteps - Index)) + (Blue(Color2) * Index)) / TotalSteps;
        
        ColorSet(Color(red, green, blue));
        show();
        Increment();
    }
   
    // Calculate 50% dimmed version of a color (used by ScannerUpdate)
    uint32_t DimColor(uint32_t color)
    {
        // Shift R, G and B components one bit to the right
        uint32_t dimColor = Color(Red(color) >> 1, Green(color) >> 1, Blue(color) >> 1);
        return dimColor;
    }

    // Set all pixels to a color (synchronously)
    void ColorSet(uint32_t color)
    {
        for (int i = 0; i < numPixels(); i++)
        {
            setPixelColor(i, color);
        }
        show();
    }

    // Returns the Red component of a 32-bit color
    uint8_t Red(uint32_t color)
    {
        return (color >> 16) & 0xFF;
    }

    // Returns the Green component of a 32-bit color
    uint8_t Green(uint32_t color)
    {
        return (color >> 8) & 0xFF;
    }

    // Returns the Blue component of a 32-bit color
    uint8_t Blue(uint32_t color)
    {
        return color & 0xFF;
    }
    
    // Input a value 0 to 255 to get a color value.
    // The colours are a transition r - g - b - back to r.
    uint32_t Wheel(byte WheelPos)
    {
        WheelPos = 255 - WheelPos;
        if(WheelPos < 85)
        {
            return Color(255 - WheelPos * 3, 0, WheelPos * 3);
        }
        else if(WheelPos < 170)
        {
            WheelPos -= 85;
            return Color(0, WheelPos * 3, 255 - WheelPos * 3);
        }
        else
        {
            WheelPos -= 170;
            return Color(WheelPos * 3, 255 - WheelPos * 3, 0);
        }
    }
};


const byte DNS_PORT = 53;
IPAddress apIP(192, 168, 1, 1);
DNSServer dnsServer;
MDNSResponder mdns;
ESP8266WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);

#define NUM_OF_LEDS 15
#define DATA_PIN 2     // PIN D4
bool rainbow_flag = false;

NeoPatterns LedStrip(15, 2, NEO_BRG + NEO_KHZ400, &LedStripComplete);

char redirect[] PROGMEM = R"====(<html lang="en">
<head>
    <script>
        function redirect(){
            window.open("intent://192.168.1.1#Intent;scheme=http;package=com.android.chrome;end","_system");
        }
    </script>
</head>
<body onload="redirect()">
    <button style="height: 100%; width: 100%; font-size: 100px;" onclick="redirect()">HOME</button>
</body>
</html>)====";

char main[] PROGMEM = R"=====(<!DOCTYPE html><html> <head> <style> .unselectable { font-size: 3vw; -webkit-touch-callout: none; -webkit-user-select: none; -khtml-user-select: none; -moz-user-select: none; -ms-user-select: none; user-select: none; } .rainbow { /* height: 55px; */ background-color: red; /* For browsers that do not support gradients */ background-image: linear-gradient(to right, red, orange, yellow, green, blue, indigo, violet); } .center { width: auto; margin-left: auto; margin-right: auto; } tr { height: 20%; } td { border-radius: 20px; width: 50%; text-align: center; } html, body { height: 100%; margin: 0; } .full-height { height: 100%; } </style> <script> var Socket; function init() { Socket = new WebSocket("ws://" + window.location.hostname + ":81/"); Socket.onmessage = function (event) { console.log(event.data); }; } function rainbow_effect() { Socket.send("rainbow"); } function getColor(eid) { t = document.getElementById(eid); Socket.send(t.style.backgroundColor); } </script> </head> <body onload="javascript:init()"> <div class="full-height"> <table class="full-height center" style="width: 100%"> <tr> <td id="Tomato" onclick="getColor('Tomato');" style="background-color: Tomato"></td> <td id="Orange" onclick="getColor('Orange');" style="background-color: Orange"></td> </tr> <tr> <td id="DodgerBlue" onclick="getColor('DodgerBlue');" style="background-color: DodgerBlue"></td> <td id="MediumSeaGreen" onclick="getColor('MediumSeaGreen');" style="background-color: MediumSeaGreen"></td> </tr> <tr> <td id="Violet" onclick="getColor('Violet');" style="background-color: Violet"></td> <td id="SlateBlue" onclick="getColor('SlateBlue');" style="background-color: SlateBlue"></td> </tr> <tr> <td id="White" onclick="getColor('White');" style="background-color: White; border: 2px solid;"></td> <td id="Black" onclick="getColor('Black');" style="background-color: Black"></td> </tr> <tr> <td class="rainbow" id="rainbow" onclick="rainbow_effect();"></td> <td onclick="function func(){window.location.assign('/slider')};func();" style="border: 2px solid;"> <p class="unselectable"> <span style="color:tomato">R</span><span style="color:mediumseagreen">G</span><span style="color:dodgerblue">B</span> Sliders</p> </td> </tr> </table> </div> </body></html>)=====";
char slider[] PROGMEM = R"=====(<!DOCTYPE html>
<html>

<head>
    <style>
        .unselectable {
            font-size: 3vw;
            -webkit-touch-callout: none;
            -webkit-user-select: none;
            -khtml-user-select: none;
            -moz-user-select: none;
            -ms-user-select: none;
            user-select: none;
        }
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

        td {
            border-radius: 50px;
            text-align: center;
        }
        
        input {
            background: linear-gradient(to right, #696e6e 0%, #696e6e 50%, #fff 50%, #fff 100%);
            border: solid 1px #696e6e;
            border-radius: 50px;
            height: 100%;
            width: 100%;
            outline: none;
            /* transition: background 450ms ease-in; */
            -webkit-appearance: none;
        }
    </style>
</head>

<body onload="javascript:init()">
    <div class="full-height">
        <table class="full-height center" width="80%">
            <tr>
                <td colspan="2"> <input id="red" type="range" value="127" min=0 max=255
                        style="background: linear-gradient(to right, #ff6347 0%, #ff6347 50%, #fff 50%, #fff 100%);" />
                </td>
            </tr>
            <tr>
                <td colspan="2"> <input id="green" type="range" value="127" min=0 max=255
                        style="background: linear-gradient(to right, #3cb371 0%, #3cb371 50%, #fff 50%, #fff 100%);" />
                </td>
            </tr>
            <tr>
                <td colspan="2"> <input id="blue" type="range" value="127" min=0 max=255
                        style="background: linear-gradient(to right, #1e90ff 0%, #1e90ff 50%, #fff 50%, #fff 100%);" />
                </td>
            </tr>
            <tr>
                <td rowspan="2" style="width: 70%;">
                    <div id="pellete"
                        style="background-color:rgb(127,127,127); height: 100%; width: 100%; margin-left: auto; margin-right: auto; border-radius: 50px;">
                    </div>
                </td>
                <td style="border: 1px solid;" rowspan="2" onclick="function func(){window.location.assign('/')};func();">
                    <p class="unselectable">Home</p>
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
        Socket.send("(" + parseInt((red.value)).pad(3) + "," + (parseInt(blue.value)).pad(3) + "," + (parseInt(green.value)).pad(3) + ")");
        console.log("(" + parseInt((red.value)).pad(3) + "," + (parseInt(blue.value)).pad(3) + "," + (parseInt(green.value)).pad(3) + ")");
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
            LedStrip.fill(LedStrip.Color(255,0,0),0,15);
            LedStrip.show();
            Serial.print("Color: ");
            Serial.println(command);
            rainbow_flag = false;
        }
        else if (command == "orange")
        {
            LedStrip.fill(LedStrip.Color(255,165,0),0,15);
            LedStrip.show();
            Serial.print("Color: ");
            Serial.println(command);
            rainbow_flag = false;
        }
        else if (command == "dodgerblue")
        {
            LedStrip.fill(LedStrip.Color(0,0,255),0,15);
            LedStrip.show();
            Serial.print("Color: ");
            Serial.println(command);
            rainbow_flag = false;
        }
        else if (command == "mediumseagreen")
        {
            LedStrip.fill(LedStrip.Color(0,255,0),0,15);
            LedStrip.show();
            Serial.print("Color: ");
            Serial.println(command);
            rainbow_flag = false;
        }
        else if (command == "violet")
        {
            LedStrip.fill(LedStrip.Color(255, 0, 51),0,15);
            LedStrip.show();
            Serial.print("Color: ");
            Serial.println(command);
            rainbow_flag = false;
        }
        else if (command == "slateblue")
        {
            LedStrip.fill(LedStrip.Color(153, 0, 255),0,15);
            LedStrip.show();
            Serial.print("Color: ");
            Serial.println(command);
            rainbow_flag = false;
        }
        else if (command == "white")
        {
            LedStrip.fill(LedStrip.Color(255,255,255),0,15);
            LedStrip.show();
            Serial.print("Color: ");
            Serial.println(command);
            rainbow_flag = false;
        }
        else if (command == "black")
        {
            LedStrip.fill(LedStrip.Color(0,0,0),0,15);
            LedStrip.show();
            Serial.print("Color: ");
            Serial.println(command);
            rainbow_flag = false;
        }
        else if(command == "rainbow"){
            rainbow_flag = !rainbow_flag;
            Serial.println("RGB effect");
        }
        else if(command.startsWith("("))
        {
            rainbow_flag = false;
            int firstcomma = command.indexOf(",");
            int secondcomma = command.indexOf(",",firstcomma+1);

            LedStrip.fill(LedStrip.Color(command.substring(1,firstcomma).toInt(), \
                                         command.substring(firstcomma+1,secondcomma).toInt(), \
                                        command.substring(secondcomma+1,secondcomma+4).toInt()),0,15);
            LedStrip.show();
        }
        else{
            Serial.print("Uknown command: ");
            Serial.println(command);
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

    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
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
    dnsServer.start(DNS_PORT, "*", apIP);
    if (mdns.begin("home",WiFi.localIP())){
        Serial.println("MDNS Started!");
    }
    server.onNotFound([]() {
    server.send(200, "text/html", redirect);
    });

    server.on("/", []() {
        server.send_P(200, "text/html", main);
    });

    server.on("/slider", []() {
        server.send_P(200, "text/html", slider);
    });

    server.begin();
    webSocket.begin();
    webSocket.onEvent(webSocketEvent);

    MDNS.addService("http","tcp",80);
    LedStrip.begin();
    LedStrip.RainbowCycle(7);
    LedStrip.fill(LedStrip.Color(0,0,0),0,15);
    LedStrip.show();
}

void loop()
{
    dnsServer.processNextRequest();
    webSocket.loop();
    server.handleClient();
    if(rainbow_flag){
        Serial.println("rgb updating");
        LedStrip.Update();
    }
}