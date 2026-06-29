console.log("script.js loaded");
const TICKER = document.querySelector(".ticker-text");

function loadNews() {
  const news = localStorage.getItem("villageNews");

  if (news && TICKER) {
    TICKER.innerText = news;
  }
}

loadNews();
