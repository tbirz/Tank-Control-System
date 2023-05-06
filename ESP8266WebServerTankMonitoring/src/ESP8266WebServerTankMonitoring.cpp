
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
ESP8266WebServer server(80);
#include <WifiManager.h>  // https://github.com/tzapu/WiFiManager
#include <DNSServer.h>
#include <WebSocketsServer.h>
WebSocketsServer webSocket = WebSocketsServer(81); //webSocket is used to manage the websocket connection


int connCount = 0; //connCount is used to count the number of connections to the web server
int serialSwapped = 0; //serialSwapped is used to indicate that the serial port has been swapped
int wsConnected = 0; //wsConnected is used to indicate that the websocket has been connected
int wsDisconnected = 0; //wsDisconnected is used to indicate that the websocket has been disconnected
String wsPayload = ""; //wsPayload is used to capture the websocket payload from client (webpage)
String rxTempString = "";  //rxTempString used to capture incoming serial data
String rxSystemAlarm = ""; //rxSystemAlarm is used to capture the system alarm status
String rxOperatingMode = "";  //rxOperatingMode is used to capture the operating mode
String rxAvailableCapacity = ""; //rxAvailableCapacity is used to capture the available capacity
String rxPercentageLevel = "";  //rxPercentageLevel is used to capture the percentage level
String rxActualLevel = ""; //rxActualLevel is used to capture the actual level
String rxTemperatureLevel = "";  //rxTemperatureLevel is used to capture the temperature level
String rxTankHeight = "";  //rxTankHeight is used to capture the tank height
String rxRefillSetPoint = "";  //rxRefillSetPoint is used to capture the refill set point
String rxFullSetPoint = "";  //rxFullSetPoint is used to capture the Tank full set point
String rxTotalCapacity = "";  //rxTotalCapacity is used to capture the total Tank capacity
String rxController12v = "";  //rxController12v is used to capture the 12v controller status
String rxController7v = "";  //rxController7v is used to capture the 7v controller status
String rxController5v = "";  //rxController5v is used to capture the 5v controller status
String rxController3v3 = "";  //rxController3v3 is used to capture the 3.3v controller status
String rxControllerVacConst = ""; //rxControllerVacConst is used to capture the Vac controller status
String rxControllerVacSw = "";  //rxControllerVacSw is used to capture the Vac controller switch status
String rxBatteryVoltageLevel = "";  //rxBatteryVoltageLevel is used to capture the battery voltage level
String rxSensorControl5v = "";  //rxSensorControl5v is used to capture the 5v sensor control status
String rxServo1Position = "";  //rxServo1Position is used to capture the servo 1 position
String rxServo2Position = "";  //rxServo2Position is used to capture the servo 2 position
String rxRelay1Position = "";  //rxRelay1Position is used to capture the relay 1 position
String rxRelay2Position = "";  //rxRelay2Position is used to capture the relay 2 position
String rxTankEmpty = "";  //rxTankEmpty is used to capture the tank empty status
String rxTankFull = "";  //rxTankFull is used to capture the tank full status
String rxTankLevelOk = "";  //rxTankLevelOk is used to capture the tank level ok status
String rxPumpRunning = "";   //rxPumpRunning is used to capture the pump running status
String rxRadioLinkRunning = "";  //rxRadioLinkRunning is used to capture the radio link running status
String rxServosRunning = "";  //rxServosRunning is used to capture the servos running status
String rxVacActive = "";  //rxVacActive is used to capture the Vac active status
String rxTankFilling = "";  //rxTankFilling is used to capture the tank filling status
String rxBuzzerActive = "";  //rxBuzzerActive is used to capture the buzzer active status

//baud rate for the serial port
long baud = 9600;
//tmpString is used to capture incoming serial data
String tmpString = "";
//count the number of characters received
unsigned int count = 0;
//maximum number of characters in the buffer
const byte maxBuffer = 64;
//use the buffer to capture incoming serial data
//define buffer as a static array to avoid stack overflow errors when using dynamic memory allocation (new) 
char  buffer[maxBuffer-1];
//webData is the data that is sent to the web page
char webData[maxBuffer];
//idxData is the index of the data in the buffer
byte idxData = 0;





// This function returns an HTML formated page in the correct type for display
// It uses the Raw string macro 'R' to place commands in PROGMEM
const char Web_page[] PROGMEM = R"=====(  
<!DOCTYPE html>
<html>
<html lang="en">

<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="stylesheet" href="https://maxcdn.bootstrapcdn.com/bootstrap/4.4.1/css/bootstrap.min.css">
  <script src="https://ajax.googleapis.com/ajax/libs/jquery/3.4.1/jquery.min.js"></script>
  <script src="https://cdnjs.cloudflare.com/ajax/libs/popper.js/1.16.0/umd/popper.min.js"></script>
  <script src="https://maxcdn.bootstrapcdn.com/bootstrap/4.4.1/js/bootstrap.min.js"></script>
<script>
$(document).ready(function(){
  
     $("#popoverOpMode").popover({
    html:true,
    placement: 'right',
    trigger: 'hover',
    content:popoverOpModes(),
    });

     $("#popoverTankFull").popover({
    html:true,
    placement: 'right',
    trigger: 'hover',
    content:popoverONOFF(),
    });

    $("#popoverTankLevelOk").popover({
    html:true,
    placement: 'right',
    trigger: 'hover',
    content:popoverONOFF(),
    });

    $("#popoverTankEmpty").popover({
    html:true,
    placement: 'right',
    trigger: 'hover',
    content:popoverONOFF(),
    });
    
    $("#popoverTankFilling").popover({
    html:true,
    placement: 'right',
    trigger: 'hover',
    content:popoverONOFF(),
    });
    
    $("#popoverPumpRunning").popover({
    html:true,
    placement: 'right',
    trigger: 'hover',
    content:popoverONOFF(),
    });

    $("#popoverServosRunning").popover({
    html:true,
    placement: 'right',
    trigger: 'hover',
    content:popoverONOFF(),
    });

     $("#popoverServo1Position").popover({
    html:true,
    placement: 'right',
    trigger: 'hover',
    content:popoverSwitching(),
    });

    $("#popoverServo2Position").popover({
    html:true,
    placement: 'right',
    trigger: 'hover',
    content:popoverSwitching(),
    });

    $("#popoverRadioLinkActive").popover({
    html:true,
    placement: 'right',
    trigger: 'hover',
    content:popoverONOFF(),
    });

    $("#popoverVacConstActive").popover({
    html:true,
    placement: 'right',
    trigger: 'hover',
    content:popoverONOFF(),
    });

    $("#popoverRelay1Position").popover({
    html:true,
    placement: 'right',
    trigger: 'hover',
    content:popoverSwitching(),
    });

    $("#popoverRelay2Position").popover({
    html:true,
    placement: 'right',
    trigger: 'hover',
    content:popoverSwitching(),
    });
    
    $("#popoverBuzzerActive").popover({
    html:true,
    placement: 'right',
    trigger: 'hover',
    content:popoverONOFF(),
    }); 

    $("#popoverSystemAlarm").popover({
    html:true,
    placement: 'right',
    trigger: 'hover',
    content:popoverAlarm(),
    });
});

