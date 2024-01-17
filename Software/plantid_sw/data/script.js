//load moisture every 3 seconds
setInterval(function() {
  getMoisture();
}, 3000);
function getMoisture() {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById('moistureValue').innerHTML = this.responseText;
    }
  };
  xhttp.open('GET', 'moisture', true);
  xhttp.send();
}

//load temperature every 3 seconds
setInterval(function() {
  getTemperature();
}, 3000);
function getTemperature() {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById('temperatureValue').innerHTML = this.responseText;
    }
  };
  xhttp.open('GET', 'temperature', true);
  xhttp.send();
}

//load pump state every 3 seconds
setInterval(function() {
  getPumpState();
}, 3000);
function getPumpState() {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById('pumpStateValue').innerHTML = this.responseText;
    }
  };
  xhttp.open('GET', 'pumpState', true);
  xhttp.send();
}

//load current watering Time
function getCurrentWateringTime() {
fetch('/getWateringTime')
  .then(response => response.json())
  .then(data => {
    document.getElementById('currentWateringTime').innerText = data.wateringTime/1000 + ' Seconds';
    document.getElementById('newWateringTime').value = data.wateringTime/1000;
  });
}

//update watering time
function updateWateringTime() {
var newWateringTime = document.getElementById('newWateringTime').value;
fetch('/updateWateringTime', {
  method: 'POST',
  headers: {
    'Content-Type': 'application/x-www-form-urlencoded',
  },
  body: 'wateringTime=' + newWateringTime*1000,
})
.then(response => response.text())
.then(data => {
  if (data === 'OK') {
    getCurrentWateringTime();
  } else {
    alert('Error while updating watering Time!');
  }
});
}

// initialize
getCurrentWateringTime();
