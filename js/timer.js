const timer = {
  startTime: null,
  length: null,
  display: null,
  interval: null,
  updateDisplay: function() {
    let seconds = this.length - Math.floor((performance.now() - this.startTime) / 1000);
    const minutes = Math.floor(seconds / 60);
    seconds = seconds % 60;
    if (seconds < 10) {
      seconds = `0${seconds}`;
    }
    this.display.innerHTML = `${minutes}:${seconds}`;
    if (this.display.innerHTML == "0:00") {
      for (let i of document.querySelector("audio")) {
        i.pause();
      }
      for (let i of document.querySelector("video")) {
        i.pause();
      }
      clearInterval(this.interval);
    }
  },
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
    this.updateDisplay();
  },
  clear: function() {
    clearInterval(this.interval);
    this.display.innerHTML = "0:00";
  }
}
