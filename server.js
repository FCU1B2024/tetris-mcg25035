const net = require('net');

var playerCount = 0;

var playerACanvas = "";
var playerAdamage = 0;
var playerBCanvas = "";
var playerBdamage = 0;


// Create a TCP server
const server = net.createServer((socket) => {
  var player;
  if (playerCount == 0) player = "A";
  else player = "B";

  console.log('Client connected');

  // Handle incoming data from the client
  socket.on('data', (data) => {
    if (data.toString().includes("tetris connect")) {
        playerCount++;
        var waitStart = setInterval(() => {
            if (playerCount == 2){
                socket.write("OK")
                console.log("new player connected")
                clearInterval(waitStart);
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

  setInterval(() => {
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
  }, 200)


  // Handle client disconnect
  socket.on('end', () => {
    playerCount--;
    console.log('Client disconnected');
  });

  // Handle errors
  socket.on('error', (err) => {
    playerCount--;
    console.error(`Error: ${err.message}`);
  });

});

setInterval(() => {
    console.log("Player A: " + playerACanvas);
    console.log("Player B: " + playerBCanvas);
}, 1000)

// Start the server and listen on port 8080
server.listen(8080, () => {
  console.log('Server listening on port 8080');
});
