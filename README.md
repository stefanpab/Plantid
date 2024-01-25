# Welcome to Plantid

Plantid is a automatic watering system which works according the micro drip principle. The water will be pumped out a watertank through a 3,7V pump.
With the help of a PVC-pipe the water will be transported to the water sprinklers - which are in the earth next to the plants. The amount of water 
coming out of the sprinkler can be adjusted by turning the head. In addition there is a web application which monitors moisture level, temperature,
the state of the pump and the current watering time. The standard watering time is 10 seconds but can always be changed manually by the user in the
web application.

![Plants placed in the box](https://github.com/stefanpab/Plantid/blob/main/img/Plantid_Plants.jpeg "Plants placed in the box")

## Content of the folders in this repo
The folders containing following information:
* _3D Case:_ all stl versions of the case for the pcb
* _Datasheets:_ all needed datasheeds for the project
* _Documentation:_ user manuals and documenation of plantid
* _Footprints:_ additional footprints needed for kicad 7.0
* _Kicad:_ schematics and PCB layout from Kicad 7.0
* _Software:_ arduino code and web application code
* _img:_ pictures of the project

## How to use Plantid
### Arduino IDE
To use plantid you have to first download the latest version of Arduino 1. !Important! You can't use the Arduino 2.0 application because the file 
upload manager you need for the project is not supported yet. You can Download it here: https://www.arduino.cc/en/software

### Arduino IDE additional managers
In addtion you need to download the file upload manager for esp32. 
1) Download following zip package: https://github.com/me-no-dev/arduino-esp32fs-plugin/releases/
2) You need to know your sketchbook location from your Arduino IDE. To find that, open the Arduino IDE and go to "File->Preferences"
3) Go to the sketchbook location in your explorer and create a folder namened "tools"
4) Unzip the downloaded package and paste it into the tools folder
5) Restart the Arduino IDE
6) Now you will find under "Tools" the "ESP32 Sketch Data Upload" button

### Code adjustments
To edit the arduino code now, you have to open the /Software/plantid_sw/plantid_sw.ino file. 
You can make several changes in this file which best fits for your usecase. At line 48 and 49 you have to replace those wifi credentials with your 
home wife name and password. In addition you can open /Software/plantid_sw/data/index.html to change the web application. You can change the CSS, or 
removing for example the pump state to be seen on the web gui. After finishing the editing of those files, go back to the Arduino Software.