</script>
<style>

    #heading {
          color: lime;
          font-size: 75%;
          font-weight: bold;
          margin-bottom: 0px;
        }    
    .table th {
          text-align: center;
          font-size: 85%;
        }    
    .table td {
          text-align: center;
          font-size: 70%;
        }
    #popoverRef td {
      display:hidden;
     align-items:left;
     text-align: left;
     font-size: 70%;     
    }
    #circleOpMode {
      background: transparent;
      border: 1px solid black;
      border-radius: 50%;
      height: 15px;
      width: 15px;
      display: inline-block;
      align-self: center;
      justify-self: center;
      -webkit-border-radius: 25px;
      -moz-border-radius: 25px;
    }
     #circleOpMode0 {
      background: lime;
      border: 1px solid black;
      border-radius: 50%;
      height: 10px;
      width: 10px;
      display: inline-block;
      align-self: center;
      justify-self: center;
      -webkit-border-radius: 25px;
      -moz-border-radius: 25px;
    }
     #circleOpMode1 {
      background: green;
      border: 1px solid black;
      border-radius: 50%;
      height: 10px;
      width: 10px;
      display: inline-block;
      align-self: center;
      justify-self: center;
      -webkit-border-radius: 25px;
      -moz-border-radius: 25px;
    }
     #circleOpMode2 {
      background: yellow;
      border: 1px solid black;
      border-radius: 50%;
      height: 10px;
      width: 10px;
      display: inline-block;
      align-self: center;
      justify-self: center;
      -webkit-border-radius: 25px;
      -moz-border-radius: 25px;
     }
       #circleOpMode3 {
      background: orange;
      border: 1px solid black;
      border-radius: 50%;
      height: 10px;
      width: 10px;
      display: inline-block;
      align-self: center;
      justify-self: center;
      -webkit-border-radius: 25px;
      -moz-border-radius: 25px;
    }
     #circleOpMode4 {
      background: grey;
      border: 1px solid black;
      border-radius: 50%;
      height: 10px;
      width: 10px;
      display: inline-block;
      align-self: center;
      justify-self: center;
      -webkit-border-radius: 25px;
      -moz-border-radius: 25px;
    }
      #circleOFF {
      background: transparent;
      border: 1px solid black;
      border-radius: 50%;
      height: 10px;
      width: 10px;
      display: inline-block;
      align-self: center;
      justify-self: center;
      -webkit-border-radius: 25px;
      -moz-border-radius: 25px;
    }
     #circleON {
      background: lime;
      border: 1px solid black;
      border-radius: 50%;
      height: 10px;
      width: 10px;
      display: inline-block;
      align-self: center;
      justify-self: center;
      -webkit-border-radius: 25px;
      -moz-border-radius: 25px;
    }
      #circleUNSW {
      background: lime;
      border: 1px solid black;
      border-radius: 50%;
      height: 10px;
      width: 10px;
      display: inline-block;
      align-self: center;
      justify-self: center;
      -webkit-border-radius: 25px;
      -moz-border-radius: 25px;
    }
     #circleSW {
      background: blue;
      border: 1px solid black;
      border-radius: 50%;
      height: 10px;
      width: 10px;
      display: inline-block;
      align-self: center;
      justify-self: center;
      -webkit-border-radius: 25px;
      -moz-border-radius: 25px;
    }
     #circleALARM {
      background: orange;
      border: 1px solid black;
      border-radius: 50%;
      height: 10px;
      width: 10px;
      display: inline-block;
      align-self: center;
      justify-self: center;
      -webkit-border-radius: 25px;
      -moz-border-radius: 25px;
    } 
    #circleERROR {
      background: red;
      border: 1px solid black;
      border-radius: 50%;
      height: 10px;
      width: 10px;
      display: inline-block;
      align-self: center;
      justify-self: center;
      -webkit-border-radius: 25px;
      -moz-border-radius: 25px;
    }
     #circleOTHER {
      background: black;
      border: 1px solid black;
      border-radius: 50%;
      height: 10px;
      width: 10px;
      display: inline-block;
      align-self: center;
      justify-self: center;
      -webkit-border-radius: 25px;
      -moz-border-radius: 25px;
    }
    #circleTankFull {
      background: transparent;
      border: 1px solid black;
      border-radius: 50%;
      height: 15px;
      width: 15px;
      display: inline-block;
      align-self: center;
      justify-self: center;
      -webkit-border-radius: 25px;
      -moz-border-radius: 25px;
    }
    #circleTankLevelOk {
      background: transparent;
      border: 1px solid black;
      border-radius: 50%;
      height: 15px;
      width: 15px;
      display: inline-block;
      align-self: center;
      justify-self: center;
      -webkit-border-radius: 25px;
      -moz-border-radius: 25px;
    }
    #circleTankEmpty {
      background: transparent;
      border: 1px solid black;
      border-radius: 50%;
      height: 15px;
      width: 15px;
      display: inline-block;
      align-self: center;
      justify-self: center;
      -webkit-border-radius: 25px;
      -moz-border-radius: 25px;
    }
    #circleTankFilling {
      background: transparent;
      border: 1px solid black;
      border-radius: 50%;
      height: 15px;
      width: 15px;
      display: inline-block;
      align-self: center;
      justify-self: center;
      -webkit-border-radius: 25px;
      -moz-border-radius: 25px;
    }
    #circlePumpRunning {
      background: transparent;
      border: 1px solid black;
      border-radius: 50%;
      height: 15px;
      width: 15px;
      display: inline-block;
      align-self: center;
      justify-self: center;
      -webkit-border-radius: 25px;
      -moz-border-radius: 25px;
    }
    #circleServosRunning {
      background: transparent;
      border: 1px solid black;
      border-radius: 50%;
      height: 15px;
      width: 15px;
      display: inline-block;
      align-self: center;
      justify-self: center;
      -webkit-border-radius: 25px;
      -moz-border-radius: 25px;
    }
    #circleServo1Position {
      background: transparent;
      border: 1px solid black;
      border-radius: 50%;
      height: 15px;
      width: 15px;
      display: inline-block;
      align-self: center;
      justify-self: center;
      -webkit-border-radius: 25px;
      -moz-border-radius: 25px;
    }
    #circleServo2Position {
      background: transparent;
      border: 1px solid black;
      border-radius: 50%;
      height: 15px;
      width: 15px;
      display: inline-block;
      align-self: center;
      justify-self: center;
      -webkit-border-radius: 25px;
      -moz-border-radius: 25px;
    }
    #circleRadioLinkActive {
      background: transparent;
      border: 1px solid black;
      border-radius: 50%;
      height: 15px;
      width: 15px;
      display: inline-block;
      align-self: center;
      justify-self: center;
      -webkit-border-radius: 25px;
      -moz-border-radius: 25px;
    }
    #circleVacActive {
      background: transparent;
      border: 1px solid black;
      border-radius: 50%;
      height: 15px;
      width: 15px;
      display: inline-block;
      align-self: center;
      justify-self: center;
      -webkit-border-radius: 25px;
      -moz-border-radius: 25px;
    }
    #circleRelay1Position {
      background: transparent;
      border: 1px solid black;
      border-radius: 50%;
      height: 15px;
      width: 15px;
      display: inline-block;
      align-self: center;
      justify-self: center;
      -webkit-border-radius: 25px;
      -moz-border-radius: 25px;
    }
    #circleRelay2Position {
      background: transparent;
      border: 1px solid black;
      border-radius: 50%;
      height: 15px;
      width: 15px;
      display: inline-block;
      align-self: center;
      justify-self: center;
      -webkit-border-radius: 25px;
      -moz-border-radius: 25px;
    }
    #circleBuzzerActive {
      background: transparent;
      border: 1px solid black;
      border-radius: 50%;
      height: 15px;
      width: 15px;
      display: inline-block;
      align-self: center;
      justify-self: center;
      -webkit-border-radius: 25px;
      -moz-border-radius: 25px;
    }
     #circleSystemAlarm {
      background: transparent;
      border: 1px solid black;
      border-radius: 50%;
      height: 15px;
      width: 15px;
      display: inline-block;
      align-self: center;
      justify-self: center;
      -webkit-border-radius: 25px;
      -moz-border-radius: 25px;
    }
  
.navbar {
 height:40px;
}

 #rxConsole {
  width: 755px!important; 
  height:30px;
  } 

  #txBuff {
    height:30px;
    width:250px!important;
  }
  #txButt {
    height:30px;
  }
  
  </style>

<body>
  <div id="heading" class="bg-primary font-weight-bold">
    <h1 style="text-align:center;">Water Tank Controller Status Web Page</h1>
    <p style="text-align:center; color:yellow; font-size:70%;">(Best viewed with the 'Chrome' browser)</p>
    <h6><p style="text-align:center; color: navy; font-size:85%;">Current DTG: <span style="color: white; font-size:85%;" id='DTG'>Current DTG</span></p>   
    <p style="text-align:center; color: navy; font-size:85%;">WS-Status: <span style="color: white; font-size:85%;" id='ws-Status'>Status</span></p>
  </div

<nav class="navbar navbar-expand-sm bg-dark navbar-dark" style="display:none;">
  <form class="d-none"> 
   <input class="form-control ml-sm-5" id="rxConsole"  type="text" readonly placeholder="Rx Data (read only)">   
     <input class="form-control ml-sm-5" "type="text" id="txBuff" placeholder="Enter Control Code" onkeydown="if(event.keyCode == 13) enterPressed();">
    <input class="btn btn-success ml-sm-2" id="txButt" type="submit" onclick="enterPressed();" value="Submit">
  </form> 
