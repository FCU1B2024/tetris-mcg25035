const net = require('net');
const fs = require('fs');

var playerCount = 0;

var playerACanvas = "";
var playerALastSend = -1;
var playerAdamage = 0;
var playerBCanvas = "";
var playerBLastSend = -1;
var playerBdamage = 0;
var gameInterval = [];
var gaming = false;
var logged = false;
/** @type {net.Socket} */
var socketA;
/** @type {net.Socket} */
var socketB;

const newGameLog = function () {
  try{

      if (!fs.existsSync("gameCount.count")) {
          fs.writeFileSync("gameCount.count", "0");
      }
  
      var gameCount = fs.readFileSync("gameCount.count").toString();
      gameCount = Number(gameCount) + 1;
      fs.writeFileSync("gameCount.count", gameCount.toString());
  }
  catch (ignored) {
      return;
  }
}


// Create a TCP server
const server = net.createServer((socket) => {
  var player;
  if (gaming) {
    socket.write("SERVERFULL");
    socket.end();
    return;
  }
  if (playerCount == 0) {
    player = "A";
    socketA = socket;
  }
  else if (playerCount == 1) {
    player = "B";
    socketB = socket;
  }
      

  console.log('Client connected');

  // Handle incoming data from the client
  socket.on('data', (data) => {
    if (player == "A") {
        playerALastSend = Date.now();
    }
    else if (player == "B") {
        playerBLastSend = Date.now();
    }
    if (data.toString().includes("tetris connect")) {
        console.log("new player connected");
        playerCount++;
        logged = false;
        var waitStart = setInterval(() => {
            if (playerCount == 2){
                if (!logged){
                    newGameLog();
                    logged = true;
                }
                console.log("game start");
                socket.write("OK")
                clearInterval(waitStart);
                gaming = true;
            }
        }, 100);
    }
    if (data.toString().includes("attackDataJson")){
        var attackData = data.toString().split("attackDataJson")[1]
        var attack = JSON.parse(attackData);
        if (attack.attack == 0) return;
        if (player == "A"){
            playerBdamage += Number(attack.attack);
        }
        else if (player == "B"){
            playerAdamage += Number(attack.attack);
        }
    }
    else if (player == "A"){
        playerACanvas = data.toString();
    }
    else if (player == "B"){
        playerBCanvas = data.toString();
    }
  });

  gameInterval.push(setInterval(() => {
    if (!gaming) return;
    if (Date.now() - playerALastSend > 10000){
        console.log("player A timeout");
        playerACanvas = "GAMEOVER";
    }
    if (Date.now() - playerBLastSend > 10000){
        console.log("player B timeout");
        playerBCanvas = "GAMEOVER";
    }
    if (player == "A" && playerAdamage > 0){
        socket.write("damage.data"+playerAdamage+"damage.data");
        playerAdamage = 0;
    }
    if (player == "B" && playerBdamage > 0){
        socket.write("damage.data"+playerBdamage+"damage.data");
        playerBdamage = 0;
    }
    if (player == "A" && playerBCanvas != ""){
        socket.write("st"+playerBCanvas+"ed");
    }
    else if (player == "B" && playerACanvas != ""){
        socket.write("st"+playerACanvas+"ed");
    }
    if (playerACanvas.includes("GAMEOVER") || playerBCanvas.includes("GAMEOVER")){
        console.log("game end, restart");
        if (socketA.writable) socketA.write("stGAMEOVERed");
        if (socketB.writable) socketB.write("stGAMEOVERed");
        socketA.destroy();
        socketB.destroy();
        for (var i = 0; i < gameInterval.length; i++){
            clearInterval(gameInterval[i]);
        }
        playerALastSend = -1;
        playerBLastSend = -1;
        playerACanvas = "";
        playerBCanvas = "";
        playerAdamage = 0;
        playerBdamage = 0;
        gaming = false;
        playerCount = 0;
    }
  }, 200))


  // Handle client disconnect
  socket.on('end', () => {
    console.log('Client disconnected');
    if (!gaming) return;
    if (player == "A") playerACanvas = "GAMEOVER";
    if (player == "B") playerBCanvas = "GAMEOVER";
  });

  // Handle errors
  socket.on('error', (err) => {
    console.log('Client disconnected');
    if (!gaming) return;
    if (player == "A") playerACanvas = "GAMEOVER";
    if (player == "B") playerBCanvas = "GAMEOVER";
    console.error(`Error: ${err.message}`);
  });

});

// setInterval(() => {
//     console.log("Player A: " + playerACanvas);
//     console.log("Player B: " + playerBCanvas);
// }, 1000)

// Start the server and listen on port 8080
server.listen(6060, () => {
  console.log('Server listening on port 6060');
});