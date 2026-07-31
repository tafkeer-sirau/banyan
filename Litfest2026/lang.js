  function toggleLang() {
    var eng = document.getElementById("eng");
    var hin = document.getElementById("hin");
    var btn = document.getElementById("langBtn");

    if (eng.style.display === "none") {
      eng.style.display = "block";
      hin.style.display = "none";
      btn.textContent = "हि";
    } else {
      eng.style.display = "none";
      hin.style.display = "block";
      btn.textContent = "En";
    }
  }