</nav>
<br>
  <div class="container">
    <div class="row">
      <div class="col-sm-4">
        <table id="actuals" class="table table-bordered table-sm border border-success">
          <h6><span class="badge badge-success">Actuals</span></h6>
            <thead class=" bg-success border border-success">
              <tr class="d-flex">
                <th class="col-6 border border-success">Reading</th>
                <th class="col-4 border border-success">Value</th>
                <th class="col-2 border border-success">Unit</th>
              </tr>
            </thead>
            <tbody>
              <tr id="actuals1" class="d-flex">
                <td class="col-6 font-weight-bold border border-success">Percentage Full</td>
                <td class="col-4 font-weight-bold border border-success"><span id="PercentageLevel">---.--</span></td>
                <td class="col-2 font-weight-bold border border-success">%</td>
              </tr>
              <tr id="actuals2" class="d-flex">
                <td class="col-6 font-weight-bold border border-success">Available Capacity</td>
                <td class="col-4 font-weight-bold border border-success"><span id="AvailableCapacity">-----.--</span></td>
                <td class="col-2 font-weight-bold border border-success">Ltrs</td>
              </tr>
              <tr id="actuals3" class="d-flex">
                <td class="col-6 font-weight-bold border border-success">Actual Level (from Top)</td>
                <td class="col-4 font-weight-bold border border-success"><span id="ActualLevel">---.--</span></td>
                <td class="col-2 font-weight-bold border border-success">cm</td>
              </tr>
              <tr id="actuals4" class="d-flex">
                <td class="col-6 font-weight-bold border border-success">Water Temperature</td>
                <td class="col-4 font-weight-bold border border-success"><span id="TemperatureLevel">----.--</span></td>
                <td class="col-2 font-weight-bold border border-success">&degC</td>
              </tr>
            </tbody>
        </table>

        <table id="fixedInfo" class="table table-bordered table-sm border border-info">
          <h6><span class="badge badge-info">Water Tank Static Info</span></h6>
          <thead class="bg-info border border-info">
            <tr class="d-flex">
              <th class="col-6 border border-info">Description</th>
              <th class="col-4 border border-info">Value</th>
              <th class="col-2 border border-info">Unit</th>
            </tr>
          </thead>
          <tbody>
            <tr id="fixedInfo1" class="d-flex">
              <td class="col-6 font-weight-bold border border-info">Tank Height</td>
              <td class="col-4 font-weight-bold border border-info"><span id="TankHeight">---.--</span></td>
              <td class="col-2 font-weight-bold border border-info">cm</td>
            </tr>
            <tr id="fixedInfo2" class="d-flex">
              <td class="col-6 font-weight-bold border border-info">Refill Set Point</td>
              <td class="col-4 font-weight-bold border border-info"><span id="RefillSetPoint">---.--</span></td>
              <td class="col-2 font-weight-bold border border-info">cm</td>
            </tr>
            <tr id="fixedInfo3" class="d-flex">
              <td class="col-6 font-weight-bold border border-info">Full Set Point</td>
              <td class="col-4 font-weight-bold border border-info"><span id="FullSetPoint">--.--</span></td>
              <td class="col-2 font-weight-bold border border-info">cm</td>
            </tr>
            <tr id="fixedInfo4" class="d-flex">
              <td class="col-6 font-weight-bold border border-info">Total Capacity (Volume)</td>
              <td class="col-4 font-weight-bold border border-info"><span id="TotalCapacity">-----.--</span></td>
              <td class="col-2 font-weight-bold border border-info">Ltrs</td>
            </tr>
          </tbody>
        </table>
        <table id="popoverRef" class="d-none table-bordered table-sm border border-success">
          <h6><span class="d-none badge badge-success">popoverRef</span></h6>
            <thead class=" bg-success border border-success">
              <tr class="d-flex border border-success">
                <th class="col-12 border border-success">Description</th>
              </tr>
            </thead>
            <tbody>
              <tr class="d-flex">
                <td class="col-sm-12 font-weight-bold border border-primary" id="popoverRef1"><span id="circleOpMode0"></span> TDPSA - Tank Disconnected, Pump Stopped in Auto Mode</td>
              </tr>
              <tr class="d-flex">
                <td class="col-sm-12 font-weight-bold border border-primary" id="popoverRef2"><span id="circleOpMode1"></span> TCPRA - Tank Connected, Pump Running in Auto Mode</td>
              </tr>
              <tr class="d-flex">
                <td class="col-sm-12 font-weight-bold border border-primary" id="popoverRef3"><span id="circleOpMode2"></span> TDPSM - Tank Disconnected, Pump Stopped in Manual Mode</td>
              </tr>
              <tr class="d-flex">
                <td class="col-sm-12 font-weight-bold border border-primary" id="popoverRef4"><span id="circleOpMode3"></span> TCPRM - Tank Cconnected, Pump Running in Manual Mode</td>
              </tr>
              <tr class="d-flex">
                <td class="col-sm-12 font-weight-bold border border-primary" id="popoverRef5"><span id="circleOpMode4"></span> TDPSB - Tank Disconnected, Pump Stopped in Bypass Mode</td>
              </tr>
              <tr class="d-flex">
                <td class="col-sm-12 font-weight-bold border border-primary" id="popoverRef6"><span id="circleOFF"></span> OFF</td>
              </tr>
              <tr class="d-flex">
                <td class="col-sm-12 font-weight-bold border border-primary" id="popoverRef7"><span id="circleON"></span> ON</td>
              </tr>
              <tr class="d-flex">
                <td class="col-sm-12 font-weight-bold border border-primary" id="popoverRef8"><span id="circleUNSW"></span> UNSWITCHED</td>
              </tr>
              <tr class="d-flex">
                <td class="col-sm-12 font-weight-bold border border-primary" id="popoverRef9"><span id="circleSW"></span> SWITCHED</td>
              </tr> 
              <tr class="d-flex">
                <td class="col-sm-12 font-weight-bold border border-primary" id="popoverError"><span id="circleError"></span> ERROR</td>
              </tr>
              <tr class="d-flex">
                <td class="col-sm-12 font-weight-bold border border-primary" id="popoverOther"><span id="circleError"></span> OTHER</td>
              </tr> 
              <tr class="d-flex">
                <td class="col-sm-12 font-weight-bold border border-primary" id="popoverAlarm"><span id="circleAlarm"></span> ALARM</td>
              </tr>          
            </tbody>
        </table>
      </div>
      <div class="col-sm-4">
        <table id="controllerIndicators" class="table table-bordered table-sm">
          <h6><span class="badge badge-primary">Controller Indicators</span></h6>
          <thead class="bg-primary border border-primary">
            <tr class="d-flex border border-primary">
              <th class="col-sm-8 border border-primary">Property</th>
              <th class="col-sm-4 border border-primary">Status</th>
              <th class="d-none border border-primary">Code</th>
            </tr>
          </thead>
          <tbody>
            <tr id="controllerIndicators1" class="d-flex">
              <td class="col-sm-8 font-weight-bold border border-primary">Operating Mode</td>
              <td class="d-none"><span id="OperatingMode">----</span></td>
              <td class="col-sm-4 font-weight-bold border border-primary" <a href="#" id="popoverOpMode" title="Indications:"></a><span id="circleOpMode"></span></td>
            </tr>
            <tr id="controllerIndicators2" class="d-flex">
              <td class="col-sm-8 font-weight-bold border border-primary">Tank Full</td>
              <td class="d-none font-weight-bold"><span id="TankFull">-</span></td>
              <td class="col-sm-4 font-weight-bold border border-primary" <a href="#" id="popoverTankFull" title="Indications:"></a><span id="circleTankFull"></span></td>
            </tr>
            <tr id="controllerIndicators3" class="d-flex">
              <td class="col-sm-8 font-weight-bold border border-primary">Tank Level Ok</td>
              <td class="d-none font-weight-bold"><span id="TankLevelOk">-</span></td>
              <td class="col-sm-4 font-weight-bold border border-primary" <a href="#" id="popoverTankLevelOk" title="Indications:"></a><span id="circleTankLevelOk"></span></td>
            </tr>
            <tr id="controllerIndicators4" class="d-flex">
              <td class="col-sm-8 font-weight-bold border border-primary">Tank Empty</td>
              <td class="d-none font-weight-bold"><span id="TankEmpty">-</span></td>
              <td class="col-sm-4 font-weight-bold border border-primary" <a href="#" id="popoverTankEmpty" title="Indications:"></a><span id="circleTankEmpty"></span></td>
            </tr>
            <tr id="controllerIndicators5" class="d-flex">
              <td class="col-sm-8 font-weight-bold border border-primary">Tank Filling</td>
              <td class="d-none font-weight-bold"><span id="TankFilling">-</span></td>
              <td class="col-sm-4 font-weight-bold border border-primary" <a href="#" id="popoverTankFilling" title="Indications:"></a><span id="circleTankFilling"></span></td>
            </tr>
            <tr id="controllerIndicators6" class="d-flex">
              <td class="col-sm-8 font-weight-bold border border-primary">Pump Running</td>
              <td class="d-none font-weight-bold"><span id="PumpRunning">-</span></td>
              <td class="col-sm-4 font-weight-bold border border-primary" <a href="#" id="popoverPumpRunning" title="Indications:"></a><span id="circlePumpRunning"></span></td>
            </tr>
            <tr id="controllerIndicators7" class="d-flex">
              <td class="col-sm-8 font-weight-bold border border-primary">Servos Running</td>
              <td class="d-none font-weight-bold"><span id="ServosRunning">-</span></td>
              <td class="col-sm-4 font-weight-bold border border-primary" <a href="#" id="popoverServosRunning" title="Indications:"></a><span id="circleServosRunning"></span></td>
            </tr>
            <tr id="controllerIndicators8" class="d-flex">
              <td class="col-sm-8 font-weight-bold border border-primary">Servo1 Position</td>
              <td class="d-none font-weight-bold"><span id="Servo1Position">-</span></td>
              <td class="col-sm-4 font-weight-bold border border-primary" <a href="#" id="popoverServo1Position" title="Indications:"></a><span id="circleServo1Position"></span></td>
            </tr>
            <tr id="controllerIndicators9" class="d-flex">
              <td class="col-sm-8 font-weight-bold border border-primary">Servo2 Position</td>
              <td class="d-none font-weight-bold"><span id="Servo2Position">-</span></td>
              <td class="col-sm-4 font-weight-bold border border-primary" <a href="#" id="popoverServo2Position" title="Indications:"></a><span id="circleServo2Position"></span></td>
            </tr>
            <tr id="controllerIndicators10" class="d-flex">
              <td class="col-sm-8 font-weight-bold border border-primary">Radio Link Active</td>
              <td class="d-none font-weight-bold"><span id="RadioLinkActive">-/span></td>
              <td class="col-sm-4 font-weight-bold border border-primary" <a href="#" id="popoverRadioLinkActive" title="Indications:"></a><span id="circleRadioLinkActive"></span></td>
            </tr>
            <tr id="controllerIndicators11" class="d-flex">
              <td class="col-sm-8 font-weight-bold border border-primary">VAC Const Active</td>
              <td class="d-none font-weight-bold"><span id="VacConstActive">-</span></td>
              <td class="col-sm-4 font-weight-bold border border-primary" <a href="#" id="popoverVacConstActive" title="Indications:"></a><span id="circleVacActive"></span></td>
            </tr>
            <tr id="controllerIndicators12" class="d-flex">
              <td class="col-sm-8 font-weight-bold border border-primary">Relay1 Position</td>
              <td class="d-none font-weight-bold"><span id="Relay1Position">--</span></td>
              <td class="col-sm-4 font-weight-bold border border-primary" <a href="#" id="popoverRelay1Position" title="Indications:"></a><span id="circleRelay1Position"></span></td>
            </tr>
            <tr id="controllerIndicators13" class="d-flex">
              <td class="col-sm-8 font-weight-bold border border-primary">Relay2 Position</td>
              <td class="d-none font-weight-bold"><span id="Relay2Position">--</span></td>
              <td class="col-sm-4 font-weight-bold border border-primary" <a href="#" id="popoverRelay2Position" title="Indications:"><span id="circleRelay2Position"></span></td>
            </tr>
            <tr id="controllerIndicators14" class="d-flex">
              <td class="col-sm-8 font-weight-bold border border-primary">Buzzer Active</td>
              <td class="d-none font-weight-bold"><span id="BuzzerActive">-</span></td>
              <td class="col-sm-4 font-weight-bold border border-primary" <a href="#" id="popoverBuzzerActive" title="Indications:"><span id="circleBuzzerActive"></span></td>
            </tr>
            <tr id="controllerIndicators15" class="d-flex">
              <td class="col-sm-8 font-weight-bold border border-primary">System Alarm</td>
              <td class="d-none font-weight-bold"><span id="SystemAlarm">-</span></td>
              <td class="col-sm-4 font-weight-bold border border-primary" <a href="#" id="popoverSystemAlarm" title="Indications:"><span id="circleSystemAlarm"></span></td>
            </tr>
          </tbody>
        </table>
      </div>
      <div class="col-sm-4">
        <table id="controllerVoltages" class="table table-bordered table-sm border border-dark">
          <h6><span class="badge badge-dark">Controller DC Voltages</span></h6>
          <thead class="thead-dark border border-dark">
            <tr class="d-flex border border-dark">
              <th class="col-5 border border-dark">Voltage</th>
              <th class="col-4 border border-dark">Value</th>
              <th class="col-3 border border-dark">Unit</th>
            </tr>
          </thead>
          <tbody>
            <tr id="controllerVoltages1" class="d-flex">
              <td class="col-5 font-weight-bold border border-dark">Control (12v)</td>
              <td class="col-4 font-weight-bold border border-dark"><span id="Controller12v">--.--</span></td>
              <td class="col-3 font-weight-bold border border-dark">Volts</td>
            </tr>
            <tr id="controllerVoltages2" class="d-flex">
              <td class="col-5 font-weight-bold border border-dark">Control (7v)</td>
              <td class="col-4 font-weight-bold border border-dark"><span id="Controller7v">--.--</span></td>
              <td class="col-3 font-weight-bold border border-dark">Volts</td>
            </tr>
            <tr id="controllerVoltages3" class="d-flex">
              <td class="col-5 font-weight-bold border border-dark">Control (5v)</td>
              <td class="col-4 font-weight-bold border border-dark"><span id="Controller5v">--.--</span></td>
              <td class="col-3 font-weight-bold border border-dark">Volts</td>
            </tr>
            <tr id="controllerVoltages4" class="d-flex">
              <td class="col-5 font-weight-bold border border-dark">Control (3v3)</td>
              <td class="col-4 font-weight-bold border border-dark"><span id="Controller3v3">--.--</span></td>
              <td class="col-3 font-weight-bold border border-dark">Volts</td>
            </tr>
          </tbody>
        </table>
         <table id="controllerVacVoltages" class="table table-bordered table-sm border border-warning">
          <h6><span class="badge badge-warning">Controller AC Voltages</span></h6>
          <thead class="bg-warning border border-warning">
            <tr class="d-flex border border-warning">
              <th class="col-5 border border-warning">Voltage</th>
              <th class="col-4 border border-warning">Value</th>
              <th class="col-3 border border-warning">Unit</th>
            </tr>
          </thead>
          <tbody>
            <tr id="controllerVacVoltages1" class="d-flex">
              <td class="col-5 font-weight-bold border border-warning">Constant (28v)</td>
              <td class="col-4 font-weight-bold border border-warning"><span id="controllerVacConst">--.--</span></td>
              <td class="col-3 font-weight-bold border border-warning">VAC</td>
            </tr>
            <tr id="controllerVacVoltages2" class="d-flex">
              <td class="col-5 font-weight-bold border border-warning">Switched (28v)</td>
              <td class="col-4 font-weight-bold border border-warning"><span id="controllerVacSw">--.--</span></td>
              <td class="col-3 font-weight-bold border border-warning">VAC</td>
            </tr>
          </tbody>
        </table>
        <table id="remoteSensorVoltages" class="table table-bordered table-sm border border-secondary">
          <h6><span class="badge badge-secondary">Remote Sensor Voltages</span></h6>
          <thead class="bg-secondary border border-secondary">
            <tr class="d-flex border border-secondary">
              <th class="col-5 border border-secondary">Voltage</th>
              <th class="col-4 border border-secondary">Value</th>
              <th class="col-3 border border-secondary">Unit</th>
            </tr>
          </thead>
          <tbody>
            <tr id="remoteSensorVoltages1" class="d-flex">
              <td class="col-5 font-weight-bold border border-secondary">Battery Input (12v)</td>
              <td class="col-4 font-weight-bold border border-secondary"><span id="BatteryVoltageLevel">--.--</span></td>
              <td class="col-3 font-weight-bold border border-secondary">Volts</td>
            </tr>
            <tr id="remoteSensorVoltages2" class="d-flex">
              <td class="col-5 font-weight-bold border border-secondary">Control (5v)</td>
              <td class="col-4 font-weight-bold border border-secondary"><span id="SensorControl5v">--.--</span></td>
              <td class="col-3 font-weight-bold border border-secondary">Volts</td>
            </tr>
          </tbody>
        </table>
      </div>
    </div>
  </div>

  <script>

    var Socket;
    var maxBuffer=64;

