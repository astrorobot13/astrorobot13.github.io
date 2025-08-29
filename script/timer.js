class Timer {
  startTime = null;
  length = null;
  display = null;
  interval = null;
  onend = () => {};
  addEventListener = (event, callback) => {
    if (event === "end") {
      this.onend = callback;
    }
  }
  updateDisplay = () => {
    let seconds = this.length - Math.floor((performance.now() - this.startTime) / 1000);
    const minutes = Math.floor(seconds / 60);
    seconds = seconds % 60;
    if (seconds < 10) {
      seconds = `0${seconds}`;
    }
    this.display.innerHTML = `${minutes}:${seconds}`;
    if (this.display.innerHTML == "0:00") {
      clearInterval(this.interval);
      for (let i of document.querySelectorAll("audio")) {
        i.pause();
      }
      for (let i of document.querySelectorAll("video")) {
        i.pause();
      }
      this.onend();
    }
  };
  set = (length, units="min") => {
    this.clear();
    this.startTime = performance.now();
    // time is in seconds
    if (units == "min") {
      this.length = length * 60;
    } else if (units == "sec") {
      this.length = length;
    } else if (units == "msec") {
      this.length = length / 1000;
    }
    this.interval = setInterval(this.updateDisplay, 1000 / 13);
    this.updateDisplay();
  };
  clear = () => {
    clearInterval(this.interval);
    this.display.innerHTML = "0:00";
  };
  constructor(displayElement=null) {
    this.display = displayElement;
  }
}
