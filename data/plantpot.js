var btn = document.getElementById("btn");
var hum = document.getElementById("hum");
var msgContainer = document.getElementById("msg-container");
var connection = new WebSocket("ws://" + location.hostname + ":81/");

function showMSG(text) {
    if(msgContainer.hasChildNodes()) {
        msgContainer.innerHTML = ''
    }
    var element = document.createElement("div");
    element.className = "msg";
    element.appendChild(document.createTextNode(text));
    msgContainer.appendChild(element);
    setTimeout(
        function() {
            element.remove();
        }, 3000
    );
}

connection.onopen = function () {
    showMSG("Connected...");
};

connection.onclose = function () {
    showMSG("You are disconnected...");
};

connection.onmessage = function (e) {

    var msg = e.data;
    let index = msg.charAt(0);
    msg = msg.substring(1);
    switch(index) {
        case "h":
            console.log(msg);
            hum.innerHTML = "<h1>" + msg + "%</h1>";
            break;
        case "p":
            console.log(msg);
            if(msg == "True") {
                btn.style.background = "rgb(208, 187, 0)";
            } else {
                btn.style.background = "rgba(208, 187, 0, 0)";
            }
            break;
    }

};

var timeout = false;
var delay = 2000;
btn.onclick = function() {
    if(timeout == false) {
        timeout = true;
        console.log(btn.value);
        waitForSocketConnection(connection, function(){
            connection.send("p");
        });
        setTimeout(function() {
            timeout = false;
        }, delay);
    }
}

// Make the function wait until the connection is made...
function waitForSocketConnection(socket, callback){
    setTimeout(function () {
        if (socket.readyState === 1) {
            console.log("Connection is made")
            if (callback != null){
                callback();
            }
        } else {
            console.log("wait for connection...")
            waitForSocketConnection(socket, callback);
        }
    }, 50); // wait 50 milisecond for the connection...
}