function popoverOpModes(){ 
  return ('<p><span id="circleOpMode0"></span> TDPSM - Tank Disconnected, Pump Stopped in Auto Mode (Default)<br>' + 
   '<span id="circleOpMode1"></span> TCPRA - Tank Connected, Pump Running in Auto Mode<br>' +
   '<span id="circleOpMode2"></span> TDPSM - Tank Disconnected, Pump Stopped in Manual Mode<br>' +
   '<span id="circleOpMode3"></span> TCPRM - Tank Connected, Pump Running in Manual Mode<br>' +
   '<span id="circleOpMode4"></span> TDPSB - Tank Disconnected, Pump Stopped in Bypass Mode<br>' +
   '<span id="circleERROR"></span> ERROR<br>' + 
   '<span id="circleOTHER"></span> OTHER</p>');
 }

 function popoverONOFF() {
  return ('<p><span id="circleOFF"></span> OFF<br>' + 
   '<span id="circleON"></span> ON<br>' +
   '<span id="circleERROR"></span> ERROR<br>' + 
   '<span id="circleOTHER"></span> OTHER</p>'); 
 }

  function popoverSwitching() {
  return ('<p><span id="circleUNSW"></span> Default Position<br>' + 
   '<span id="circleSW"></span> Switched Position<br>' +
   '<span id="circleERROR"></span> ERROR<br>' + 
   '<span id="circleOTHER"></span> OTHER</p>');  
 }
function popoverAlarm() {
  return ('<p><span id="circleOFF"></span> OFF<br>' + 
   '<span id="circleALARM"></span> ALARM<br>' +
   '<span id="circleERROR"></span> ERROR<br>' + 
   '<span id="circleOTHER"></span> OTHER</p>');  
 }

  function wsReadyState(){
   if (Socket.readyState == 0) {
    document.getElementById("ws-Status").innerHTML="WS CONNECTING....";
   }
   else if(Socket.readyState == 1) {
    document.getElementById("ws-Status").innerHTML="WS OPEN";
   }
   else if(Socket.readyState == 2) {
    document.getElementById("ws-Status").innerHTML="WS CLOSING....";
   }
    else if(Socket.readyState == 3) {
    document.getElementById("ws-Status").innerHTML="WS CLOSED";
   }
   else {
    document.getElementById("ws-Status").innerHTML="NOT RUNNING";
   }
   setTxItemStatus();
}


function setTxItemStatus() {
  if (Socket.readyState !=1) {
 document.getElementById("txBuff").disabled=true; 
 document.getElementById("txButt").disabled=true; 
  }
  else {
 document.getElementById("txBuff").disabled=false; 
 document.getElementById("txButt").disabled=false;     
  }
}
   
 function initWebSockets() {
    Socket = new WebSocket('ws://' + window.location.hostname + ':81/'); 
    Socket.onopen = function(e) {
    Socket.send('Socket Connected @: ' + new Date());  
      wsReadyState();    
    }
}

function socketClose() {
    Socket.onclose = function(e) {
     Socket.send('Socket Closed @: ' + new Date());   
     wsReadyState(); 
    console.log('Socket Closed: ' + e.code + e.reason)
    }
}

function socketReceive() { 
    var rxBuffer;
    Socket.bufferType = "string"; 
    Socket.onmessage = function(e) { // Log messages from the server     
        document.getElementById("rxConsole").value+=e.data;
        rxBuffer=document.getElementById("rxConsole").value;
        rxBuffer=""; 
        for (var rxIdx=0; rxIdx < maxBuffer; rxIdx++) {
          rxBuffer += e.data;                                 
        }
        console.log('Data: ' + rxBuffer)
        processData(rxBuffer, maxBuffer);
        statusCondition();
        document.getElementById("rxConsole").value=""; 
  } 
}

