// ==========================================================================
// News ticker — runs on every page that has a `.ticker-text` element
// ==========================================================================

const TICKER = document.querySelector(".ticker-text");

function loadNews() {
  const news = localStorage.getItem("villageNews");

  if (news && TICKER) {
    TICKER.innerText = news;
  }
}

loadNews();

// ==========================================================================
// Management page controls — only runs where these elements actually exist,
// so this file is safe to include on every page without extra script tags.
// ==========================================================================

const START_BTN = document.getElementById("spt");
const TEXT_AREA = document.getElementById("news");
const UPDATE = document.getElementById("news_b");

if (START_BTN && TEXT_AREA && UPDATE) {
  document.addEventListener("DOMContentLoaded", () => {
    TEXT_AREA.value = localStorage.getItem("villageNews");
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

      for (let i = event.resultIndex; i < event.results.length; i++) {
        const text = event.results[i][0].transcript;

        if (event.results[i].isFinal) {
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
}
