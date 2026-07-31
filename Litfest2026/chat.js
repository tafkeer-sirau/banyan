	var inputField = document.getElementById('messageInput');
    var chatDisplay = document.getElementById('chatDisp');
    var typing = document.getElementById('typing');

    inputField.addEventListener('keydown', function(){
        typing.style.visibility="visible";
    });

    inputField.addEventListener('keyup', function(){
        typing.style.visibility="hidden";
    });

     var displayChat = document.getElementById('chatDisp');

     if(!localStorage.getItem("chat")){
     localStorage.setItem("chat", JSON.stringify([]));
     }
     var chatHistory = JSON.parse(localStorage.getItem("chat"));


     var send = document.getElementById('sendMessage');
     var i =0;
     send.addEventListener('click', function () {

         if (document.getElementById('messageInput').value == ""){
             alert("please Enter something in Chat....");
             return
         }

         var message={
             name : "Villager",
             text : document.getElementById('messageInput').value,
             dateTime : new Date().toLocaleTimeString() + " " + new Date().toDateString(),
             src : "http://pickaface.net/includes/themes/clean/img/slide4.png"
         };
         chatHistory.push(message);
         document.getElementById('messageInput').value = "";
         localStorage.setItem("chat", JSON.stringify(chatHistory));

         localData = localStorage.getItem("chat");
         localData = JSON.parse(localData);
         var templateDiv =
             "<div class='message'>" +
             "<p class='text'>" + localData[i].text + "</p>" + "<div class='clear'></div>" + "</div>"+
         "<p class='datetime'>" + localData[i].dateTime + "</p>" + "<div class='clear'></div>";
         i++;
         chatDisplay.innerHTML += templateDiv;
     });

document.addEventListener("DOMContentLoaded", () => {
	localData = localStorage.getItem("chat");
         localData = JSON.parse(localData);
         var templateDiv =
             "<div class='message'>" +
             
             "<p class='text'>" + localData[i].text + "</p>" + "<div class='clear'></div>" + "</div>"+
         "<p class='datetime'>" + localData[i].dateTime + "</p>" + "<div class='clear'></div>";
         i++;
         chatDisplay.innerHTML += templateDiv;
});