function processData(rxData,maxBuffer) {
  var filterDiscard = /^[A-Za-z,\s]+$/;  //\s=whitespace
  var rxStringValue="";
  var valueLengthMax=10;
  for (var idx = 0; idx < maxBuffer; idx++) {
    //-----------------------------------------------------
    //Reported data levels
    //-----------------------------------------------------
    if (rxData[idx] == 'A') { //Available Capacity
      for (var idxSub = 1; idxSub < valueLengthMax ; idxSub++) {
        if (rxData[idx + idxSub].match(filterDiscard)) {
          rxData[idx] = rxData[idx + idxSub];
          break;
        }
        else {
          rxStringValue += rxData[idx + idxSub];              
        }      
      }
      document.getElementById("AvailableCapacity").innerHTML = rxStringValue;
      rxStringValue="";
   } 
    //-----------------------------------------------------    
    if (rxData[idx] == 'B') { //BatteryVoltageLevel
      for (var idxSub = 1; idxSub < valueLengthMax ; idxSub++) {
        if (rxData[idx + idxSub].match(filterDiscard)) {
          rxData[idx] = rxData[idx + idxSub];
          break;
        }
        else {
          rxStringValue += rxData[idx + idxSub];              
        }      
      }
      document.getElementById("BatteryVoltageLevel").innerHTML = rxStringValue;
      rxStringValue="";
    }
    //-----------------------------------------------------     
    if (rxData[idx] == 'C') { //TotalCapacity
      for (var idxSub = 1; idxSub < valueLengthMax ; idxSub++) {
        if (rxData[idx + idxSub].match(filterDiscard)) {
          rxData[idx] = rxData[idx + idxSub];
          break;
        }
        else {
          rxStringValue += rxData[idx + idxSub];              
        }      
      }
      document.getElementById("TotalCapacity").innerHTML = rxStringValue;
      rxStringValue="";
    } 
    //-----------------------------------------------------   
    if (rxData[idx] == 'D') { //ActualLevel
      for (var idxSub = 1; idxSub < valueLengthMax ; idxSub++) {
        if (rxData[idx + idxSub].match(filterDiscard)) {
          rxData[idx] = rxData[idx + idxSub];
          break;
        }
        else {
          rxStringValue += rxData[idx + idxSub];              
        }      
      }
      document.getElementById("ActualLevel").innerHTML = rxStringValue;
      rxStringValue="";
    } 
    //-----------------------------------------------------  
    if (rxData[idx] == 'F') { //FullSetPoint
      for (var idxSub = 1; idxSub < valueLengthMax ; idxSub++) {
        if (rxData[idx + idxSub].match(filterDiscard)) {
          rxData[idx] = rxData[idx + idxSub];
          break;
        }
        else {
          rxStringValue += rxData[idx + idxSub];              
        }      
      }
      document.getElementById("FullSetPoint").innerHTML = rxStringValue;
      rxStringValue="";
    } 
    //-----------------------------------------------------
   if (rxData[idx] == 'H') { //TankHeight
      for (var idxSub = 1; idxSub < valueLengthMax ; idxSub++) {
        if (rxData[idx + idxSub].match(filterDiscard)) {
          rxData[idx] = rxData[idx + idxSub];
          break;
        }
        else {
          rxStringValue += rxData[idx + idxSub];              
        }      
      }
      document.getElementById("TankHeight").innerHTML = rxStringValue;
      rxStringValue="";
    } 
    //-----------------------------------------------------
    if (rxData[idx] == 'I') { //Controller12v
      for (var idxSub = 1; idxSub < valueLengthMax ; idxSub++) {
        if (rxData[idx + idxSub].match(filterDiscard)) {
          rxData[idx] = rxData[idx + idxSub];
          break;
        }
        else {
          rxStringValue += rxData[idx + idxSub];              
        }      
      }
      document.getElementById("Controller12v").innerHTML = rxStringValue;
      rxStringValue="";
    } 
    //----------------------------------------------------
    if (rxData[idx] == 'J') { //Controller7v
      for (var idxSub = 1; idxSub < valueLengthMax ; idxSub++) {
        if (rxData[idx + idxSub].match(filterDiscard)) {
          rxData[idx] = rxData[idx + idxSub];
          break;
        }
        else {
          rxStringValue += rxData[idx + idxSub];              
        }      
      }
      document.getElementById("Controller7v").innerHTML = rxStringValue;
      rxStringValue="";
    } 
    //----------------------------------------------------- 
    if (rxData[idx] == 'K') { //Controller5v
      for (var idxSub = 1; idxSub < valueLengthMax ; idxSub++) {
        if (rxData[idx + idxSub].match(filterDiscard)) {
          rxData[idx] = rxData[idx + idxSub];
          break;
        }
        else {
          rxStringValue += rxData[idx + idxSub];              
        }      
      }
      document.getElementById("Controller5v").innerHTML = rxStringValue;
      rxStringValue="";
    } 
    //-----------------------------------------------------
    if (rxData[idx] == 'L') { //Controller3v3
      for (var idxSub = 1; idxSub < valueLengthMax ; idxSub++) {
        if (rxData[idx + idxSub].match(filterDiscard)) {
          rxData[idx] = rxData[idx + idxSub];
          break;
        }
        else {
          rxStringValue += rxData[idx + idxSub];              
        }      
      }
      document.getElementById("Controller3v3").innerHTML = rxStringValue;
      rxStringValue="";
    } 
    //----------------------------------------------------- 
    if (rxData[idx] == 'M') { //controllerVacConst
      for (var idxSub = 1; idxSub < valueLengthMax ; idxSub++) {
        if (rxData[idx + idxSub].match(filterDiscard)) {
          rxData[idx] = rxData[idx + idxSub];
          break;
        }
        else {
          rxStringValue += rxData[idx + idxSub];              
        }      
      }
      document.getElementById("controllerVacConst").innerHTML = rxStringValue;
      rxStringValue="";
    } 
    //----------------------------------------------------- 
    if (rxData[idx] == 'N') { //controllerVacSw
      for (var idxSub = 1; idxSub < valueLengthMax ; idxSub++) {
        if (rxData[idx + idxSub].match(filterDiscard)) {
          rxData[idx] = rxData[idx + idxSub];
          break;
        }
        else {
          rxStringValue += rxData[idx + idxSub];              
        }      
      }
      document.getElementById("controllerVacSw").innerHTML = rxStringValue;
      rxStringValue="";
    } 
    //-----------------------------------------------------   
    if (rxData[idx] == 'P') { //PercentageLevel
      for (var idxSub = 1; idxSub < valueLengthMax ; idxSub++) {
        if (rxData[idx + idxSub].match(filterDiscard)) {
          rxData[idx] = rxData[idx + idxSub];
          break;
        }
        else {
          rxStringValue += rxData[idx + idxSub];              
        }      
      }
      document.getElementById("PercentageLevel").innerHTML = rxStringValue;
      rxStringValue="";
    } 
    //-----------------------------------------------------
     if (rxData[idx] == 'R') { //RefillSetPoint
      for (var idxSub = 1; idxSub < valueLengthMax ; idxSub++) {
        if (rxData[idx + idxSub].match(filterDiscard)) {
          rxData[idx] = rxData[idx + idxSub];
          break;
        }
        else {
          rxStringValue += rxData[idx + idxSub];              
        }      
      }
      document.getElementById("RefillSetPoint").innerHTML = rxStringValue;
      rxStringValue="";
    } 
    //----------------------------------------------------- 
    if (rxData[idx] == 'T') { //TemperatureLevel
      for (var idxSub = 1; idxSub < valueLengthMax ; idxSub++) {
        if (rxData[idx + idxSub].match(filterDiscard)) {
          rxData[idx] = rxData[idx + idxSub];
          break;
        }
        else {
          rxStringValue += rxData[idx + idxSub];              
        }      
      }
      document.getElementById("TemperatureLevel").innerHTML = rxStringValue;
      rxStringValue="";
    } 
    //-----------------------------------------------------
     if (rxData[idx] == 'V') { //SensorControl5v
      for (var idxSub = 1; idxSub < valueLengthMax ; idxSub++) {
        if (rxData[idx + idxSub].match(filterDiscard)) {
          rxData[idx] = rxData[idx + idxSub];
          break;
        }
        else {
          rxStringValue += rxData[idx + idxSub];              
        }      
      }
      document.getElementById("SensorControl5v").innerHTML = rxStringValue;
      rxStringValue="";
    }     
    //-----------------------------------------------------
    //Controller indications
    //-----------------------------------------------------    
         if (rxData[idx] == 'a') { //AlarmStatus
      for (var idxSub = 1; idxSub < valueLengthMax ; idxSub++) {
        if (rxData[idx + idxSub].match(filterDiscard)) {
          rxData[idx] = rxData[idx + idxSub];
          break;
        }
        else {
          rxStringValue += rxData[idx + idxSub];              
        }      
      }
      document.getElementById("SystemAlarm").innerHTML = rxStringValue;
      rxStringValue="";
    }  
  //-----------------------------------------------------            
     if (rxData[idx] == 'b') { //BuzzerActive
      for (var idxSub = 1; idxSub < valueLengthMax ; idxSub++) {
        if (rxData[idx + idxSub].match(filterDiscard)) {
          rxData[idx] = rxData[idx + idxSub];
          break;
        }
        else {
          rxStringValue += rxData[idx + idxSub];              
        }      
      }
      document.getElementById("BuzzerActive").innerHTML = rxStringValue;
      rxStringValue="";
    }  
    //-----------------------------------------------------  
    if (rxData[idx] == 'e') { //TankEmpty
      for (var idxSub = 1; idxSub < valueLengthMax ; idxSub++) {
        if (rxData[idx + idxSub].match(filterDiscard)) {
          rxData[idx] = rxData[idx + idxSub];
          break;
        }
        else {
          rxStringValue += rxData[idx + idxSub];              
        }      
      }
      document.getElementById("TankEmpty").innerHTML = rxStringValue;
      rxStringValue="";
    } 
    //----------------------------------------------------- 
    if (rxData[idx] == 'f') { //TankFull
      for (var idxSub = 1; idxSub < valueLengthMax ; idxSub++) {
        if (rxData[idx + idxSub].match(filterDiscard)) {
          rxData[idx] = rxData[idx + idxSub];
          break;
        }
        else {
          rxStringValue += rxData[idx + idxSub];              
        }      
      }
      document.getElementById("TankFull").innerHTML = rxStringValue;
      rxStringValue="";
    } 
    //----------------------------------------------------- 
     if (rxData[idx] == 'l') { //TankLevelOk
      for (var idxSub = 1; idxSub < valueLengthMax ; idxSub++) {
        if (rxData[idx + idxSub].match(filterDiscard)) {
          rxData[idx] = rxData[idx + idxSub];
          break;
        }
        else {
          rxStringValue += rxData[idx + idxSub];              
        }      
      }
      document.getElementById("TankLevelOk").innerHTML = rxStringValue;
      rxStringValue="";
    } 
    //----------------------------------------------------- 
    if (rxData[idx] == 'o') { //OperatingMode
      for (var idxSub = 1; idxSub < valueLengthMax ; idxSub++) {
        if (rxData[idx + idxSub].match(filterDiscard)) {
          rxData[idx] = rxData[idx + idxSub];
          break;
        }
        else {
          rxStringValue += rxData[idx + idxSub];              
        }      
      }
      document.getElementById("OperatingMode").innerHTML = rxStringValue;
      rxStringValue="";
    } 
    //-----------------------------------------------------  
    if (rxData[idx] == 'p') { //PumpRunning
      for (var idxSub = 1; idxSub < valueLengthMax ; idxSub++) {
        if (rxData[idx + idxSub].match(filterDiscard)) {
          rxData[idx] = rxData[idx + idxSub];
          break;
        }
        else {
          rxStringValue += rxData[idx + idxSub];              
        }      
      }
      document.getElementById("PumpRunning").innerHTML = rxStringValue;
      rxStringValue="";
    } 
    //----------------------------------------------------- 
    if (rxData[idx] == 'r') { //RadioLinkActive
      for (var idxSub = 1; idxSub < valueLengthMax ; idxSub++) {
        if (rxData[idx + idxSub].match(filterDiscard)) {
          rxData[idx] = rxData[idx + idxSub];
          break;
        }
        else {
          rxStringValue += rxData[idx + idxSub];              
        }      
      }
      document.getElementById("RadioLinkActive").innerHTML = rxStringValue;
      rxStringValue="";
    } 
    //----------------------------------------------------- 
    if (rxData[idx] == 's') { //ServosRunning
      for (var idxSub = 1; idxSub < valueLengthMax ; idxSub++) {
        if (rxData[idx + idxSub].match(filterDiscard)) {
          rxData[idx] = rxData[idx + idxSub];
          break;
        }
        else {
          rxStringValue += rxData[idx + idxSub];              
        }      
      }
      document.getElementById("ServosRunning").innerHTML = rxStringValue;
      rxStringValue="";
    } 
    //-----------------------------------------------------
    if (rxData[idx] == 't') { //TankFilling
      for (var idxSub = 1; idxSub < valueLengthMax ; idxSub++) {
        if (rxData[idx + idxSub].match(filterDiscard)) {
          rxData[idx] = rxData[idx + idxSub];
          break;
        }
        else {
          rxStringValue += rxData[idx + idxSub];              
        }      
      }
      document.getElementById("TankFilling").innerHTML = rxStringValue;
      rxStringValue="";
    } 
    //----------------------------------------------------- 
    if (rxData[idx] == 'v') { //VacActive
      for (var idxSub = 1; idxSub < valueLengthMax ; idxSub++) {
        if (rxData[idx + idxSub].match(filterDiscard)) {
          rxData[idx] = rxData[idx + idxSub];
          break;
        }
        else {
          rxStringValue += rxData[idx + idxSub];              
        }      
      }
      document.getElementById("VacConstActive").innerHTML = rxStringValue;
      rxStringValue="";
    } 
    //----------------------------------------------------- 
    if (rxData[idx] == 'w') { //Relay1Position
      for (var idxSub = 1; idxSub < valueLengthMax ; idxSub++) {
        if (rxData[idx + idxSub].match(filterDiscard)) {
          rxData[idx] = rxData[idx + idxSub];
          break;
        }
        else {
          rxStringValue += rxData[idx + idxSub];              
        }      
      }
      document.getElementById("Relay1Position").innerHTML = rxStringValue;
      rxStringValue="";
    } 
    //-----------------------------------------------------
    if (rxData[idx] == 'x') { //Relay2Position
      for (var idxSub = 1; idxSub < valueLengthMax ; idxSub++) {
        if (rxData[idx + idxSub].match(filterDiscard)) {
          rxData[idx] = rxData[idx + idxSub];
          break;
        }
        else {
          rxStringValue += rxData[idx + idxSub];              
        }      
      }
      document.getElementById("Relay2Position").innerHTML = rxStringValue;
      rxStringValue="";
    } 
    //-----------------------------------------------------
    if (rxData[idx] == 'y') { //Servo1Position
      for (var idxSub = 1; idxSub < valueLengthMax ; idxSub++) {
        if (rxData[idx + idxSub].match(filterDiscard)) {
          rxData[idx] = rxData[idx + idxSub];
          break;
        }
        else {
          rxStringValue += rxData[idx + idxSub];              
        }      
      }
      document.getElementById("Servo1Position").innerHTML = rxStringValue;
      rxStringValue="";
    } 
    //-----------------------------------------------------
    if (rxData[idx] == 'z') { //Servo2Position
      for (var idxSub = 1; idxSub < valueLengthMax ; idxSub++) {
        if (rxData[idx + idxSub].match(filterDiscard)) {
          rxData[idx] = rxData[idx + idxSub];
          break;
        }
        else {
          rxStringValue += rxData[idx + idxSub];              
        }      
      }
      document.getElementById("Servo2Position").innerHTML = rxStringValue;
      rxStringValue="";
    }             
  }
}

