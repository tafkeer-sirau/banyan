const START_BTN = document.getElementById("spt");
const TEXT_AREA = document.getElementById("news");
const TICKER = document.querySelector(".ticker-text");
const UPDATE = document.getElementById("news_b");

const news = localStorage.getItem("villageNews");
document.addEventListener("DOMContentLoaded", () => {
  console.log("Page loaded!");
  TEXT_AREA.value = news;
});

let isRecording = false;
let speechObj;
let finalTranscript = "";

const SpeechRecognition =
  window.SpeechRecognition || window.webkitSpeechRecognition;

if (!SpeechRecognition) {
  START_BTN.innerText = "Speech Not Supported";
  START_BTN.disabled = true;
} else {
  speechObj = new SpeechRecognition();
  speechObj.continuous = true;
  speechObj.interimResults = true;
  speechObj.lang = "en-IN";

  speechObj.onresult = (event) => {
    let interim = "";
    let final = "";

    for (let i = event.resultIndex; i < event.results.length; i++) {
      const text = event.results[i][0].transcript;

      if (event.results[i].isFinal) {
        final += text + " ";
        finalTranscript += text + " ";
      } else {
        interim += text;
      }
    }

    TEXT_AREA.value = finalTranscript + interim;
  };

  speechObj.onerror = (e) => {
    console.error("Speech error:", e.error);
  };
}

START_BTN.addEventListener("click", () => {
  if (!speechObj) return;

  if (!isRecording) {
    speechObj.start();
    START_BTN.innerText = "Stop Recording";
  } else {
    speechObj.stop();
    START_BTN.innerText = "Start Speech";
  }

  isRecording = !isRecording;
});

UPDATE.addEventListener("click", () => {
  const news = TEXT_AREA.value.trim();
  if (!news) return;

  localStorage.setItem("villageNews", news);

  alert("News updated!");
window.location.reload();

});
