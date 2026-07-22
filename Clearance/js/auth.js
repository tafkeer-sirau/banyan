// Simple client-side passcode gate for the management page.
// Note: this only deters casual visitors — anyone can read this file's
// source or the page's local storage, so it is not real access control.

const STORED_HASH =
  "fa67b4af01b38188840a1442ffbd05b5bc8cf19f1d4f645af4461a53764dfe8c";

async function sha256(message) {
  const msgBuffer = new TextEncoder().encode(message);
  const hashBuffer = await crypto.subtle.digest("SHA-256", msgBuffer);
  const hashArray = Array.from(new Uint8Array(hashBuffer));
  return hashArray.map((b) => b.toString(16).padStart(2, "0")).join("");
}

async function authenticateUser() {
  if (localStorage.getItem("confirmation") === "true") {
    document.documentElement.style.display = "block";
    return;
  }

  while (true) {
    const userInput = prompt("Please enter the passcode:");

    if (userInput === null) {
      document.write("<h1>Access Denied</h1>");
      window.stop();
      return;
    }

    if ((await sha256(userInput)) === STORED_HASH) {
      localStorage.setItem("confirmation", "true");
      document.documentElement.style.display = "block";
      return;
    }

    alert("Incorrect passcode. Try again.");
  }
}

document.documentElement.style.display = "none";
window.onload = authenticateUser;