function socketErrors() {// Log errors
  Socket.onerror = function (e) {
    console.log('WebSocket Error: ' + e.message);
  } 
}
    
function enterPressed() {//function for when 'send' button pressed
   if(Socket.readyState==1) {
      document.getElementById("txButt").style.color="red"; 
      Socket.send(document.getElementById("txBuff").value); //Send the data from txBuff input to server, to use this function just input the string you want to send.
       //alert("Submitted code: " && document.getElementById("txBuff").value && " has been transmitted");
      document.getElementById("txBuff").value = ""; //clears the texbox after data is sent
   }
   else if(Socket.readyState==1 && document.getElementById("txBuff").value == "") {
         alert("To submit you must add text!.");
         return;
   }
}
   
function addLoadEvent(func) {
   var oldonload = window.onload;
   if (typeof window.onload != 'function') {
       window.onload = func;
   } 
   else {
          window.onload = function() {
          if (oldonload) {
              oldonload();
          }
          func();
          }   
  }
}
addLoadEvent(initWebSockets);
addLoadEvent(statusCondition);

setInterval(function () { updateDTG(); }, 1000); //used for showing current DTG

function updateDTG() {
  var d = new Date();
  var t = "";
  t = d.toLocaleString('en-AU', { hour12: false });
  document.getElementById('DTG').innerHTML = t;
}

function statusCondition() {
  checkPercentageLevel();
  checkTemperatureLevel();
  checkControllerDCVoltageLevels();
  checkControllerACVoltageLevels();
  checkRemoteSensorDCVoltageLevels();
  checkControllerIndicators();
  checkFixedInfo();
}
//----------------------------------------------------------------------------
// Status Color for table rows & indicators
//----------------------------------------------------------------------------
 //Static Table Info
