# Welcome to Plantid

Plantid is an automatic watering system which works according the micro drip principle. The water will be pumped out a watertank through a 3,7V pump.
With the help of a PVC-pipe the water will be transported to the water sprinklers - which are in the earth next to the plants. The amount of water 
coming out of the sprinkler can be adjusted by turning the head. In addition there is a web application which monitors moisture level, temperature,
the state of the pump and the current watering time. The standard watering time is 10 seconds but can always be changed manually by the user in the
web application. The watering system will start watering if the moisture value drops under 25% and will only water once a day to avoid overwatering 
the plant. In addition you can press the "EN" button to water manually. It will be watered as long as you press the button.

![Plants placed in the box](https://github.com/stefanpab/Plantid/blob/main/img/Plantid_PlantsBox.jpeg "Plants placed in the box")

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
1) Download ESP32FS-1.1 zip package: https://github.com/me-no-dev/arduino-esp32fs-plugin/releases/
2) You need to know your sketchbook location from your Arduino IDE. To find that, open the Arduino IDE and go to "File->Preferences"
![Arduino Preferences](https://github.com/stefanpab/Plantid/blob/main/img/Preferences_Arduino.png "Arduino Preferences")
3) Go to the sketchbook location in your explorer and create a folder namened "tools"
![Arduino exlporer](https://github.com/stefanpab/Plantid/blob/main/img/Arduino_exlorer.png "Arduino explorer")
4) Unzip the downloaded package and paste it into the tools folder
5) Restart the Arduino IDE
6) Now you will find under "Tools" the "ESP32 Sketch Data Upload" button

### Arduino IDE additional libraries
To make the project work you need to install some additional librarys. To install them click on Sketch->Include Library->Manage Libraries
Search for "ESP32Time" and press on install for following:
![ESP32 Time Library](https://github.com/stefanpab/Plantid/blob/main/img/ESP32TimeLibrary.png "ESP32 Time Library")
You will need two additional libraries which are not in the Library Manager. To install them go to File->Preferences
Click on following button:
![ESP32 Additional Libraries](https://github.com/stefanpab/Plantid/blob/main/img/Preferences_Arduino_Libraries.png "ESP32 Additional Libraries")
Paste then the following line into it and press ok. With that line you will install the the AsyncTCP and the AsynchWebServer which are 
necassary to build the asynchrouns webserver.
"https://github.com/me-no-dev/ESPAsyncWebServer; https://github.com/me-no-dev/AsyncTCP"

### Code adjustments
To edit the arduino code, you have to open the /Software/plantid_sw/plantid_sw.ino file. 
You can make several changes in this file which best fits for your usecase. At line 48 and 49 you have to replace those wifi credentials with your 
home wife name and password. In addition you can open /Software/plantid_sw/data/index.html to change the web application. You can change the CSS, or 
removing for example the pump state to be seen on the web gui. After finishing the editing of those files, go back to the Arduino Software.
Plug in a micro USB cable into the ESP32 and into your computer. Now go to "tools" and press the "ESP32 Sketch Data Upload" button. In most cases you 
now have to press the boot button for two seconds. After you get the message "Done uploading" in the arduino console, the index.html is 
successfully be uploaded. Now you have to upload the code. Click on tools and apply following settings:
![Arduino code upload settings](https://github.com/stefanpab/Plantid/blob/main/img/Arduino_CodeUpload.png "Arduino code upload settings")
Most important are Port and Board. Select "ESP32 Dev Module" as board and for the port the correct one where your ESP32 is plugged in. After selecting
the correct settings press the upload button (button in the red rectangle). In most cases it can be necassary to press the boot button again to 
successfully upload the code. After uploading the Code you can plug out the micro USB cable and screw the battery wires into the screw clamp.

### Web GUI
Now after uploading everything you can open the web gui by entering "plantid.local" in any internet explorer. You will now be able to see the current 
values of the watering system. In addition you can manually change the watering time by entering your needed time in seconds and confirming that by
pressing the "Change value" button.
![WEB GUI Plantid](https://github.com/stefanpab/Plantid/blob/main/img/Plantid_WEBgui.png "WEB GUI Plantid")

### Hardware
Fill the tank with water but fill it no higher than the plant pot to avoid that the water won't stop floating after watering. Plant your plants into the 
pot and place the moisture sensor in the pot. For the best results from the sensor, bury the sensor as deep as possible under the plants. Scew the PCB into
the 3D case and connect the pump, sensor and battery to the pcb. Close the PCB and plug the pipes into the pump and the water tank.

## Documentation
In the documentation folder you can find the ibom file. This file shows you an interactive instruction how to solder the pcb with which components. In 
addition you have the components list as an csv file to know the order number of the required parts. Also you can find the circuit diagram, so you dont have 
to download KiCad to have a look at it. Last but not least you will also find the block diagram in there.

# Support
If you run into any problems or have any questions regarding the project, feel free to message me under my email: sa21b004@technikum-wien.at
