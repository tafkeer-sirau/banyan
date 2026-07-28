        function startSpeech() {
            const recognition = new (window.SpeechRecognition || window.webkitSpeechRecognition)();
            recognition.lang = "en-IN";
            recognition.start();

            recognition.onresult = function(event) {
                document.getElementById("speechOutput").innerHTML = event.results[0][0].transcript;
            }
        }

// 1. Handle browser compatibility prefixes
const SpeechRecognition = window.SpeechRecognition || window.webkitSpeechRecognition;

if (!SpeechRecognition) {
  console.error("Your browser does not support the Web Speech API.");
} else {
  // 2. Initialize the speech recognition object
  const recognition = new SpeechRecognition();

  // 3. Configure behavior
  recognition.continuous = true;          // Keep listening even if the user pauses
  recognition.interimResults = true;      // Show real-time partial results as you speak
  recognition.lang = 'en-US';              // Set the preferred language BCP 47 code

  // 4. Start the microphone stream
  recognition.start();

  // 5. Process the audio stream results
  recognition.onresult = (event) => {
    let finalTranscript = '';
    let interimTranscript = '';

    for (let i = event.resultIndex; i < event.results.length; ++i) {
      if (event.results[i].isFinal) {
        finalTranscript += event.results[i][0].transcript;
      } else {
        interimTranscript += event.results[i][0].transcript;
      }
    }

    // Output live transcription data to console or DOM elements
    console.log("Final:", finalTranscript);
    console.log("Interim (Live):", interimTranscript);
  };

  // 6. Handle errors or unexpected stops
  recognition.onerror = (event) => {
    console.error("Speech recognition error:", event.error);
  };

  recognition.onend = () => {
    console.log("Speech recognition service disconnected.");
    // Optional: Call recognition.start() here to auto-restart the mic stream
  };
}