function checkFixedInfo() {     
  var tblFixedInfo = document.getElementById('fixedInfo');
  var rowValue;
  for(i=1; i<tblFixedInfo.rows.length; i++){
    rowValue=tblFixedInfo.rows[i].cells[tblFixedInfo.rows[i].cells.length-2].innerText;
    rowValue=parseFloat(rowValue);
    //console.log(rowValue);
    if(isNaN(rowValue)) {
      bgColor='red';
      tblFixedInfo.rows[i].style.backgroundColor = bgColor;
      tblFixedInfo.rows[i].style.color = 'white';
    }
    else if(rowValue >0) {
      bgColor='yellowgreen';
      tblFixedInfo.rows[i].style.backgroundColor = bgColor;
      tblFixedInfo.rows[i].style.color = 'initial';
    }
    else if(rowValue <=0) {
      bgColor='red';
      tblFixedInfo.rows[i].style.backgroundColor = bgColor;
      tblFixedInfo.rows[i].style.color = 'white';
    }
    else {
      bgColor='transparent';
      tblFixedInfo.rows[i].style.backgroundColor = bgColor;
      tblFixedInfo.rows[i].style.color = 'initial';
    }              
  }                   
} 
//----------------------------------------------------------------------------
  //Control Status Indicators
   function checkControllerIndicators() {
    //OperatingMode
      var opMode = document.getElementById("OperatingMode").innerHTML;
      if (opMode == '0') { //TDPSA
        document.getElementById("circleOpMode").style.backgroundColor = 'lime';
      }
      else if (opMode == '1') { //TCPRA
        document.getElementById("circleOpMode").style.backgroundColor = 'green';
      }
      else if (opMode == '2') { //TDPSM
        document.getElementById("circleOpMode").style.backgroundColor = 'yellow';
      }
      else if (opMode == '3') { //TCPRM
        document.getElementById("circleOpMode").style.backgroundColor = 'orange';
      }
      else if (opMode == '4') { //TDPSB
        document.getElementById("circleOpMode").style.backgroundColor = 'grey';
      }
      else if (isNaN(opMode)) {
        document.getElementById("circleOpMode").style.backgroundColor = 'red';
      }
      else {
        document.getElementById("circleOpMode").style.backgroundColor = 'black';
      }

      //TankFull
      var tankFull = document.getElementById("TankFull").innerHTML;
      if (tankFull == '1') {
        document.getElementById("circleTankFull").style.backgroundColor = 'lime';
      }
      else if (tankFull == '0') {
        document.getElementById("circleTankFull").style.backgroundColor = 'transparent';
      }
       else if (isNaN(tankFull)) {
        document.getElementById("circleTankFull").style.backgroundColor = 'red';
      }
      else {
        document.getElementById("circleTankFull").style.backgroundColor = 'black';
      }

      //TankLevelOk
      var tankLevelOk = document.getElementById("TankLevelOk").innerHTML;
      if (tankLevelOk == '1') {
        document.getElementById("circleTankLevelOk").style.backgroundColor = 'lime';
      }
      else if (tankLevelOk == '0') {
        document.getElementById("circleTankLevelOk").style.backgroundColor = 'transparent';
      }
      else if (isNaN(tankLevelOk)) {
        document.getElementById("circleTankLevelOk").style.backgroundColor = 'red';
      }
      else {
        document.getElementById("circleTankLevelOk").style.backgroundColor = 'black';
      }

      //TankEmpty
      var tankEmpty = document.getElementById("TankEmpty").innerHTML;
      if (tankEmpty == '1') {
        document.getElementById("circleTankEmpty").style.backgroundColor = 'lime';
      }
      else if (tankEmpty == '0') {
        document.getElementById("circleTankEmpty").style.backgroundColor = 'transparent';
      }
      else if (isNaN(tankEmpty)) {
        document.getElementById("circleTankEmpty").style.backgroundColor = 'red';
      }
      else {
        document.getElementById("circleTankEmpty").style.backgroundColor = 'black';
      }

      //TankFilling
      var tankFilling = document.getElementById("TankFilling").innerHTML;
      if (tankFilling == '1') {
        document.getElementById("circleTankFilling").style.backgroundColor = 'lime';
      }
      else if (tankFilling == '0') {
        document.getElementById("circleTankFilling").style.backgroundColor = 'transparent';
      }
       else if (isNaN(tankFilling)) {
        document.getElementById("circleTankFilling").style.backgroundColor = 'red';
      }
      else {
        document.getElementById("circleTankFilling").style.backgroundColor = 'black';
      }

      //PumpRunning
      var pumpRunning = document.getElementById("PumpRunning").innerHTML;
      if (pumpRunning == '1') {
        document.getElementById("circlePumpRunning").style.backgroundColor = 'lime';
      }
      else if (pumpRunning == '0') {
        document.getElementById("circlePumpRunning").style.backgroundColor = 'transparent';
      }
       else if (isNaN(pumpRunning)) {
        document.getElementById("circlePumpRunning").style.backgroundColor = 'red';
      }
      else {
        document.getElementById("circlePumpRunning").style.backgroundColor = 'black';
      }

      //ServosRunning
      var servosRunning = document.getElementById("ServosRunning").innerHTML;
      if (servosRunning == '1') {
        document.getElementById("circleServosRunning").style.backgroundColor = 'lime';
      }
      else if (servosRunning == '0') {
        document.getElementById("circleServosRunning").style.backgroundColor = 'transparent';
      }
       else if (isNaN(servosRunning)) {
        document.getElementById("circleServosRunning").style.backgroundColor = 'red';
      }
      else {
        document.getElementById("circleServosRunning").style.backgroundColor = 'black';
      }

      //Servo1Position
      var servo1Position = document.getElementById("Servo1Position").innerHTML;
      if (servo1Position == '0') { //vertical
        document.getElementById("circleServo1Position").style.backgroundColor = 'lime';
      }
      else if (servo1Position == '1') { //horizontal
        document.getElementById("circleServo1Position").style.backgroundColor = 'blue';
      }
       else if (isNaN(servo1Position)) {
        document.getElementById("circleServo1Position").style.backgroundColor = 'red';
      }
      else {
        document.getElementById("circleServo1Position").style.backgroundColor = 'black';
      }

      //Servo2Position
      var servo2Position = document.getElementById("Servo2Position").innerHTML;
      if (servo2Position == '0') { //horizontal
        document.getElementById("circleServo2Position").style.backgroundColor = 'lime';
      }
      else if (servo2Position == '1') { //vertical
        document.getElementById("circleServo2Position").style.backgroundColor = 'blue';
      }
       else if (isNaN(servo2Position)) {
        document.getElementById("circleServo2Position").style.backgroundColor = 'red';
      }
      else {
        document.getElementById("circleServo2Position").style.backgroundColor = 'black';
      }

      //RadioLinkRunning
      var radioLinkActive = document.getElementById("RadioLinkActive").innerHTML;
      if (radioLinkActive == '1') {
        document.getElementById("circleRadioLinkActive").style.backgroundColor = 'lime';
      }
      else if (radioLinkActive == '0') {
        document.getElementById("circleRadioLinkActive").style.backgroundColor = 'transparent';
      }
       else if (isNaN(radioLinkActive)) {
        document.getElementById("circleRadioLinkActive").style.backgroundColor = 'red';
      }
      else {
        document.getElementById("circleRadioLinkActive").style.backgroundColor = 'black';
      }

      //VacConstActive
      var vacConstActive = document.getElementById("VacConstActive").innerHTML;
      if (vacConstActive == '1') {
        document.getElementById("circleVacActive").style.backgroundColor = 'lime';
      }
      else if (vacConstActive == '0') {
        document.getElementById("circleVacActive").style.backgroundColor = 'transparent';
      }
      else if (isNaN(vacConstActive)) {
        document.getElementById("circleVacActive").style.backgroundColor = 'red';
      }
      else {
        document.getElementById("circleVacActive").style.backgroundColor = 'black';
      }
      

      //Relay1Position
      var relay1Position = document.getElementById("Relay1Position").innerHTML;
      if (relay1Position == '0') {
        document.getElementById("circleRelay1Position").style.backgroundColor = 'lime';
      }
      else if (relay1Position == '1') {
        document.getElementById("circleRelay1Position").style.backgroundColor = 'blue';
      }
      else if (isNaN(relay2Position)) {
        document.getElementById("circleRelay1Position").style.backgroundColor = 'red';
      }
      else {
        document.getElementById("circleRelay1Position").style.backgroundColor = 'black';
      }

      //Relay2Position
      var relay2Position = document.getElementById("Relay2Position").innerHTML;
      if (relay2Position == '0') {
        document.getElementById("circleRelay2Position").style.backgroundColor = 'lime';
      }
      else if (relay2Position == '1') {
        document.getElementById("circleRelay2Position").style.backgroundColor = 'blue';
      }
        else if (isNaN(relay2Position)) {
        document.getElementById("circleRelay2Position").style.backgroundColor = 'red';
      }
      else {
        document.getElementById("circleRelay2Position").style.backgroundColor = 'black';
      }

      //BuzzerActive
      var buzzerActive = document.getElementById("BuzzerActive").innerHTML;
      if (buzzerActive =='0') {
        document.getElementById("circleBuzzerActive").style.backgroundColor = 'transparent';
      }
      else if (buzzerActive == '1') {
        document.getElementById("circleBuzzerActive").style.backgroundColor = 'lime';
      }
      else if (isNaN(buzzerActive)) {
        document.getElementById("circleBuzzerActive").style.backgroundColor = 'red';
      }
      else {
        document.getElementById("circleBuzzerActive").style.backgroundColor = 'black';
      } 

      //SystemAlarm
      var systemAlarm = document.getElementById("SystemAlarm").innerHTML;
      if (systemAlarm =='0') {
        document.getElementById("circleSystemAlarm").style.backgroundColor = 'transparent';
      }
      else if (systemAlarm == '1') {
        document.getElementById("circleSystemAlarm").style.backgroundColor = 'orange';
      }
      else if (isNaN(systemAlarm)) {
        document.getElementById("circleSystemAlarm").style.backgroundColor = 'red';
      }
      else {
        document.getElementById("circleSystemAlarm").style.backgroundColor = 'black';
      } 
}   
//----------------------------------------------------------------------------
function checkControllerDCVoltageLevels() {
      //controller voltages:
      var value12v = document.getElementById("Controller12v").innerHTML;
      value12v = parseFloat(value12v);
      var value7v = document.getElementById("Controller7v").innerHTML;
      value7v = parseFloat(value7v);
      var value5v = document.getElementById("Controller5v").innerHTML;
      value5v = parseFloat(value5v);
      var value3v3 = document.getElementById("Controller3v3").innerHTML;
      value3v3 = parseFloat(value3v3);

      if (value12v >= 10.8 && value12v <= 13.2) {
        document.getElementById("controllerVoltages1").style.backgroundColor = "yellowgreen";
        document.getElementById("controllerVoltages1").style.color = "initial";
      }
      else {
        document.getElementById("controllerVoltages1").style.backgroundColor = "red";
        document.getElementById("controllerVoltages1").style.color = "white";
      }

      if (value7v >= 6.3 && value7v <= 7.7) {
        document.getElementById("controllerVoltages2").style.backgroundColor = "yellowgreen";
        document.getElementById("controllerVoltages2").style.color = "initial";
      }
      else {
        document.getElementById("controllerVoltages2").style.backgroundColor = "red";
        document.getElementById("controllerVoltages2").style.color = "white";
      }

      if (value5v >= 4.5 && value5v <= 5.5) {
        document.getElementById("controllerVoltages3").style.backgroundColor = "yellowgreen";
        document.getElementById("controllerVoltages3").style.color = "initial";
      }
      else {
        document.getElementById("controllerVoltages3").style.backgroundColor = "red";
        document.getElementById("controllerVoltages3").style.color = "white";
      }

      if (value3v3 >= 2.97 && value3v3 <= 3.63) {
        document.getElementById("controllerVoltages4").style.backgroundColor = "yellowgreen";
        document.getElementById("controllerVoltages4").style.color = "initial";
      }
      else {
        document.getElementById("controllerVoltages4").style.backgroundColor = "red";
        document.getElementById("controllerVoltages4").style.color = "white";
      }
}       
//----------------------------------------------------------------------------
function checkControllerACVoltageLevels() {
      //controller AC voltages:
      var valueVacConst = document.getElementById("controllerVacConst").innerHTML;
      valueVacConst = parseFloat(valueVacConst);
      var valueVacSw = document.getElementById("controllerVacSw").innerHTML;
      valueVacSw = parseFloat(valueVacSw);

      //vac Const
    if (valueVacConst >= 23.5 && valueVacConst <= 29.5) {
        document.getElementById("controllerVacVoltages1").style.backgroundColor = "yellowgreen";
        document.getElementById("controllerVacVoltages1").style.color = "initial";
      }
      else {
        document.getElementById("controllerVacVoltages1").style.backgroundColor = "red";
        document.getElementById("controllerVacVoltages1").style.color = "white";
      }

      //vac Switched
      if (valueVacSw >= 23.5 && valueVacSw <= 29.5) {
        document.getElementById("controllerVacVoltages2").style.backgroundColor = "yellowgreen";
        document.getElementById("controllerVacVoltages2").style.color = "initial";
      }
      else {
        document.getElementById("controllerVacVoltages2").style.backgroundColor = "red";
        document.getElementById("controllerVacVoltages2").style.color = "white";
      }
}    
//----------------------------------------------------------------------------
function checkRemoteSensorDCVoltageLevels() {
      //remote Sensor Voltages
      var valueRS12v = document.getElementById("BatteryVoltageLevel").innerHTML;
      valueRS12v = parseFloat(valueRS12v);
      var valueRS5v = document.getElementById("SensorControl5v").innerHTML;
      valueRS5v = parseFloat(valueRS5v);
      //12v Battery
      if (valueRS12v >= 10.8 && valueRS12v <= 13.2) {
        document.getElementById("remoteSensorVoltages1").style.backgroundColor = "yellowgreen";
        document.getElementById("remoteSensorVoltages1").style.color = "initial";
      }
      else {
        document.getElementById("remoteSensorVoltages1").style.backgroundColor = "red";
        document.getElementById("remoteSensorVoltages1").style.color = "white";
      }
      //5v Control
      if (valueRS5v >= 4.5 && valueRS5v <= 5.5) {
        document.getElementById("remoteSensorVoltages2").style.backgroundColor = "yellowgreen";
        document.getElementById("remoteSensorVoltages2").style.color = "initial";
      }
      else {
        document.getElementById("remoteSensorVoltages2").style.backgroundColor = "red";
        document.getElementById("remoteSensorVoltages2").style.color = "white";
      }
}   
//----------------------------------------------------------------------------
function checkPercentageLevel() {  
      // Percentage reading affects first 3 rows in 'actuals' table.
      var tblActuals = document.getElementById('actuals');
      var rowValue;
      for(i=1; i<tblActuals.rows.length-1; i++){  
          rowValue=tblActuals.rows[i].cells[tblActuals.rows[i].cells.length-2].innerText;
          rowValue=parseFloat(rowValue);
          //console.log(rowValue);
          if (i==1) {
             if(isNaN(rowValue) || rowValue <=0) {
                bgColor='red';
                tblActuals.rows[i].style.backgroundColor = bgColor;
                tblActuals.rows[i].style.color = 'white';
             }
             else if(rowValue < 0 && rowValue > 100) {
                bgColor='red';
                tblActuals.rows[i].style.backgroundColor = bgColor;
                tblActuals.rows[i].style.color = 'white';
             }
             else  if(rowValue >= 95 && rowValue < 100) {
                bgColor='dodgerblue';
                tblActuals.rows[i].style.backgroundColor = bgColor;
                tblActuals.rows[i].style.color = 'white';
             }
             else if(rowValue >= 75 && rowValue < 95) {
                bgColor='lightgreen';
                tblActuals.rows[i].style.backgroundColor = bgColor;
                tblActuals.rows[i].style.color = 'initial';
             }
             else if(rowValue >= 50 && rowValue < 75) {
                bgColor='yellow';
                tblActuals.rows[i].style.backgroundColor = bgColor;
                tblActuals.rows[i].style.color = 'initial';
             }
             else if(rowValue >= 10 && rowValue < 50) {
                bgColor='orange';
                tblActuals.rows[i].style.backgroundColor = bgColor;
                tblActuals.rows[i].style.color = 'initial';
             }
             else if(rowValue >= 0 && rowValue < 10) {
                bgColor='lightgrey';
                tblActuals.rows[i].style.backgroundColor = bgColor;
                tblActuals.rows[i].style.color = 'white';
             }
           }
           if (i==2) { //Available capacity
             if(isNaN(rowValue) || rowValue <=0) {
                bgColor='red';
                tblActuals.rows[i].style.backgroundColor = bgColor;
                tblActuals.rows[i].style.color = 'white';
             }
             else if(rowValue > 0) {
               tblActuals.rows[i].style.backgroundColor = "" + tblActuals.rows[1].style.backgroundColor;
               tblActuals.rows[i].style.color = '' + tblActuals.rows[1].style.color;
             } 
           }
           if (i==3) { //Actual Level (from Top)
             if(isNaN(rowValue) || rowValue <20) {//FullSetPoint
                bgColor='red';
                tblActuals.rows[i].style.backgroundColor = bgColor;
                tblActuals.rows[i].style.color = 'white';
             }
             else if(rowValue >= 20) { //FullSetPoint
                tblActuals.rows[i].style.backgroundColor = "" + tblActuals.rows[1].style.backgroundColor;
                tblActuals.rows[i].style.color = '' + tblActuals.rows[1].style.color;
             } 
           }
      }                             
} 
//---------------------------------------------------------------------------- 
 function checkTemperatureLevel() {  
    // Temperature reading located on 5th (bottom) row of 'actuals' table.
    var tblActuals = document.getElementById('actuals');
    var rowValue;
    for(i=1; i<tblActuals.rows.length; i++){  
       rowValue=tblActuals.rows[i].cells[tblActuals.rows[i].cells.length-2].innerText;
       if (i==4) {
          rowValue=parseFloat(rowValue);
          //console.log(rowValue);
          if(isNaN(rowValue) || rowValue <=0) {
             bgColor='red';
             tblActuals.rows[i].style.backgroundColor = bgColor;
             tblActuals.rows[i].style.color = 'white';
          }
          else if(rowValue >40) {
                bgColor='crimson';
                tblActuals.rows[i].style.backgroundColor = bgColor;
                tblActuals.rows[i].style.color = 'white';
          }
          else if(rowValue >= 30 && rowValue <= 40) {
                bgColor='orange';
                tblActuals.rows[i].style.backgroundColor = bgColor;
                tblActuals.rows[i].style.color = 'initial';
          }
          else if(rowValue >= 20 && rowValue <= 30) {
                bgColor='yellow';
                tblActuals.rows[i].style.backgroundColor = bgColor;
                tblActuals.rows[i].style.color = 'initial';
          }
          else if(rowValue >= 10 && rowValue <= 20) {
                bgColor='grey';
                tblActuals.rows[i].style.backgroundColor = bgColor;
                tblActuals.rows[i].style.color = 'white';
          }
          else if(rowValue >= 0 && rowValue <= 10) {
                bgColor='white';
                tblActuals.rows[i].style.backgroundColor = bgColor;
                tblActuals.rows[i].style.color = 'initial';
          }
          break; 
       }
    }                              
} 
//----------------------------------------------------------------------------

