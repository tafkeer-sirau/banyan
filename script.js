        function startSpeech() {
            const recognition = new (window.SpeechRecognition || window.webkitSpeechRecognition)();
            recognition.lang = "en-IN";
            recognition.start();

            recognition.onresult = function(event) {
                document.getElementById("speechOutput").innerHTML = event.results[0][0].transcript;
            }
        }