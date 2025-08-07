let time, clock, start, total;
function stop() {
  for (i of document.getElementsByTagName("audio")) {
    i.pause();
  }
  clearInterval(clock);
  document.getElementById("minutes").innerHTML = "0";
  document.getElementById("seconds").innerHTML = "00";
}
function setTimer() {
  clearTimeout(time);
  clearInterval(clock);
  if (isNaN(parseInt(document.getElementsByTagName("option")[document.getElementById("timer").selectedIndex].innerHTML))) {
    document.getElementById("minutes").innerHTML = "0";
    document.getElementById("seconds").innerHTML = "00";
  } else {
    const minutes = parseInt(document.getElementsByTagName("option")[document.getElementById("timer").selectedIndex].innerHTML);
    start = performance.now();
    total = minutes * 60;
    time = setTimeout(stop, minutes * 60000);
    clock = setInterval(displayClock, 75);
} }
function displayClock() {
  let seconds = total - Math.floor((performance.now() - start) / 1000);
  const minutes = Math.floor(seconds / 60);
  seconds = seconds % 60;
  if (seconds < 10) {
    seconds = "0".concat(seconds.toString());
  } else {
    seconds = seconds.toString();
  }
  document.getElementById("minutes").innerHTML = minutes.toString();
  document.getElementById("seconds").innerHTML = seconds;
}
const timer = {
  startTime: null,
  length: null,
  display: null,
  interval: null,
  set: function(length, units="min") {
    if (length == 0) {
      this.clear();
    }
    this.startTime = performance.now();
    if (units == "min") {
      this.length = length * 60000;
    } else if (units == "sec") {
      this.length = length * 1000;
    } else if (units == "msec") {
      this.length = length;
    }
    this.interval = setInterval(this.updateDisplay, 75);
  },
  updateDisplay: function() {
    const elapsedTime = Math.ceil((performance.now() - this.startTime) / 1000);
    
  },
  clear: function() {

  }
}