window.addEventListener("change",statusCondition(),false);  

window.onload = function(e){ 
    initWebSockets();
    wsReadyState();
    setTxItemStatus();
    socketReceive();
    socketErrors();
    socketClose();
    statusCondition();
}
</script>
</body>
<title>Water Tank Status Monitor</title>
</head>

</html>
)=====";



//===================================================================
// This routine is executed when you open a browser at the IP address
//===================================================================
void handleRoot() {
  //Display HTML contents
 // String s = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
  digitalWrite(2, 0); //Blinks on board led on page request
  server.send(200, "text/html",Web_page); //Send web page
  digitalWrite(2, 1);
} 
//------------------------------------------------------------------------//
void webSocketEvent(byte num, WStype_t type, uint8_t * payload, size_t length) { //for receiving websockets data


// Figure out the type of WebSocket event
  switch(type) {

    // Client has disconnected
    case WStype_DISCONNECTED:
        wsDisconnected = 1; 
        break;

    // New client has connected
    case WStype_CONNECTED:
        wsConnected = 1;     
        break;

    // Handle text messages from client
    case WStype_TEXT:
      {  
        // since payload is a pointer we need to type cast to char
       for(int i = 0; i <  maxBuffer; i++) { 
        wsPayload+= String((char *) &payload[i]);
        }
       String((char *) &payload)="\0"; //clear payload
       break;
      }       
    
	  
	case WStype_PING:
            // pong will be send automatically
            Serial.printf("[WSc] get ping\n");
            break;
			
    case WStype_PONG:
            // answer to a ping we send
            Serial.printf("[WSc] get pong\n");
            break; 

    // For everything else: do nothing
    case WStype_BIN:
    case WStype_ERROR:
    case WStype_FRAGMENT_TEXT_START:
    case WStype_FRAGMENT_BIN_START:
    case WStype_FRAGMENT:
    case WStype_FRAGMENT_FIN:
    
    default:
      break;
  }
} 
//------------------------------------------------------------------------//
void processData() {
  const char startByte = '<';
  const char stopByte = '>';
  byte overflowBuffer=0;
  byte index =0;
  byte idx=0;
  byte rxData=0;
  byte txData=0;
  String junk="";
  String overflowMsg="Overflow Occured!";
  delay(250);
  Serial.swap(); //to rx GPIO13, tx GPIO15
  delay(250);
  serialSwapped=1;
  if (connCount==1) {
    if (Serial.available()) {
      Serial1.println();
      Serial1.println("Serial available.");
      Serial1.println();
      rxData=0;   
    while (Serial.available() > 0){
          char inChar = {(char)Serial.read()}; 
       if (inChar == startByte) { // If start byte is received
          index = 0; 
       } 
       else if (inChar == stopByte) { // If end byte is received  
          webData[index] = '\0'; // then null terminate for end of data
          idx=index;
          index = 0; // this isn't necessary, but helps limit overflow
          rxData=1; //Data received
          webSocket.broadcastTXT(webData, sizeof(webData));
          txData=1;
       }
       else { // otherwise
          webData[index] = inChar; // put the character into our array
          index++; // and move to the next key in the array
       }
       
       if (index >= maxBuffer) {
        overflowBuffer=1;
       }
    }
        junk = Serial.read();
    }
    else {
      Serial1.println();
      Serial1.println("Serial NOT available to rx data.");
    }            
}        
  delay(250);
  Serial.swap(); // back to tx GPIO1, rx GPIO3 default
  delay(250);
  if (serialSwapped==1) {
    serialSwapped=0; 
  }
  if (rxData==1) {
    Serial.print("Data Received from Controller: ") && Serial.print(idx) && Serial.println(" bytes");
    rxData=0;
 }
   if (txData==1) {
    Serial.print("Data Sent to Webpage: ") && Serial.print(idx) && Serial.println(" bytes");   
    txData=0;
    idx=0; 
 }
 if (overflowBuffer==1) {
  Serial.println(overflowMsg);
  overflowBuffer=0;
 } 
 if (wsPayload.length()> 0) {
  //Serial.print("Length of wsPayload: ") && Serial.println(wsPayload.length());  
  //Serial.println();
  Serial.println(wsPayload);
  wsPayload="";
 }
 if (wsConnected==1 && connCount==0) {
   Serial.println("ClientSocket Connected!");
   connCount=1;
}
else if (wsDisconnected==1 && connCount==1) {
  Serial.println("ClientSocket Disconnected!");
    Serial.println("Socket connection Failed Resetting Server.....");
    ESP.restart();
}
}
//------------------------------------------------------------------------//
void handleWebRequests(){
  String msg = "Error 404: WebPage Not Detected\n\n";
  msg += "URI: ";
  msg += server.uri();
  msg += server.args();
  msg += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    msg += " NAME:"+server.argName(i) + "\n VALUE:" + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", msg);
  Serial.println(msg);
}
//------------------------------------------------------------------------//
void clientCommandActions(WsPayload) {
  switch(WsPayload) {
    case "resetCont":


    case "pumpOFF":


    case "pumpON":


    case "servoPosChange":


    case else:
      Serial.println (wsPayload);
      wsPayload="";
      break; 
}
}
//--------------------------------------------------------------------------//
void setup(void)
{ 
    pinMode(2,OUTPUT);
   Serial.begin(baud); //tx GPIO1, rx GPIO3 default
    Serial1.begin(baud); //tx only gpio2 for debugging
     // WiFiManager
  // Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;
  
  // Uncomment and run it once, if you want to erase all the stored information
  //wifiManager.resetSettings();
  
  // set custom ip for portal
  //wifiManager.setAPConfig(IPAddress(10,0,1,1), IPAddress(10,0,1,1), IPAddress(255,255,255,0));

  // fetches ssid and pass from eeprom and tries to connect
  // if it does not connect it starts an access point with the specified name
  // here  "AutoConnectAP"
  // and goes into a blocking loop awaiting configuration
  wifiManager.autoConnect("AutoConnectAP");
  // or use this for auto generated name ESP + ChipID
  //wifiManager.autoConnect();
  
  // if you get here you have connected to the WiFi
  Serial.println("Connected.");
 
    while (!Serial){
        ; // wait for serial port to connect. Needed for Leonardo only
    }
     server.on("/", handleRoot); // This displays the main webpage, it is called when you open a client connection on the IP address using a browser
     server.onNotFound(handleWebRequests); //Set setver all paths are not found so we can handle as per URI 
     server.begin();

     Serial.println("HTTP server started");

//--------------------------- 
  webSocket.begin();   // start the websocket server
  webSocket.onEvent(webSocketEvent);
  webSocket.enableHeartbeat(15000, 3000, 2); // 15 sec ping, 3 sec pong, 2 fails = disconnect
 
  
  Serial.println("WebSocket server started.");



}//end SETUP
 
void loop()
{
    server.handleClient();  // Keep checking for a client connection
    webSocket.loop();
    processData(); // rx data from controller tx to webpage via websockets   
}//end LOOP