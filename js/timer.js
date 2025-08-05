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